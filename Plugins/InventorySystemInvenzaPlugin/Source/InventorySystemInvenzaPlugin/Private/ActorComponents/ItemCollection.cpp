//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/ItemCollection.h"

#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/Items/itemBase.h"
#include "ActorComponents/SaveLoad/SaveLoadStructs.h"
#include "Blueprint/WidgetTree.h"
#include "Factory/ItemFactory.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Container/InvBaseContainerWidget.h"
#include "UI/Inventory/InventorySlot.h"
#include "UI/Inventory/ListInventoryWidget.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"

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

void UItemCollection::AddItem(UItemBase* NewItem, FItemMapping ItemMapping)
{
	ItemLocations.FindOrAdd(TObjectPtr<UItemBase>(NewItem)).Mappings.Add(ItemMapping);
}

void UItemCollection::RemoveItem(UItemBase* Item, UInvBaseContainerWidget* Container)
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
	auto MappingArrayWrapper = ItemLocations.Find(TObjectPtr<UItemBase>(Item));
	if (!MappingArrayWrapper)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem: Item %s not found in ItemContainers."), *Item->GetName());
		return;
	}
	
	int32 RemovedCount = MappingArrayWrapper->Mappings.RemoveAll([Container](const FItemMapping& Mapping)
	{
		return Mapping.InventoryContainerName == Container->GetFName();
	});

	auto MappingsMappingArrayWrapperTwo = ItemLocations.Find(TObjectPtr<UItemBase>(Item));
	if (MappingsMappingArrayWrapperTwo && MappingsMappingArrayWrapperTwo->Mappings.Num() == 0)
	{
		ItemLocations.Remove(TObjectPtr<UItemBase>(Item));
	}
}

void UItemCollection::RemoveItemFromAllContainers(UItemBase* Item)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemFromAllContainers: Item is null."));
		return;
	}
	auto MappingArrayWrapper = ItemLocations.Find(TObjectPtr<UItemBase>(Item));
	if (!MappingArrayWrapper)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemFromAllContainers: Item %s not found in ItemLocations."), *Item->GetName());
		return;
	}
	
	for (int32 i = MappingArrayWrapper->Mappings.Num() - 1; i >= 0; --i)
	{
		const FItemMapping& Mapping = (MappingArrayWrapper->Mappings)[i];
		
		//UE_LOG(LogTemp, Warning, TEXT("InventoryWidgetBaseLink %s"), *Mapping.InventoryWidgetBaseLink->GetName());
		if (UWidget* FoundWidget = InvManager->GetCoreHUDWidget()->WidgetTree->FindWidget(Mapping.InventoryContainerName))
		{
			if (auto InvBaseContainerWidget = Cast<UInvBaseContainerWidget>(FoundWidget))
			{
				InvBaseContainerWidget->GetInventoryFromContainerSlot()->HandleRemoveItemFromContainer(Item);
			}
		}
	}
	
	ItemLocations.Remove(TObjectPtr<UItemBase>(Item));
}

FItemMapping* UItemCollection::FindItemMappingForItemInContainer(UItemBase* TargetItem, UInvBaseContainerWidget* InContainer)
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
	
	auto MappingArrayWrapper = ItemLocations.Find(TObjectPtr<UItemBase>(TargetItem));
	if (!MappingArrayWrapper)
	{
		UE_LOG(LogTemp, Warning, TEXT("FindItemMappingForItemInContainer: No mapping found for item %s."), *TargetItem->GetName());
		return nullptr;
	}
	
	for (FItemMapping& Mapping : MappingArrayWrapper->Mappings)
	{
		if (InContainer->EqualsByNameAndType(Mapping.InventoryContainerName, Mapping.InventoryType))
		{
			return &Mapping;
		}
	}
	return nullptr;
}

bool UItemCollection::HasItemInContainer(UItemBase* Item, UInvBaseContainerWidget* Container) const
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("HasItemInContainer: Item is null!"));
		return false;
	}
	
	const auto MappingArrayWrapper = ItemLocations.Find(TObjectPtr<UItemBase>(Item));
	if (!MappingArrayWrapper)
	{
		return false;
	}
	for (const FItemMapping& Mapping : MappingArrayWrapper->Mappings)
	{
		if (Container->EqualsByNameAndType(Mapping.InventoryContainerName, Mapping.InventoryType))
		{
			return true;
		}
	}

	return false;
}

TArray<FInventorySlotData> UItemCollection::CollectOccupiedSlotsByContainer(UInvBaseContainerWidget* InContainer)
{
	TArray<FInventorySlotData> OccupiedSlots;
	if (!InContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetOccupiedSlotsForContainer: TargetContainer is null."));
		return OccupiedSlots;
	}
	
	if (ItemLocations.IsEmpty())
		return OccupiedSlots;

	for (const auto& Pair : ItemLocations)
	{
		for (const FItemMapping& Mapping : Pair.Value.Mappings)
		{
			if (InContainer->EqualsByNameAndType(Mapping.InventoryContainerName, Mapping.InventoryType) && !Mapping.ItemSlotDatas.IsEmpty())
			{
				OccupiedSlots.Append(Mapping.ItemSlotDatas);
			}
		}
	}
	
	return OccupiedSlots;
}

UItemBase* UItemCollection::GetItemFromSlot(FInventorySlotData TargetSlotData, UInvBaseContainerWidget* TargetContainer) const
{
	if (!TargetContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetItemFromSlot: %s"), TEXT("TargetContainer is null."));
		return nullptr;
	}

	if (ItemLocations.IsEmpty())
		return nullptr;
	
	for (const auto& Pair : ItemLocations)
	{
		for (const FItemMapping& Mapping : Pair.Value.Mappings)
		{
			if (TargetContainer->EqualsByNameAndType(Mapping.InventoryContainerName, Mapping.InventoryType) && Mapping.ItemSlotDatas.Contains(TargetSlotData))
			{
				return Pair.Key.Get();
			}
		}
	}
    
	//UE_LOG(LogTemp, Warning, TEXT("GetItemFromSlot: No item found for slot %s in container %s"), *TargetSlot->GetName(), *TargetContainer->GetName());
	return nullptr;
}

TArray<UItemBase*> UItemCollection::GetAllItemsByContainer(UInvBaseContainerWidget* TargetContainer) const
{
	TArray<UItemBase*> Result;
    
	if (!TargetContainer || ItemLocations.IsEmpty())
	{
		return Result;
	}
	
	for (const auto& Pair : ItemLocations)
	{
		auto Item = Pair.Key;
		const FItemMappingArrayWrapper& MappingArrayWrapper = Pair.Value;
		
		for (const FItemMapping& Mapping : MappingArrayWrapper.Mappings)
		{
			if (TargetContainer->EqualsByNameAndType(Mapping.InventoryContainerName, Mapping.InventoryType))
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

TArray<UItemBase*> UItemCollection::GetAllSameItemsInContainer(UInvBaseContainerWidget* TargetContainer,
                                                               UItemBase* ReferenceItem) const
{
	TArray<UItemBase*> SameItems;
	if (!TargetContainer || !ReferenceItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetAllSameItemsInContainer: %s"),
			!TargetContainer ? TEXT("TargetContainer is null.") : TEXT("ReferenceItem is null."));
		return SameItems;
	}

	if (ItemLocations.IsEmpty())
		return SameItems;

	auto RefName = ReferenceItem->GetItemRef().ItemTextData.Name;
	for (const auto& Pair : ItemLocations)
	{
		auto Item = Pair.Key;
		if (Item && Item->GetItemRef().ItemTextData.Name.EqualTo(RefName))
		{
			for (const FItemMapping& Mapping : Pair.Value.Mappings)
			{
				if (TargetContainer->EqualsByNameAndType(Mapping.InventoryContainerName, Mapping.InventoryType))
				{
					SameItems.AddUnique(Item.Get());
					break;
				}
			}
		}
	}
    
	return SameItems;
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

UInventoryItemWidget* UItemCollection::GetItemLinkedWidgetForSlot(FInventorySlotData ItemSlotData)
{	
	for (const auto& Pair : ItemLocations)
	{
		const FItemMappingArrayWrapper& MappingArrayWrapper = Pair.Value;
		for (const FItemMapping& Mapping : MappingArrayWrapper.Mappings)
		{
			if (Mapping.ItemSlotDatas.Contains(ItemSlotData))
			{
				return Mapping.ItemVisualLinked;
			}
		}
	}
	return nullptr;
}

void UItemCollection::SortInContainer(UInvBaseContainerWidget* ContainerToSort)
{
	if (auto ListInventory = Cast<UListInventoryWidget>(ContainerToSort->GetInventoryFromContainerSlot()))
	{
		ListInventory->SortInventory();
		return;
	}
	
	TArray<TPair<TObjectPtr<UItemBase>, FItemMapping*>> Mappings;
	for (auto& Pair : ItemLocations)
	{
		for (auto& Mapping : Pair.Value.Mappings)
		{
			if (ContainerToSort->EqualsByNameAndType(Mapping.InventoryContainerName, Mapping.InventoryType))
			{
				Mappings.Emplace(Pair.Key, &Mapping);
			}
		}
	}

	if (Mappings.Num() == 0)
	{
		return;
	}
	
	Mappings.Sort([](auto& A, auto& B) {
		const FString NameA = A.Key->GetItemRef().ItemTextData.Name.ToString();
		const FString NameB = B.Key->GetItemRef().ItemTextData.Name.ToString();
		//UE_LOG(LogTemp, Log, TEXT("Comparing %s and %s"), *NameA, *NameB);
		return NameA.Compare(NameB, ESearchCase::IgnoreCase) < 0;
	});

	UE_LOG(LogTemp, Log, TEXT("Data after sorting:"));
	for (const auto& Pair : Mappings) {
		if (Pair.Key) {
			FString ItemName;
			if (Pair.Key)
				ItemName = Pair.Key->GetItemRef().ItemTextData.Name.ToString();
			else
				ItemName = "INVALID ITEMREF";
			UE_LOG(LogTemp, Log, TEXT("  Item: %s"), *ItemName);
		} else {
			UE_LOG(LogTemp, Log, TEXT("  Item: NULL"));
		}
	}

	if (auto Inv = ContainerToSort->GetInventoryFromContainerSlot())
	{
		Inv->ReDrawAllItems();
	}

	if (auto SlotbasedInventory = Cast<USlotbasedInventoryWidget>(ContainerToSort->GetInventoryFromContainerSlot()))
	{
		for (auto i =0; i < Mappings.Num();  i++)
		{
			auto& Pair = Mappings[i];
			Pair.Value->ItemSlotDatas.Empty();
			auto AvSlot = SlotbasedInventory->GetAvailableSlotForItem(Pair.Key.Get());
			if (AvSlot)
			{				
				Pair.Value->ItemSlotDatas.Add(AvSlot->GetSlotData());
			}
		}

		SlotbasedInventory->ReDrawAllItems();
	}
}

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

	for (const auto& Data : InData)
	{
		UItemBase* Item = UItemFactory::CreateItemByID(GetOwner(), Data.Item.ItemID, Data.Item.Quantity);
		if (!Item) continue;

		FItemMappingArrayWrapper RestoredMappingArrayWrapper;
		for (FItemMappingSaveData SaveMapping : Data.Containers)
		{
			FItemMapping Mapping;
			Mapping.InventoryContainerName = SaveMapping.InventoryContainerName;
			Mapping.InventoryType = SaveMapping.InventoryType;
			for (auto SlotSaveData : SaveMapping.SlotSaveDatas)
			{
				FInventorySlotData SlotData;
				SlotData.SlotName = SlotSaveData.SlotName;
				SlotData.SlotPosition = SlotSaveData.SlotPosition;
				Mapping.ItemSlotDatas.Add(SlotData);
			}

			RestoredMappingArrayWrapper.Mappings.Add(Mapping);
		}

		ItemLocations.Add(Item, RestoredMappingArrayWrapper);

		for (auto RestoredMapping : RestoredMappingArrayWrapper.Mappings)
		{
			if (UWidget* FoundWidget = InvManager->GetCoreHUDWidget()->GetWidgetFromName(RestoredMapping.InventoryContainerName))
			{
				if (!IsValid(FoundWidget))
					continue;
				
				if (auto InvBaseContainerWidget = Cast<UInvBaseContainerWidget>(FoundWidget))
				{
					InvBaseContainerWidget->GetInventoryFromContainerSlot()->ReDrawAllItems();
				}
			}
		}
	}
}


