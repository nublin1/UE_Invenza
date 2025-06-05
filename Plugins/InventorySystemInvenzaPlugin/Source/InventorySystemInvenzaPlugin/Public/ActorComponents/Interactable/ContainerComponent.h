//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "ContainerComponent.generated.h"

class UBaseUserWidget;
class UInvBaseContainerWidget;
/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UContainerComponent : public UInteractableComponent
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UContainerComponent();
	virtual void BeginFocus() override;
	virtual void EndFocus() override;
	
	virtual void Interact(UInteractionComponent* InteractionComponent) override;
	virtual void StopInteract(UInteractionComponent* InteractionComponent) override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory|Container")
	TObjectPtr<UItemCollection> ItemCollection;
	
	UPROPERTY()
	TObjectPtr<UUInventoryWidgetBase> InventoryWidget;
	UPROPERTY()
	TObjectPtr<UInvBaseContainerWidget> ContainerWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Container")
	FVector2D InventorySize = FVector2D(5,4); // Only for SlotBased 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Container")
	bool bDestroyWhenEmpty = false;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void OnRegister() override;

	UFUNCTION()
	virtual void ContainerWidgetVisibilityChanged(ESlateVisibility NewVisibility);
	
	virtual void InitializeInteractionComponent() override;
	UFUNCTION()
	void InitializeItemCollection();
	
	virtual void UpdateInteractableData() override;
	UFUNCTION(BlueprintCallable, Category = "Inventory|Container")
	virtual void FindContainerWidget();
	
	UFUNCTION()
	virtual void DestroyWhenEmpty();
};
