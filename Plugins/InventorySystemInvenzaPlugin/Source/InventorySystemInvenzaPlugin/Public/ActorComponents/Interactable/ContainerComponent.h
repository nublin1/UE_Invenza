//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "Interface/Interaction/LootContainerProvider.h"
#include "ContainerComponent.generated.h"

class UInvenzaBaseWidget;
class UInventoryContainerWidget;
/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UContainerComponent : public UInteractableComponent, public ILootContainerProvider
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

	virtual const TObjectPtr<UInventoryBase>& GetMainLootContainer() const override {return MainLootInventory;}
	virtual const TMap<FString, TObjectPtr<UInventoryBase>>& GetInventoriesToDisplay() const override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Container")
	FInventoryStartupData InventoryStartupData;

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Container")
	FGameplayTag MainLootContainerInvTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Container")
	bool bDestroyWhenEmpty = false;

	// Data
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInventoryBase> MainLootInventory;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TMap<FString, TObjectPtr<UInventoryBase>> Inventories;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CachedMesh;

	//Refs
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory|Container")
	TObjectPtr<UItemCollection> ItemCollectionRef;

	//====================================================================
	// FUNCTIONS
	//====================================================================	
	virtual void InitializeInteractionComponent() override;
	virtual void UpdateInteractableData() override;

	UFUNCTION(BlueprintCallable)
	void InitializeInventoryStartupData();

	UFUNCTION(BlueprintCallable)
	void SetupStartingResources();
		
	UFUNCTION()
	virtual void DestroyWhenEmpty(FItemMapping ItemSlots, UItemBase* Item);
};
