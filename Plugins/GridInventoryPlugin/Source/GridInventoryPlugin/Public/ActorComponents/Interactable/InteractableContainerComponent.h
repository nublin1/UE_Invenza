//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "InteractableContainerComponent.generated.h"

class UInvBaseContainerWidget;
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
	FName ContainerWidgetName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UItemCollection> ItemCollection;

	UPROPERTY()
	TObjectPtr<UBaseInventoryWidget> InventoryWidget;
	UPROPERTY()
	TObjectPtr<UInvBaseContainerWidget> ContainerWidget;
	UPROPERTY()
	bool bContainerIsOpen = false;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void OnRegister() override;

	UFUNCTION()
	void InitializeContainerComponent();
	UFUNCTION()
	void InitializeItemCollection();

	
	virtual void UpdateInteractableData() override;
	UFUNCTION()
	virtual UBaseInventoryWidget* FindContainerWidget();

	
	
};
