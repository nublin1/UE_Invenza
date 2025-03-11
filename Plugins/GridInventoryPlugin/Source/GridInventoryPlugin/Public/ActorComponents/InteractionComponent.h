// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/InteractionData.h"
#include "InteractionComponent.generated.h"

#pragma region delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBeginFocus, FInteractableData&, InteractableData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEndFocus, FInteractableData&, InteractableData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIteract, UInteractableComponent*, TargetInteractableComponent);
#pragma endregion

enum class EInteractableType : uint8;
class UInteractableComponent;
class UInputAction;
class IInteractableData;
class UCameraComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GRIDINVENTORYPLUGIN_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Delegates
	UPROPERTY(BlueprintAssignable)
	FBeginFocus BeginFocusDelegate;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FEndFocus EndFocusDelegate;
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FIteract IteractableDataDelegate;
	
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
	UPROPERTY(EditAnywhere, Category = "Character | Interaction")
	float InteractionCheckInterval;
	UPROPERTY(EditAnywhere, Category = "Character | Interaction")
	float InteractionCheckDistance;
	UPROPERTY(EditAnywhere, Category = "Character | Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel;
	UPROPERTY(EditAnywhere, Category = "Character | Interaction")
	float BaseEyeHeight;

	//
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> InteractAction;
	
	//
	FTimerHandle TimerHandle_Interaction;
	FInteractionData InteractionData;
	UPROPERTY(EditAnywhere, Category = "Character | Interaction")
	UInteractableComponent* TargetInteractableComponent;	
	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraComponent;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	void PerformInteractionCheck();
	void FoundInteractable(AActor *NewInteractable, UInteractableComponent* NewInteractableComp);
	void NotFoundInteractable();
	void BeginInteract();
	void EndInteract();
	void Interact();

	void IteractNotify();
	
	//Overrides
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
