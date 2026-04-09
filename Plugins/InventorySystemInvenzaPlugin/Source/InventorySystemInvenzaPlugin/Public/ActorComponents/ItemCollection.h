//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/Map.h"
#include "UObject/ObjectPtr.h"
#include "Engine/DataTable.h"
#include "Data/Items/ItemBase.h"
#include "Data/Inventory/InventoryTypes.h"
#include "ItemCollection.generated.h"


class UIInventoryManager;
struct FItemMappingSaveData;
struct FItemSaveData;
class UUInventoryWidgetBase;
class UInventorySlot;
class UInventoryItemWidget;
class USlotbasedInventorySlot;
class USlotbasedInventoryWidget;
class UItemBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UItemCollection : public UActorComponent
{
	GENERATED_BODY()

public:
	UItemCollection();

protected:
	virtual void BeginPlay() override;

public:
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Item Collection")
	float CalculateAvailableMoney();

	TMap<TObjectPtr<UItemBase>, FItemMappingArrayWrapper> GetItemLocations() const {return ItemLocations;}

	int32 GetTotalItemCountInContainer(FString InvID);
	TArray<UItemBase*> GetAllItemsByContainer(FString InvID);
	TArray<UItemBase*> GetAllSameItemsInContainer(FString InvID, UItemBase* ReferenceItem) const;
	TArray<FItemMapping> GetAllMappingsByContainer(const FString& InvID);
	TMap<UItemBase*, FItemMapping*> GetItemsWithMappingsByContainer(const FString& InvID);
	TArray<UItemBase*> GetAllItemsByCategory(EItemCategory ItemCategory);
	UItemBase* GetItemFromSlot(UInventorySlotData* TargetSlotData, const FString& InventoryID);
	
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	FItemMapping& AddItem(UItemBase* NewItem, const FItemMapping& ItemMapping);
	UFUNCTION(BlueprintCallable, Category="Item Collection|Item Management")
	void RemoveItem(UItemBase* Item, FString ContainerID);
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void RemoveItemFromAllContainers(UItemBase* Item);
	
	FItemMapping* FindItemMappingByContainerName(UItemBase* Item, FString InventoryID);

	UFUNCTION(BlueprintCallable)
	bool ItemHasInventory(UItemBase* Item, FString InventoryID);

	//
	UFUNCTION()
	void SetInvManager(UIInventoryManager* NewManager) {InvManager = NewManager;}

	UFUNCTION(BlueprintCallable)
	void SerializeForSave(TArray<FItemSaveEntry>& OutData, const TArray<FString>& InventoryFilter);
	UFUNCTION(BlueprintCallable)
	void DeserializeFromSave(const TArray<FItemSaveEntry>& InData, UInventoryBase* InInventory);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY()
	TObjectPtr<UIInventoryManager> InvManager = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<TObjectPtr<UItemBase>, FItemMappingArrayWrapper> ItemLocations;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
