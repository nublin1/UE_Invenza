// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Interaction/InteractionData.h"
#include "Data/Interactable/InteractableData.h"
#include "InteractionComponent.generated.h"

class UIInventoryManager;
class UItemBase;
enum class EInteractableType : uint8;
class UInteractableComponent;
class UInputAction;
class IInteractableData;
class UCameraComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

protected:
	// Unreal overrides
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginFocus, const TArray<FInteractionDisplayEntry>&, Entries);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndFocus, const TArray<FInteractionDisplayEntry>&, Entries);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteract, UInteractableComponent*, TargetInteractableComponent,
												 EInteractionType, Type);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStopInteract, UInteractableComponent*, TargetInteractableComponent,
												 EInteractionType, Type);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndInteract, UInteractableComponent*, TargetInteractableComponent);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionProgress, float, Progress);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionDisplayChanged, const TArray<FInteractionDisplayEntry>&,
												Entries);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractableBusy);
#pragma endregion Delegates

	// Properties and variables
	//Delegates
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnBeginFocus OnBeginFocus;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnEndFocus OnEndFocus;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteract OnInteract;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnStopInteract OnStopInteract;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnEndInteract OnEndInteract;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionProgress OnInteractionProgress;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionDisplayChanged OnInteractionDisplayChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractableBusy OnInteractableBusy;

	// Functions
	UFUNCTION()
	void InitInteractionComponent();

	UFUNCTION()
	FInteractableData GetDefaultInteractionData() { return DefaultInteractionData; };

	UInteractableComponent* GetActiveInteractable() const { return CurrentInteractableComponent; }

	const TArray<FInteractionKeyBinding>& GetKeyBindings() const { return KeyBindings; }

	EInteractionType GetActiveInteractionType() const { return ActiveInteractionType; }

	bool IsNotifyingStop() const { return bNotifyingStop; }

	static bool IsWithinInteractionDistance(const AActor* Interactor, const UInteractableComponent* Target);

	/** Registers a successfully opened server-side interaction, including AI interactions. */
	void TrackActiveInteraction(UInteractableComponent* Target, EInteractionType Type);

	/**
	* Stops an already active interaction with the current interactable.
	* Used when:
	* - pawn interacts again with the same object
	* - interaction needs to be forcefully stopped
	* Calls InteractableComponent->StopInteract().
	*/
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StopInteract();

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Trace")
	float InteractionCheckInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Trace")
	float InteractionCheckDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Input")
	TArray<FInteractionKeyBinding> KeyBindings;

	//
	UPROPERTY()
	FTimerHandle TimerHandle_Interaction;

	UPROPERTY(Category = "Invenza|Interaction", BlueprintReadOnly)
	float InteractionStartTime = 0.0f;

	UPROPERTY(Category = "Invenza|Interaction", BlueprintReadOnly)
	FInteractionData InteractionData;

	UPROPERTY(VisibleAnywhere, Category = "Interaction|State")
	UInteractableComponent* TargetInteractableComponent;

	UPROPERTY(VisibleAnywhere, Category = "Interaction|State")
	UInteractableComponent* CurrentInteractableComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction")
	TArray<FInteractableData> AvailableInteractions;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction")
	FInteractableData DefaultInteractionData;

	//Refs
	UPROPERTY(Category = "Invenza|Interaction", BlueprintReadWrite)
	TObjectPtr<UCameraComponent> CameraComponent;

	EInteractionType PendingInteractionType = EInteractionType::Primary;

	EInteractionType ActiveInteractionType = EInteractionType::Primary;

	uint32 ActiveInteractionSerial = 0;

	bool bNotifyingStop = false;

	bool bApplyingServerStop = false;

	FTimerHandle InteractionRangeTimer;

	FTimerHandle InteractionInitTimer;

	// Functions
	void StartInteractionRangeTimer();

	void CheckInteractionRange();

	void CompleteInteraction(bool bNotify);

	UFUNCTION(Server, Reliable)
	void Server_TrackInteraction(UInteractableComponent* Target, EInteractionType Type, uint32 Serial);

	UFUNCTION(Server, Reliable)
	void Server_StopInteraction(UInteractableComponent* Target, EInteractionType Type, uint32 Serial);

	UFUNCTION(Client, Reliable)
	void Client_ConfirmInteraction(UInteractableComponent* Target, EInteractionType Type, uint32 Serial);

	UFUNCTION(Client, Reliable)
	void Client_StopInteraction(UInteractableComponent* Target, EInteractionType Type, uint32 Serial);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RefreshInteractionDisplay();

	UFUNCTION()
	void PerformInteractionCheck();

	UFUNCTION()
	void FoundInteractable(AActor* NewInteractable, UInteractableComponent* NewInteractableComp);

	UFUNCTION()
	void NotFoundInteractable();

	/* Called when player presses the interact input.
	 * Starts interaction with the currently focused interactable.
	 * Behaviour:
	 * - If interaction duration is zero → Interact() executes instantly
	 * - Otherwise a timer is started and interaction progress begins
	 */
	UFUNCTION()
	void BeginInteract(EInteractionType Type);

	/**
	* Called when player releases the interact input.
	* Only affects interactables that require holding the button
	* (InteractableData.bHoldToInteract == true).
	* This cancels the current interaction progress and clears the interaction timer.
	* Does NOT stop an already completed interaction.
	*/
	UFUNCTION()
	void EndInteract(EInteractionType Type);

	/**
	* Executes the interaction once its duration timer finishes
	* or immediately for instant interactions.
	* This calls InteractableComponent->Interact().
	*/
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact();

	UFUNCTION()
	void InteractNotify();

	UFUNCTION()
	void EndInteractNotify();

	UFUNCTION()
	void BusyNotify();

	UFUNCTION(Category = "Invenza|Interaction", BlueprintCallable)
	void CollectInteractionActions();

	UFUNCTION(Category = "Invenza|Interaction", BlueprintCallable, BlueprintPure)
	TArray<FInteractionDisplayEntry> BuildDisplayEntries(UInteractableComponent* Target) const;
};
