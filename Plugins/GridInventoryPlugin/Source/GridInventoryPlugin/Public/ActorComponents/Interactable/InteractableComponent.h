// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interactable/InteractableData.h"
#include "InteractableComponent.generated.h"


UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class GRIDINVENTORYPLUGIN_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInteractableComponent();

	UFUNCTION()
	virtual void BeginFocus();
	UFUNCTION()
	virtual void EndFocus();
	UFUNCTION()
	virtual void BeginInteract(UInteractionComponent* InteractionComponent);
	UFUNCTION()
	virtual void EndInteract(UInteractionComponent* InteractionComponent);
	UFUNCTION()
	virtual void Interact(UInteractionComponent* InteractionComponent);
	UFUNCTION()
	virtual void StopInteract(UInteractionComponent* InteractionComponent);

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FInteractableData InteractableData;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY()
	bool bIsInteracting = false;

	UPROPERTY()
	TObjectPtr<UInteractionComponent> CurrentInteractionComponent = nullptr;

	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION()
	virtual void InitializeInteractionComponent();
	UFUNCTION()
	virtual void UpdateInteractableData();


};
