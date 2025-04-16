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
	
	virtual void Interact(UInteractionComponent* InteractionComponent) override;
	virtual void StopInteract(UInteractionComponent* InteractionComponent) override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UItemCollection> ItemCollection;
	
	UPROPERTY()
	TObjectPtr<UUInventoryWidgetBase> InventoryWidget;
	UPROPERTY()
	TObjectPtr<UInvBaseContainerWidget> ContainerWidget;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void OnRegister() override;
	
	virtual void InitializeInteractionComponent() override;
	UFUNCTION()
	void InitializeItemCollection();
	
	virtual void UpdateInteractableData() override;
	UFUNCTION(BlueprintCallable)
	virtual void FindContainerWidget();

};
