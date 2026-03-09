// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "Data/EquipmentStructures.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/InventorySlotData.h"
#include "EquipmentInventory.generated.h"

class UEquipmentSlotData;
class UInvBaseContainerWidget;
struct FInventorySlotData;
struct FItemMapping;
class UItemBase;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INVENTORYSYSTEMINVENZAPLUGIN_API UEquipmentInventory : public UInventoryBase
{
	GENERATED_BODY()
	
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItem, FName, SlotName, UItemBase*, Item);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnequippedItem, FName, SlotName, UItemBase*, Item);
#pragma endregion

public:
	UEquipmentInventory();

protected:

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable, BlueprintCallable,Category = "Equipment|Events")
	FOnEquippedItem OnEquippedItem;
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable,Category = "Equipment|Events")
	FOnUnequippedItem OnUnequippedItem;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void InitInventory(UItemCollection* ItemCollectionRef, FVector2D NewSize ) override;

	UFUNCTION(Category = "Equipment|Initialization")
	virtual void InitializeSlotsFromTable();
	
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	void HandleReplaceItem(TArray<UInventorySlotData*> OldItemSlots, TArray<UInventorySlotData*> NewItemSlots, UItemBase* Item);
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	void HandleItemEquippedFromMapping(FItemMapping ItemSlots, UItemBase* Item);
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	void EquipItemToSlot(FName SlotName, UItemBase* Item);
	
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck = false) override;
	
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	void UnequipItemFromSlot(FName SlotName, UItemBase* Item);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	/** Initial items with their quantities */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Config")
	TArray<FInitItemsEntry> InitialItems;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Equipment|Config")
	TObjectPtr<UDataTable> SlotDefinitionTable;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Slots")
	TMap<FName, TObjectPtr<UEquipmentSlotData>> EquipmentSlots;

	//
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category="Equipment|Config")
	TObjectPtr<UInvBaseContainerWidget> CharacterEquipmentWidget = nullptr;

	// Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName InventoryContainerID = NAME_None; // Uniq ID
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION(Category = "Equipment|Initialization")
	virtual void BindWidgetsToSlots();
	
	UFUNCTION(Category = "Equipment|Management")
	void ResourceAmountChanged(int32 AmountChanged, UItemBase* Item);
	
};
