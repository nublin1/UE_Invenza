//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/ItemCollection.h"

#include "ActorComponents/Items/itemBase.h"
#include "UI/Inventory/BaseInventoryWidget.h"


UItemCollection::UItemCollection()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UItemCollection::AddItem(UItemBase* NewItem,  FItemMapping ItemMapping)
{
	ItemLocations.FindOrAdd(NewItem).Add(ItemMapping);
	//UE_LOG(LogTemp, Warning, TEXT("Widget already exists for Item: %s"), *NewItem->GetName());
}

void UItemCollection::RemoveItem(UItemBase* Item, UBaseInventoryWidget* Container)
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
	TArray<FItemMapping>* Mappings = ItemLocations.Find(Item);
	if (!Mappings)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem: Item %s not found in ItemContainers."), *Item->GetName());
		return;
	}
	
	int32 RemovedCount = Mappings->RemoveAll([Container](const FItemMapping& Mapping)
	{
		return Mapping.BaseInventoryWidgetLink == Container;
	});
}

void UItemCollection::RemoveItemFromAllContainers(UItemBase* Item)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemFromAllContainers: Item is null."));
		return;
	}
	TArray<FItemMapping>* Mappings = ItemLocations.Find(Item);
	if (!Mappings)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemFromAllContainers: Item %s not found in ItemLocations."), *Item->GetName());
		return;
	}
	for (const FItemMapping& Mapping : *Mappings)
	{
		if (Mapping.BaseInventoryWidgetLink)
		{
			Mapping.BaseInventoryWidgetLink->HandleRemoveItem(Item);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("RemoveItemFromAllContainers: Invalid container in mapping for item %s."), *Item->GetName());
		}
	}
	Mappings->Empty();
	ItemLocations.Remove(Item);
}

FItemMapping* UItemCollection::FindItemMappingForItemInContainer(UItemBase* TargetItem, UBaseInventoryWidget* InContainer)
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
	
	TArray<FItemMapping>* Mappings = ItemLocations.Find(TargetItem);
	if (!Mappings)
	{
		UE_LOG(LogTemp, Warning, TEXT("FindItemMappingForItemInContainer: No mapping found for item %s."), *TargetItem->GetName());
		return nullptr;
	}
	
	for (FItemMapping& Mapping : *Mappings)
	{
		if (Mapping.BaseInventoryWidgetLink == InContainer)
		{
			return &Mapping;
		}
	}
	return nullptr;
}

bool UItemCollection::HasItemInContainer(UItemBase* Item, UBaseInventoryWidget* Container) const
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("HasItemInContainer: Item is null!"));
		return false;
	}
	
	const TArray<FItemMapping>* Mappings = ItemLocations.Find(Item);
	if (!Mappings)
	{
		return false;
	}
	for (const FItemMapping& Mapping : *Mappings)
	{
		if (Mapping.BaseInventoryWidgetLink == Container)
		{
			return true;
		}
	}

	return false;
}

TArray<TObjectPtr<UBaseInventorySlot>> UItemCollection::CollectOccupiedSlotsByContainer(UBaseInventoryWidget* InContainer)
{
	TArray<TObjectPtr<UBaseInventorySlot>> OccupiedSlots;
	if (!InContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetOccupiedSlotsForContainer: TargetContainer is null."));
		return OccupiedSlots;
	}

	for (const auto& Pair : ItemLocations)
	{
		for (const FItemMapping& Mapping : Pair.Value)
		{
			if (Mapping.BaseInventoryWidgetLink == InContainer)
			{
				OccupiedSlots.Append(Mapping.ItemSlots);
			}
		}
	}

	OccupiedSlots.RemoveAll([](UBaseInventorySlot* Slot) { return Slot == nullptr; });
	return OccupiedSlots;
}

UItemBase* UItemCollection::GetItemFromSlot(UBaseInventorySlot* TargetSlot, UBaseInventoryWidget* TargetContainer) const
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
			if (Mapping.BaseInventoryWidgetLink == TargetContainer && Mapping.ItemSlots.Contains(TargetSlot))
			{
				return Pair.Key;
			}
		}
	}
    
	//UE_LOG(LogTemp, Warning, TEXT("GetItemFromSlot: No item found for slot %s in container %s"), *TargetSlot->GetName(), *TargetContainer->GetName());
	return nullptr;
}

TArray<UItemBase*> UItemCollection::GetAllItemsByContainer(UBaseInventoryWidget* TargetContainer) const
{
	TArray<UItemBase*> Result;
    
	if (!TargetContainer || ItemLocations.IsEmpty())
	{
		return Result;
	}
	
	for (const auto& Pair : ItemLocations)
	{
		UItemBase* Item = Pair.Key;
		const TArray<FItemMapping>& Mappings = Pair.Value;
		
		for (const FItemMapping& Mapping : Mappings)
		{
			if (Mapping.BaseInventoryWidgetLink == TargetContainer)
			{
				if (!Result.Contains(Item))
				{
					Result.Add(Item);
				}
				break;
			}
		}
	}

	return Result;
}

TArray<UItemBase*> UItemCollection::GetAllSameItemsInContainer(UBaseInventoryWidget* TargetContainer,
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
		UItemBase* Item = Pair.Key;
		if (Item && Item->GetItemRef().ItemTextData.Name.EqualTo(RefName))
		{
			for (const FItemMapping& Mapping : Pair.Value)
			{
				if (Mapping.BaseInventoryWidgetLink == TargetContainer)
				{
					SameItems.AddUnique(Item);
					break;
				}
			}
		}
	}
    
	return SameItems;
}

UInventoryItemWidget* UItemCollection::GetItemLinkedWidgetForSlot(UBaseInventorySlot* _ItemSlot) const
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