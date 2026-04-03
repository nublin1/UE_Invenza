//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "ContainerComponent.generated.h"

class UInvenzaBaseWidget;
class UInvBaseContainerWidget;
/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UContainerComponent : public UInteractableComponent
{
	GENERATED_BODY()

public:
	UContainerComponent();

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

public:	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	virtual void BeginFocus() override;
	virtual void EndFocus() override;
	
	virtual void Interact(UInteractionComponent* InteractionComponent) override;
	virtual void StopInteract(UInteractionComponent* InteractionComponent) override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Container")
	FInventoryStartupData InventoryStartupData;

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Container")
	bool bDestroyWhenEmpty = false;

	// Data
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TMap<FString, TObjectPtr<UInventoryBase>> Inventories;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CachedMesh;

	//Refs
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory|Container")
	TObjectPtr<UItemCollection> ItemCollection;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	

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
