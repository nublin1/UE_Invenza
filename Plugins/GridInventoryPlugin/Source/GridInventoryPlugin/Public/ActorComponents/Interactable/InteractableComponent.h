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

	virtual void BeginFocus();
	virtual void EndFocus();
	virtual void BeginInteract(UInteractionComponent* InteractionComponent);
	virtual void EndInteract(UInteractionComponent* InteractionComponent);
	virtual void Interact(UInteractionComponent* InteractionComponent);

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FInteractableData InteractableData;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY()
	bool bIsInteracting = false;

	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION()
	virtual void InitializeInteractionComponent();
	UFUNCTION()
	virtual void UpdateInteractableData();


};
