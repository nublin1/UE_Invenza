// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Interaction/InteractionData.h"
#include "Settings/InvnzaSettings.h"
#include "InteractionComponent.generated.h"

class UIInventoryManager;
class UItemBase;
enum class EInteractableType : uint8;
class UInteractableComponent;
class UInputAction;
class IInteractableData;
class UCameraComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginFocus, FInteractableData&, InteractableData);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndFocus, FInteractableData&, InteractableData);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, UInteractableComponent*, TargetInteractableComponent);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStopInteract, UInteractableComponent*, TargetInteractableComponent);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndInteract, UInteractableComponent*, TargetInteractableComponent);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionProgress, float, Progress);
#pragma endregion

public:
	UInteractionComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Delegates
	UPROPERTY(BlueprintAssignable, Category="Interaction|Events")
	FOnBeginFocus OnBeginFocus;
	UPROPERTY(BlueprintAssignable, Category="Interaction|Events")
	FOnEndFocus OnEndFocus;
	UPROPERTY(BlueprintAssignable, Category="Interaction|Events")
	FOnInteract OnInteract;
	UPROPERTY(BlueprintAssignable, Category="Interaction|Events")
	FOnStopInteract OnStopInteract;
	UPROPERTY(BlueprintAssignable, Category="Interaction|Events")
	FOnEndInteract OnEndInteract;
	UPROPERTY(BlueprintAssignable, Category="Interaction|Events")
	FOnInteractionProgress OnInteractionProgress;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void InitInteractionComponent();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
#pragma region Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Trace")
	float InteractionCheckInterval;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Trace")
	float InteractionCheckDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel;
#pragma endregion

#pragma region Input
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Input")
	TObjectPtr<UInputAction> InteractAction;
#pragma endregion

	//
	UPROPERTY()
	FTimerHandle TimerHandle_Interaction;
	UPROPERTY()
	float InteractionStartTime = 0.0f;
	UPROPERTY()
	FInteractionData InteractionData;
	UPROPERTY(VisibleAnywhere, Category = "Interaction|State")
	UInteractableComponent* TargetInteractableComponent;
	UPROPERTY(VisibleAnywhere, Category = "Interaction|State")
	UInteractableComponent* CurrentInteractableComponent;

	//Refs
	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraComponent;
	UPROPERTY()
	TObjectPtr<UIInventoryManager> InventoryManager;

	//====================================================================
	// FUNCTIONS
	//====================================================================
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
	void BeginInteract();
	
	/**
	* Called when player releases the interact input.
	* Only affects interactables that require holding the button
	* (InteractableData.bHoldToInteract == true).
	* This cancels the current interaction progress and clears the interaction timer.
	* Does NOT stop an already completed interaction.
	*/
	UFUNCTION()
	void EndInteract();

	/**
 	* Executes the interaction once its duration timer finishes
 	* or immediately for instant interactions.
 	* This calls InteractableComponent->Interact().
 	*/
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact();

public:
	/**
 	* Stops an already active interaction with the current interactable.
 	* Used when:
 	* - pawn interacts again with the same object
 	* - interaction needs to be forcefully stopped
 	* Calls InteractableComponent->StopInteract().
 	*/
	UFUNCTION()
	void StopInteract();

protected:
	UFUNCTION()
	void InteractNotify();
	UFUNCTION()
	void EndInteractNotify();
};
