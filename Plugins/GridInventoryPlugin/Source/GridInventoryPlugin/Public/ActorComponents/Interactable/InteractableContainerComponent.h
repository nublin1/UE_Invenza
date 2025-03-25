// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "InteractableContainerComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRIDINVENTORYPLUGIN_API UInteractableContainerComponent : public UInteractableComponent
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInteractableContainerComponent();
	virtual void BeginFocus() override;
	virtual void EndFocus() override;
	
	UFUNCTION()
	virtual void Interact(UInteractionComponent* InteractionComponent) override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TargetWidgetName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UItemCollection> ItemCollection;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void OnRegister() override;

	UFUNCTION()
	void InitializeContainerComponent();
	void InitializeItemCollection();

	virtual void UpdateInteractableData() override;
	
};
