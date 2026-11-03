// Nublin Studio 2026 All Rights Reserved.

#include "UI/Inventory/InventoryViewEntry.h"

#include "Data/Inventory/InventoryBase.h"
#include "Interface/Interaction/ObjectDataProvider.h"

UInventoryBase* UInventoryViewEntry::GetSourceInventory() const
{
	return SourceInventory.Get();
}

void UInventoryViewEntry::Initialize(UInventoryBase* Inventory, UObject* InItem)
{
	SourceInventory = Inventory;
	Item = InItem;
	RefreshDisplayData();
}

void UInventoryViewEntry::RefreshDisplayData()
{
	UObject* CurrentItem = Item.Get();
	if (SourceInventory.IsValid() && IsValid(CurrentItem) && CurrentItem->Implements<UObjectDataProvider>())
	{
		const FItemMetaData Data = IObjectDataProvider::Execute_GetItemRef(CurrentItem);
		DisplayName = Data.ItemTextData.DisplayName;
		Icon = Data.ItemAssetData.Icon;
		Category = Data.ItemCategory;
		Quantity = IObjectDataProvider::Execute_GetQuantity(CurrentItem);
	}
	else
	{
		DisplayName = FText::GetEmpty();
		Icon = nullptr;
		Category = FGameplayTag();
		Quantity = 0;
	}
	OnEntryUpdated.Broadcast();
}
