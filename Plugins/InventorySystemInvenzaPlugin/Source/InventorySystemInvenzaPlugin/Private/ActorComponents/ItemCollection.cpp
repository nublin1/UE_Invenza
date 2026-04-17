//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/ItemCollection.h"

#include "ActorComponents/UIInventoryManager.h"
#include "Data//Items/itemBase.h"
#include "ActorComponents/SaveLoad/SaveLoadStructs.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Engine/ActorChannel.h"
#include "Factory/ItemFactory.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


UItemCollection::UItemCollection()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	SetIsReplicatedByDefault(true);
}

void UItemCollection::BeginPlay()
{
	Super::BeginPlay();
			

	if (GetOwner()->HasAuthority())
	{
		InventoryArray.OwningManager = Cast<UIInventoryManager>(GetOwner()->FindComponentByClass<UIInventoryManager>());
		InvManager = InventoryArray.OwningManager;
	}
}

void UItemCollection::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItemCollection, ActorInventories);
	DOREPLIFETIME(UItemCollection, InventoryArray);
}


void UItemCollection::AddPawnInventory_Internal(UInventoryBase* InInventory)
{
	ActorInventories.Add(InInventory);
}

float UItemCollection::CalculateAvailableMoney()
{
	if (ItemLocations.IsEmpty())
	{
		return 0;
	}

	TArray<TObjectPtr<UItemBase>> MoneyItems;
	for (const auto& Pair : ItemLocations)
	{
		auto Item = Pair.Key;
		if (Item->GetItemRef().ItemCategory != EItemCategory::Money)
			continue;
		
		for (auto& Mapping : Pair.Value.Mappings)
		{
			if (Mapping.bIsReferenceContainer)
				continue;

			MoneyItems.Add(Item);
		}
	}

	if (MoneyItems.IsEmpty())
		return 0;

	float AvailableMoney = 0.0f;
	for (UItemBase* MoneyItem : MoneyItems)
	{
		if (MoneyItem)
		{
			AvailableMoney += MoneyItem->GetItemRef().ItemTradeData.BasePrice * MoneyItem->GetQuantity();
		}
	}

	return AvailableMoney;
}

int32 UItemCollection::GetStackCountInContainer(FString InvID)
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
				Result.Add(Mapping);
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
	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AddItem called on Client! This is not allowed."));
		FItemMapping Dummy;
		return Dummy; 
	}
	
	FInventoryEntry* FoundEntry = InventoryArray.Items.FindByPredicate([NewItem](const FInventoryEntry& Entry) {
		return Entry.Item == NewItem;
	});

	if (FoundEntry)
	{
		FoundEntry->Locations.Mappings.Add(ItemMapping);
        
		// КРИТИЧЕСКИ ВАЖНО: Помечаем элемент как "грязный", чтобы сервер отправил изменения клиентам
		InventoryArray.MarkItemDirty(*FoundEntry);
        
		return FoundEntry->Locations.Mappings.Last();
	}
	
	FInventoryEntry& NewEntry = InventoryArray.Items.AddDefaulted_GetRef();
	NewEntry.Item = NewItem;
	NewEntry.Locations.Mappings.Add(ItemMapping);

	// Помечаем новый элемент для репликации
	InventoryArray.MarkItemDirty(NewEntry);

	return NewEntry.Locations.Mappings.Last();
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

void UItemCollection::SerializeForSave(TArray<FItemSaveEntry>& OutData, const TArray<FString>& InventoryFilter)
{
	OutData.Empty();

	for (const auto& Pair : ItemLocations)
	{
		UItemBase* Item = Pair.Key.Get();
		if (!Item) continue;

		FItemSaveEntry Entry;
		Entry.ItemID = Item->GetItemID();
		Entry.Quantity = Item->GetQuantity();
		Entry.SourceItemRow = Item->GetItemRow();

		for (const FItemMapping& Mapping : Pair.Value.Mappings)
		{
			if (InventoryFilter.Num() > 0 && !InventoryFilter.Contains(Mapping.InventoryID))
				continue;
			
			FItemMappingSaveEntry MSave;
			MSave.InventoryID = Mapping.InventoryID;
			MSave.bIsReferenceContainer = Mapping.bIsReferenceContainer;
			MSave.ItemOrientation = Mapping.ItemOrientation;

			for (UInventorySlotData* Slot : Mapping.OccupiedSlots)
			{
				if (Slot)
				{
					MSave.OccupiedCells.Add(Slot->InventorySlotInfo.CellPosition);
				}
			}
			Entry.Mappings.Add(MSave);
		}
		OutData.Add(Entry);
	}

	UE_LOG(LogTemp, Log, TEXT("Serialize: OutData contains %d items"), OutData.Num());
}

void UItemCollection::DeserializeFromSave(const TArray<FItemSaveEntry>& InData, UInventoryBase* OverrideInventory,
	const TMap<FString, FString>& IDMapping)
{
	if (InData.IsEmpty()) return;

	//UE_LOG(LogTemp, Log, TEXT("Deserialize: InData contains %d items"), InData.Num());
	
	ItemLocations.Empty();

	for (const FItemSaveEntry& Entry : InData)
    {
        UItemBase* NewItem = UItemFactory::CreateItemByHandle(this, Entry.SourceItemRow, Entry.Quantity);
        if (!NewItem) continue;
       
        FItemMappingArrayWrapper& Wrapper = ItemLocations.FindOrAdd(NewItem);

        for (const FItemMappingSaveEntry& MSave : Entry.Mappings)
        {
            FItemMapping NewMapping;
            
        	// --- LOGIC FOR DETERMINING ID AND INVENTORY OBJECT ---
            FString TargetID = MSave.InventoryID;
            UInventoryBase* TargetInventory = nullptr;

        	// 1. If this is a simulation of a specific inventory object
            if (OverrideInventory)
            {
                TargetInventory = OverrideInventory;
                TargetID = OverrideInventory->GetInventoryContainerID();
            }
        	// 2. If there is an ID mapping (e.g., for complex simulation of multiple containers)
            else if (IDMapping.Contains(MSave.InventoryID))
            {
                TargetID = IDMapping[MSave.InventoryID];
                TargetInventory = InvManager ? InvManager->GetInventoryByID(TargetID) : nullptr;
            }
        	// 3. Normal loading
            else
            {
                TargetInventory = InvManager ? InvManager->GetInventoryByID(TargetID) : nullptr;
            }

            if (!TargetInventory) continue;

            NewMapping.InventoryID = TargetID;
            NewMapping.bIsReferenceContainer = MSave.bIsReferenceContainer;
            NewMapping.ItemOrientation = MSave.ItemOrientation;
            
        	// Find slots in the target inventory
            if (auto* Slotbased = Cast<USlotbasedInventory>(TargetInventory))
            {
                for (const FIntPoint& CellPos : MSave.OccupiedCells)
                {
                    if (UInventorySlotData* Slot = Slotbased->GetSlotByPosition(CellPos))
                    {
                        NewMapping.OccupiedSlots.Add(Slot);
                    }
                }
            }
            
            Wrapper.Mappings.Add(NewMapping);
        }
    }
}

bool UItemCollection::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	for (UInventoryBase* Inv : ActorInventories)
	{
		if (IsValid(Inv))
		{
			WroteSomething |= Channel->ReplicateSubobject(Inv, *Bunch, *RepFlags);
		}
	}
	
	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		if (IsValid(Entry.Item))
		{
			WroteSomething |= Channel->ReplicateSubobject(Entry.Item, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

