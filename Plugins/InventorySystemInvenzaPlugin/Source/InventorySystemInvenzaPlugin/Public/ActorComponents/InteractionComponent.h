// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/InteractionData.h"
#include "Settings/InvnzaSettings.h"
#include "InteractionComponent.generated.h"

#pragma region delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBeginFocus, FInteractableData&, InteractableData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEndFocus, FInteractableData&, InteractableData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIteract, UInteractableComponent*, TargetInteractableComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStopIteract, UInteractableComponent*, TargetInteractableComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEndIteract, UInteractableComponent*, TargetInteractableComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionProgressDelegate, float, Progress);
#pragma endregion

class UItemBase;
enum class EInteractableType : uint8;
class UInteractableComponent;
class UInputAction;
class IInteractableData;
class UCameraComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INVENTORYSYSTEMINVENZAPLUGIN_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Delegates
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FBeginFocus BeginFocusDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FEndFocus EndFocusDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FIteract IteractableDataDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FStopIteract StopIteractDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FEndIteract EndIteractDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FInteractionProgressDelegate OnInteractionProgress;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|UI")
	FUISettings RegularSettings;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInteractionComponent();

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
	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraComponent;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void PerformInteractionCheck();
	UFUNCTION()
	void FoundInteractable(AActor *NewInteractable, UInteractableComponent* NewInteractableComp);
	UFUNCTION()
	void NotFoundInteractable();
	UFUNCTION()
	void BeginInteract();
	UFUNCTION()
	void EndInteract();
	UFUNCTION(BlueprintCallable)
	void Interact();
public:
	UFUNCTION()
	void StopInteract();
protected:
	UFUNCTION()
	void IteractNotify();
	UFUNCTION()
	void EndIteractNotify();

	//Overrides
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
};
