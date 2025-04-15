//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/ItemCollection.h"

#include "ActorComponents/Items/itemBase.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"


UItemCollection::UItemCollection()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UItemCollection::AddItem(UItemBase* NewItem, FItemMapping ItemMapping)
{
	ItemLocations.FindOrAdd(TStrongObjectPtr<UItemBase>(NewItem)).Add(ItemMapping);
	//UE_LOG(LogTemp, Warning, TEXT("Widget already exists for Item: %s"), *NewItem->GetName());
}

void UItemCollection::RemoveItem(UItemBase* Item, UUInventoryWidgetBase* Container)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem: Item is null."));
		return;
	}
	if (!Container)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem: Container is null."));
		return;
	}
	auto Mappings = ItemLocations.Find(TStrongObjectPtr<UItemBase>(Item));
	if (!Mappings)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem: Item %s not found in ItemContainers."), *Item->GetName());
		return;
	}
	
	int32 RemovedCount = Mappings->RemoveAll([Container](const FItemMapping& Mapping)
	{
		return Mapping.InventoryWidgetBaseLink == Container;
	});

	auto MappingsTwo = ItemLocations.Find(TStrongObjectPtr<UItemBase>(Item));
	if (MappingsTwo && MappingsTwo->Num() == 0)
	{
		ItemLocations.Remove(TStrongObjectPtr<UItemBase>(Item));
	}
}

void UItemCollection::RemoveItemFromAllContainers(UItemBase* Item)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemFromAllContainers: Item is null."));
		return;
	}
	TArray<FItemMapping>* Mappings = ItemLocations.Find(TStrongObjectPtr<UItemBase>(Item));
	if (!Mappings)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemFromAllContainers: Item %s not found in ItemLocations."), *Item->GetName());
		return;
	}
	
	for (int32 i = Mappings->Num() - 1; i >= 0; --i)
	{
		const FItemMapping& Mapping = (*Mappings)[i];
		if (Mapping.InventoryWidgetBaseLink)
		{
			//UE_LOG(LogTemp, Warning, TEXT("InventoryWidgetBaseLink %s"), *Mapping.InventoryWidgetBaseLink->GetName());
			Mapping.InventoryWidgetBaseLink->HandleRemoveItemFromContainer(Item);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("RemoveItemFromAllContainers: Invalid container in mapping for item %s."), *Item->GetName());
		}
	}
	
	ItemLocations.Remove(TStrongObjectPtr<UItemBase>(Item));
}

FItemMapping* UItemCollection::FindItemMappingForItemInContainer(UItemBase* TargetItem, UUInventoryWidgetBase* InContainer)
{
	if (!TargetItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("FindItemMappingForItemInContainer: TargetItem is null."));
		return nullptr;
	}
	if (!InContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("FindItemMappingForItemInContainer: InContainer is null."));
		return nullptr;
	}
	
	TArray<FItemMapping>* Mappings = ItemLocations.Find(TStrongObjectPtr<UItemBase>(TargetItem));
	if (!Mappings)
	{
		UE_LOG(LogTemp, Warning, TEXT("FindItemMappingForItemInContainer: No mapping found for item %s."), *TargetItem->GetName());
		return nullptr;
	}
	
	for (FItemMapping& Mapping : *Mappings)
	{
		if (Mapping.InventoryWidgetBaseLink == InContainer)
		{
			return &Mapping;
		}
	}
	return nullptr;
}

bool UItemCollection::HasItemInContainer(UItemBase* Item, USlotbasedInventoryWidget* Container) const
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("HasItemInContainer: Item is null!"));
		return false;
	}
	
	const TArray<FItemMapping>* Mappings = ItemLocations.Find(TStrongObjectPtr<UItemBase>(Item));
	if (!Mappings)
	{
		return false;
	}
	for (const FItemMapping& Mapping : *Mappings)
	{
		if (Mapping.InventoryWidgetBaseLink == Container)
		{
			return true;
		}
	}

	return false;
}

TArray<TObjectPtr<UInventorySlot>> UItemCollection::CollectOccupiedSlotsByContainer(USlotbasedInventoryWidget* InContainer)
{
	TArray<TObjectPtr<UInventorySlot>> OccupiedSlots;
	if (!InContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetOccupiedSlotsForContainer: TargetContainer is null."));
		return OccupiedSlots;
	}
	
	if (ItemLocations.IsEmpty())
		return OccupiedSlots;

	for (const auto& Pair : ItemLocations)
	{
		for (const FItemMapping& Mapping : Pair.Value)
		{
			if (Mapping.InventoryWidgetBaseLink == InContainer && !Mapping.ItemSlots.IsEmpty())
			{
				OccupiedSlots.Append(Mapping.ItemSlots);
			}
		}
	}

	OccupiedSlots.RemoveAll([](UInventorySlot* Slot) { return Slot == nullptr; });
	return OccupiedSlots;
}

UItemBase* UItemCollection::GetItemFromSlot(UInventorySlot* TargetSlot, UUInventoryWidgetBase* TargetContainer) const
{
	if (!TargetSlot || !TargetContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetItemFromSlot: %s"), !TargetSlot ? TEXT("TargetSlot is null.") : TEXT("TargetContainer is null."));
		return nullptr;
	}
    
	for (const auto& Pair : ItemLocations)
	{
		for (const FItemMapping& Mapping : Pair.Value)
		{
			if (Mapping.InventoryWidgetBaseLink == TargetContainer && Mapping.ItemSlots.Contains(TargetSlot))
			{
				return Pair.Key.Get();
			}
		}
	}
    
	//UE_LOG(LogTemp, Warning, TEXT("GetItemFromSlot: No item found for slot %s in container %s"), *TargetSlot->GetName(), *TargetContainer->GetName());
	return nullptr;
}

TArray<UItemBase*> UItemCollection::GetAllItemsByContainer(UUInventoryWidgetBase* TargetContainer) const
{
	TArray<UItemBase*> Result;
    
	if (!TargetContainer || ItemLocations.IsEmpty())
	{
		return Result;
	}
	
	for (const auto& Pair : ItemLocations)
	{
		auto Item = Pair.Key;
		const TArray<FItemMapping>& Mappings = Pair.Value;
		
		for (const FItemMapping& Mapping : Mappings)
		{
			if (Mapping.InventoryWidgetBaseLink == TargetContainer)
			{
				if (!Result.Contains(Item.Get()))
				{
					Result.Add(Item.Get());
				}
				break;
			}
		}
	}

	return Result;
}

TArray<UItemBase*> UItemCollection::GetAllSameItemsInContainer(UUInventoryWidgetBase* TargetContainer,
                                                               UItemBase* ReferenceItem) const
{
	TArray<UItemBase*> SameItems;
	if (!TargetContainer || !ReferenceItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetAllSameItemsInContainer: %s"),
			!TargetContainer ? TEXT("TargetContainer is null.") : TEXT("ReferenceItem is null."));
		return SameItems;
	}

	auto RefName = ReferenceItem->GetItemRef().ItemTextData.Name;
	for (const auto& Pair : ItemLocations)
	{
		auto Item = Pair.Key;
		if (Item && Item->GetItemRef().ItemTextData.Name.EqualTo(RefName))
		{
			for (const FItemMapping& Mapping : Pair.Value)
			{
				if (Mapping.InventoryWidgetBaseLink == TargetContainer)
				{
					SameItems.AddUnique(Item.Get());
					break;
				}
			}
		}
	}
    
	return SameItems;
}

UInventoryItemWidget* UItemCollection::GetItemLinkedWidgetForSlot(USlotbasedInventorySlot* _ItemSlot) const
{
	if (!_ItemSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetItemLinkedWidget: TargetItem or InContainer is null."));
		return nullptr;
	}
	
	for (const auto& Pair : ItemLocations)
	{
		const TArray<FItemMapping>& Mappings = Pair.Value;
		for (const FItemMapping& Mapping : Mappings)
		{
			if (Mapping.ItemSlots.Contains(_ItemSlot))
			{
				return Mapping.ItemVisualLinked;
			}
		}
	}
	return nullptr;
}