//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/ItemCollection.h"

#include "ActorComponents/UIInventoryManager.h"
#include "Data//Items/itemBase.h"
#include "ActorComponents/SaveLoad/SaveLoadStructs.h"
#include "Data/Inventory/InventoryBase.h"
#include "Kismet/GameplayStatics.h"


UItemCollection::UItemCollection()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UItemCollection::BeginPlay()
{
	Super::BeginPlay();

	auto Manager = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->FindComponentByClass<UIInventoryManager>();
	if (Manager)
		InvManager = Manager;
}

int32 UItemCollection::GetTotalItemCountInContainer(FString InvID)
{
	if (ItemLocations.IsEmpty())
	{
		return 0;
	}
	
	int32 TotalItemCount = 0;
	for (const auto& Pair : ItemLocations)
	{
		auto Item = Pair.Key;
		const FItemMappingArrayWrapper& MappingArrayWrapper = Pair.Value;
		
		for (const FItemMapping& Mapping : MappingArrayWrapper.Mappings)
		{
			if (InvID == (Mapping.InventoryID))
			{
				TotalItemCount++;
				break;
			}
		}
	}

	return TotalItemCount;
}

TArray<UItemBase*> UItemCollection::GetAllItemsByContainer(FString InvID)
{
	TArray<TObjectPtr<UItemBase>> Result;

	if (ItemLocations.IsEmpty())
	{
		return Result;
	}

	for (const auto& Pair : ItemLocations)
	{
		auto Item = Pair.Key;
		const FItemMappingArrayWrapper& MappingArrayWrapper = Pair.Value;
		
		for (const FItemMapping& Mapping : MappingArrayWrapper.Mappings)
		{
			if (InvID == (Mapping.InventoryID))
			{
				Result.AddUnique(Item.Get());
				break;
			}
		}
	}

	return Result;
}

TArray<UItemBase*> UItemCollection::GetAllSameItemsInContainer(FString InvID, UItemBase* ReferenceItem) const
{
	TArray<UItemBase*> SameItems;
	if (InvID.IsEmpty() || !ReferenceItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetAllSameItemsInContainer: %s"),
			InvID.IsEmpty() ? TEXT("InvID is empty.") : TEXT("ReferenceItem is null."));
		return SameItems;
	}

	if (ItemLocations.IsEmpty())
		return SameItems;

	auto RefItemID = ReferenceItem->GetItemID();
	for (const auto& Pair : ItemLocations)
	{
		auto Item = Pair.Key;
		if (Item && Item->GetItemID() == RefItemID)
		{
			for (const FItemMapping& Mapping : Pair.Value.Mappings)
			{
				if (InvID == Mapping.InventoryID)
				{
					SameItems.AddUnique(Item.Get());
					break;
				}
			}
		}
	}
    
	return SameItems;
}

TArray<FItemMapping> UItemCollection::GetAllMappingsByContainer(const FString& InvID)
{
	TArray<FItemMapping> Result;

	if (ItemLocations.IsEmpty())
		return Result;

	for (const auto& Pair : ItemLocations)
	{
		for (const FItemMapping& Mapping : Pair.Value.Mappings)
		{
			if (Mapping.InventoryID == InvID)
			{
				Result.AddUnique(Mapping);
			}
		}
	}

	return Result;
}

TMap<UItemBase*, FItemMapping*> UItemCollection::GetItemsWithMappingsByContainer(const FString& InvID)
{
	TMap<UItemBase*, FItemMapping*> Result;

	if (ItemLocations.IsEmpty())
		return Result;

	for (auto& Pair : ItemLocations)
	{
		UItemBase* Item = Pair.Key;

		for (int32 i = 0; i < Pair.Value.Mappings.Num(); i++)
		{
			 FItemMapping* Mapping = &Pair.Value.Mappings[i];

			if (Mapping->InventoryID == InvID)
			{
				Result.Add(Item, Mapping);
			}
		}
	}

	return Result;
}

TArray<UItemBase*> UItemCollection::GetAllItemsByCategory(EItemCategory ItemCategory)
{
	TArray<UItemBase*> SameItems;
	for (const auto& Pair : ItemLocations)
	{
		auto Item = Pair.Key;
		if (Item->GetItemRef().ItemCategory == ItemCategory)
		{
			SameItems.Add(Item.Get());
		}
	}

	return SameItems;
}

UItemBase* UItemCollection::GetItemFromSlot(UInventorySlotData* TargetSlotData, const FString& InventoryID)
{
	if (!TargetSlotData || InventoryID.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetItemFromSlot: Invalid parameters."));
		return nullptr;
	}

	if (ItemLocations.IsEmpty())
		return nullptr;
	
	for (const auto& Pair : ItemLocations)
	{
		const UItemBase* Item = Pair.Key.Get();
		const FItemMappingArrayWrapper& Wrapper = Pair.Value;

		for (const FItemMapping& Mapping : Wrapper.Mappings)
		{
			if (Mapping.InventoryID != InventoryID)
				continue;

			if (Mapping.OccupiedSlots.Contains(TargetSlotData))
			{
				return const_cast<UItemBase*>(Item);
			}
		}
	}
    
	//UE_LOG(LogTemp, Warning, TEXT("GetItemFromSlot: No item found for slot %s in container %s"), *TargetSlotData->GetName(), *InventoryID);
	return nullptr;
}

FItemMapping& UItemCollection::AddItem(UItemBase* NewItem, const FItemMapping& ItemMapping)
{
	FItemMappingArrayWrapper& Wrapper = ItemLocations.FindOrAdd(NewItem);
	Wrapper.Mappings.Add(ItemMapping);
	return Wrapper.Mappings.Last(); 
}

void UItemCollection::RemoveItem(UItemBase* Item, FString ContainerID)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem: Item is null."));
		return;
	}

	if (ContainerID.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem: ContainerID is Empty."));
		return;
	}

	FItemMappingArrayWrapper* Wrapper = ItemLocations.Find(Item);
	if (!Wrapper)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem: Item %s not found in ItemLocations."), *Item->GetName());
		return;
	}

	const int32 RemovedCount = Wrapper->Mappings.RemoveAll(
		[&](const FItemMapping& Mapping)
		{
			return Mapping.InventoryID == ContainerID;
		});

	if (RemovedCount == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem: No mapping removed for container %s"), *ContainerID);
	}

	if (Wrapper->Mappings.IsEmpty())
	{
		ItemLocations.Remove(Item);
	}
}

void UItemCollection::RemoveItemFromAllContainers(UItemBase* Item)
{
	if (!Item)
		return;

	auto Wrapper = ItemLocations.Find(Item);
	if (!Wrapper)
		return;

	TArray<FItemMapping> MappingsCopy = Wrapper->Mappings;

	for (const FItemMapping& Mapping : MappingsCopy)
	{
		if (UInventoryBase* Container = InvManager->GetInventoryByID(Mapping.InventoryID))
		{
			Container->HandleRemoveItem(Item, Item->GetQuantity());
		}
	}
}

FItemMapping* UItemCollection::FindItemMappingByContainerName(UItemBase* Item, FString InventoryID)
{
	FItemMappingArrayWrapper* Wrapper = ItemLocations.Find(Item);
	if (!Wrapper) return nullptr;

	for (FItemMapping& Mapping : Wrapper->Mappings)
	{
		if (Mapping.InventoryID == InventoryID)
		{
			return &Mapping;
		}
	}

	return nullptr;
}

bool UItemCollection::ItemHasInventory(UItemBase* Item, FString InventoryID)
{
	const FItemMappingArrayWrapper* Wrapper = ItemLocations.Find(Item);
	if (!Wrapper) return false;

	return Wrapper->Mappings.ContainsByPredicate(
		[InventoryID](const FItemMapping& Mapping)
		{
			return Mapping.InventoryID == InventoryID;
		});
}


/*UInventoryItemWidget* UItemCollection::GetItemLinkedWidgetForSlot(FInventorySlotData ItemSlotData)
{	
	for (const auto& Pair : ItemLocations)
	{
		const FItemMappingArrayWrapper& MappingArrayWrapper = Pair.Value;
		for (const FItemMapping& Mapping : MappingArrayWrapper.Mappings)
		{
			if (Mapping.OccupiedSlots.Contains(ItemSlotData))
			{
				return Mapping.ItemVisualLinked;
			}
		}
	}
	return nullptr;
}*/


void UItemCollection::SerializeForSave(TArray<FItemSaveEntry>& OutData)
{
	OutData.Empty();

	for (const auto& Pair : ItemLocations)
	{
		FItemSaveData Key(Pair.Key.Get());
		//UE_LOG(LogTemp, Warning, TEXT("ItemID: %s"), *Pair.Key->GetItemID().ToString());
		TArray<FItemMappingSaveData> SaveMappings;

		for (const FItemMapping& Mapping : Pair.Value.Mappings)
		{
			auto ContainerType = Mapping.InventoryType;
			if (ContainerType != EInventoryType::VendorInventory && ContainerType!= EInventoryType::ContainerInventory)
			{
				FItemMappingSaveData Data;
				Data.InitializeFromMapping(Mapping);
				SaveMappings.Add(Data);
			}
		}

		FItemSaveEntry ItemSaveEntry;
		ItemSaveEntry.Item = Key;
		ItemSaveEntry.Containers = SaveMappings;

		OutData.Add(ItemSaveEntry);
	}
}

void UItemCollection::DeserializeFromSave(TArray<FItemSaveEntry> InData)
{
	ItemLocations.Empty();

	/*for (const auto& Data : InData)
	{
		UItemBase* Item = UItemFactory::CreateItemByID(GetOwner(), Data.Item.ItemID, Data.Item.Quantity);
		if (!Item) continue;

		FItemMappingArrayWrapper RestoredMappingArrayWrapper;
		for (FItemMappingSaveData SaveMapping : Data.Containers)
		{
			FItemMapping Mapping;
			Mapping.InventoryID = SaveMapping.InventoryContainerName;
			Mapping.InventoryType = SaveMapping.InventoryType;
			for (auto SlotSaveData : SaveMapping.SlotSaveDatas)
			{
				FInventorySlotData SlotData;
				SlotData.SlotName = SlotSaveData.SlotName;
				SlotData.SlotPosition = SlotSaveData.SlotPosition;
				Mapping.OccupiedSlots.Add(SlotData);
			}

			RestoredMappingArrayWrapper.Mappings.Add(Mapping);
		}

		ItemLocations.Add(Item, RestoredMappingArrayWrapper);

		for (auto RestoredMapping : RestoredMappingArrayWrapper.Mappings)
		{
			if (UWidget* FoundWidget = InvManager->GetCoreHUDWidget()->GetWidgetFromName(RestoredMapping.InventoryID))
			{
				if (!IsValid(FoundWidget))
					continue;
				
				if (auto InvBaseContainerWidget = Cast<UInvBaseContainerWidget>(FoundWidget))
				{
					InvBaseContainerWidget->GetInventoryFromContainerSlot()->ReDrawAllItems();
					InvBaseContainerWidget->GetInventoryFromContainerSlot()->UpdateMoneyInfo();
					InvBaseContainerWidget->GetInventoryFromContainerSlot()->UpdateWeightInfo();
				}
			}
		}
	}*/
}


