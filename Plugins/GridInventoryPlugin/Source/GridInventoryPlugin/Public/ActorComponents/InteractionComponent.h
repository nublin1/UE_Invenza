// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/InteractionData.h"
#include "Interfaces/InteractionInterface.h"
#include "InteractionComponent.generated.h"

#pragma region delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBeginFocus,FInteractionData&, InteractableData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEndFocus, FInteractionData&, InteractableData);
#pragma endregion


class UInputAction;
class IInteractionInterface;
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
	FInteractionData InteractionData;
	UPROPERTY(EditAnywhere, Category = "Character | Interaction")
	TScriptInterface<IInteractionInterface> TargetInteractable;	
	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraComponent;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	void PerformInteractionCheck();
	void FoundInteractable(AActor *NewInteractable);
	void NotFoundInteractable();
	void BeginInteract();
	void EndInteract();
	void Interact();
	
	//Overrides
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
