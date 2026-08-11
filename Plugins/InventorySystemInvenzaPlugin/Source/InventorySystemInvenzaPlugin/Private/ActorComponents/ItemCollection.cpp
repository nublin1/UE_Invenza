//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/ItemCollection.h"

#include "ActorComponents/UIInventoryManager.h"
#include "Data//Items/itemBase.h"
#include "ActorComponents/SaveLoad/SaveLoadStructs.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Engine/ActorChannel.h"
#include "Factory/ItemFactory.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "UI/Inventory/UInventoryBaseWidget.h"
#include "Utility/InterfaceUtils.h"


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

	if (!bWasInit)
		InitItemCollection();
}

void UItemCollection::ReadyForReplication()
{
	Super::ReadyForReplication();
	
	if (!bWasInit)
		InitItemCollection();
}

void UItemCollection::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItemCollection, ActorInventories);
	DOREPLIFETIME(UItemCollection, LinkedInventories);
	DOREPLIFETIME(UItemCollection, InventoryArray);
}

void UItemCollection::InitItemCollection()
{
	InventoryArray.OwningManager = Cast<UIInventoryManager>(GetOwner()->FindComponentByClass<UIInventoryManager>());
	InventoryArray.OwningCollection = this;

	bWasInit = true;
}

void UItemCollection::Server_SetSlotBasedInventoryWidgetInitData_Implementation(const FString& ContainerID,
	FSlotBasedInventoryWidgetInitData InitData)
{
	if (ContainerID.IsEmpty())
		return;

	auto FindResult = GetInventoryByID(ContainerID);
	if (!FindResult)
		return;

	if (USlotbasedInventory* SlotBased = Cast<USlotbasedInventory>(FindResult))
	{
		SlotBased->SetWidgetInitData(InitData);
	}
}

UInventoryBase* UItemCollection::GetInventoryByTag(const FGameplayTag& Tag)
{
	for (auto Element : ActorInventories)
	{
		if (Element->GetInventorySettings().InventoryTag == Tag)
			return Element;
	}

	if (IsValid(LinkedInventories.ExternalInventory) && 
		LinkedInventories.ExternalInventory->GetInventorySettings().InventoryTag == Tag)
	{
		return LinkedInventories.ExternalInventory;
	}
	
	if (IsValid(LinkedInventories.VendorInventory) && 
		LinkedInventories.VendorInventory->GetInventorySettings().InventoryTag == Tag)
	{
		return LinkedInventories.VendorInventory;
	}

	return nullptr;
}

TArray<UInventoryBase*> UItemCollection::GetAllInventoriesByTag(const FGameplayTag& Tag)
{
	TArray<UInventoryBase*> Result;
	
	for (auto Element : ActorInventories)
	{
		if (Element->GetInventorySettings().InventoryTag == Tag)
			Result.Add(Element);
	}

	if (IsValid(LinkedInventories.ExternalInventory) && 
		LinkedInventories.ExternalInventory->GetInventorySettings().InventoryTag == Tag)
	{
		Result.Add(LinkedInventories.ExternalInventory);
	}
	
	if (IsValid(LinkedInventories.VendorInventory) && 
		LinkedInventories.VendorInventory->GetInventorySettings().InventoryTag == Tag)
	{
		Result.Add(LinkedInventories.VendorInventory);
	}
	
	return Result;
}

UInventoryBase* UItemCollection::GetInventoryByID(FString ContainerID)
{
	if (ContainerID.IsEmpty())
		return nullptr;

	for (auto Element : ActorInventories)
	{
		if (Element->GetInventorySettings().InventoryID == ContainerID)
			return Element;
	}

	if (IsValid(LinkedInventories.ExternalInventory) && 
		LinkedInventories.ExternalInventory->GetInventoryContainerID() == ContainerID)
	{
		return LinkedInventories.ExternalInventory;
	}
	
	if (IsValid(LinkedInventories.VendorInventory) && 
		LinkedInventories.VendorInventory->GetInventoryContainerID() == ContainerID)
	{
		return LinkedInventories.VendorInventory;
	}

	return nullptr;
}

void UItemCollection::AddPawnInventory_Internal(UInventoryBase* InInventory)
{
	ActorInventories.Add(InInventory);
}

void UItemCollection::RegisterContainerWidget(UInventoryBase* Inventory, UInventoryContainerWidget* Widget)
{
	if (!Inventory || !Widget)
	{
		return;
	}

	InventoryContainerWidgetMap.Add(Inventory, Widget);

	auto InvCollection = Inventory->GetItemCollectionLinked();
	if (InvCollection != this)
	{
		InvCollection->OnInventoryItemsChanged.AddDynamic(this, &UItemCollection::NotifyUI_ReDraw);
	}
}

void UItemCollection::UnregisterContainerWidget(UInventoryBase* Inventory)
{
	if (!Inventory)
	{
		return;
	}

	InventoryContainerWidgetMap.Remove(Inventory);

	auto InvCollection = Inventory->GetItemCollectionLinked();
	if (InvCollection != this)
	{
		InvCollection->OnInventoryItemsChanged.RemoveDynamic(this, &UItemCollection::NotifyUI_ReDraw);
	}
}

UInventoryContainerWidget* UItemCollection::GetContainerWidget(UInventoryBase* Inventory) const
{
	if (!Inventory)
	{
		return nullptr;
	}

	if (const TObjectPtr<UInventoryContainerWidget>* Found =
		InventoryContainerWidgetMap.Find(Inventory))
	{
		return Found->Get();
	}

	return nullptr;
}

void UItemCollection::MarkItemAsDirty(UObject* Item)
{
	if (!GetOwner()->HasAuthority() || !Item) return;
	
	FInventoryEntry* FoundEntry = InventoryArray.Items.FindByPredicate([Item](const FInventoryEntry& Entry) {
		return Entry.Item == Item;
	});

	if (FoundEntry)
	{
		InventoryArray.MarkItemDirty(*FoundEntry);
	}
}

float UItemCollection::CalculateAvailableMoney()
{
	float AvailableMoney = 0.0f;

	const auto* MySettings = UInvenzaInventorySettingsSubsystem::GetSettingsStatic(this);
	if (!MySettings) return AvailableMoney;

	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		if (!UInterfaceUtils::ImplementsObjectDataProvider(Entry.Item))
			continue;

		const FItemMetaData& ItemData = IObjectDataProvider::Execute_GetItemRef(Entry.Item);
		if (ItemData.ItemCategory != MySettings->CurrencyGameplayTag)
			continue;

		for (const FItemMapping& Mapping : Entry.Locations.Mappings)
		{
			if (!Mapping.bIsReferenceContainer)
			{
				AvailableMoney += ItemData.ItemTradeData.BasePrice * IObjectDataProvider::Execute_GetQuantity(Entry.Item);
			}
		}
	}

	return AvailableMoney;
}

void UItemCollection::UpdateItemMapping(UObject* Item, const FString& InventoryID,
	const TArray<UInventorySlotData*>& NewSlots, EItemOrientationType NewOrientation)
{
	if (!GetOwner()->HasAuthority() || !Item) return;

	FInventoryEntry* Entry = nullptr;
	if (FItemMapping* Mapping = GetMappingMutable(Item, InventoryID, Entry))
	{
		Mapping->OccupiedSlots = NewSlots;
		Mapping->ItemOrientation = NewOrientation;
		
		InventoryArray.MarkItemDirty(*Entry);
	}
}

void UItemCollection::UpdateItemVisualLinks(UObject* Item, const FString& InventoryID, UInventoryItemWidget* InWidget,
	AStorageVisualRepresentation* InActor)
{
	FInventoryEntry* Entry = nullptr;
	if (FItemMapping* Mapping = GetMappingMutable(Item, InventoryID, Entry))
	{
		bool bChanged = false;

		if (InWidget) 
		{
			Mapping->ItemVisualLinked = InWidget;
			bChanged = true;
		}

		if (InActor)
		{
			Mapping->ItemVisualRepresentation = InActor;
			bChanged = true;
		}
		
		if (bChanged && GetOwner()->HasAuthority())
		{
			InventoryArray.MarkItemDirty(*Entry);
		}
	}
}

TArray<FItemIDEntry> UItemCollection::CollectItemsAggregated(FString InvID)
{
	TArray<FItemIDEntry> Result;
	TArray<UObject*> Instances = GetAllItemsByContainer(InvID);

	TMap<FName, int32> AggregatedMap;

	for (UObject* Instance : Instances)
	{
		if (!UInterfaceUtils::ImplementsObjectDataProvider(Instance))
			continue;

		const FName ItemID = IObjectDataProvider::Execute_GetItemID(Instance);
		const int32 Count = IObjectDataProvider::Execute_GetQuantity(Instance);

		AggregatedMap.FindOrAdd(ItemID) += Count;
	}

	for (const auto& Pair : AggregatedMap)
	{
		FItemIDEntry Entry;
		Entry.ItemID = Pair.Key;
		Entry.Amount = Pair.Value;
		Result.Add(Entry);
	}

	return Result;
}

int32 UItemCollection::GetStackCountInContainer(FString InvID)
{
	int32 TotalItemCount = 0;

	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		for (const FItemMapping& Mapping : Entry.Locations.Mappings)
		{
			if (Mapping.InventoryID == InvID)
			{
				TotalItemCount++;
				break;
			}
		}
	}

	return TotalItemCount;
}

TArray<UObject*> UItemCollection::GetAllItemsByContainer(FString InvID)
{
	TArray<UObject*> Result;

	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		for (const FItemMapping& Mapping : Entry.Locations.Mappings)
		{
			if (Mapping.InventoryID == InvID)
			{
				if (Entry.Item) Result.AddUnique(Entry.Item);
				break;
			}
		}
	}

	return Result;
}

TArray<UObject*> UItemCollection::GetAllSameItemsInContainerByItemSample(const FString& InvID, const UObject* ReferenceItem) const
{
	TArray<UObject*> SameItems;
	if (InvID.IsEmpty() || !ReferenceItem) return SameItems;
	
	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ReferenceItem, TEXT("UItemCollection::GetAllSameItemsInContainerByItemSample")))
		return SameItems;

	const FName RefItemID = IObjectDataProvider::Execute_GetItemID(const_cast<UObject*>(ReferenceItem));

	return GetAllSameItemsInContainerByID(InvID, RefItemID);
}

TArray<UObject*> UItemCollection::GetAllSameItemsInContainerByID(const FString& InvID, const FName ReferenceID) const
{
	TArray<UObject*> SameItems;
	if (InvID.IsEmpty() || ReferenceID.IsNone()) return SameItems;

	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		if (!UInterfaceUtils::ImplementsObjectDataProvider(Entry.Item))
			continue;

		if (IObjectDataProvider::Execute_GetItemID(Entry.Item) != ReferenceID)
			continue;

		for (const FItemMapping& Mapping : Entry.Locations.Mappings)
		{
			if (Mapping.InventoryID == InvID)
			{
				SameItems.AddUnique(Entry.Item);
				break;
			}
		}
	}

	return SameItems;
}

TArray<FItemMapping> UItemCollection::GetAllMappingsByContainer(const FString& InvID)
{
	TArray<FItemMapping> Result;

	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		for (const FItemMapping& Mapping : Entry.Locations.Mappings)
		{
			if (Mapping.InventoryID == InvID)
			{
				Result.Add(Mapping);
			}
		}
	}

	return Result;
}

TMap<UObject*, FItemMapping*> UItemCollection::GetItemsWithMappingsByContainer(const FString& InvID)
{
	TMap<UObject*, FItemMapping*> Result;

	// ВНИМАНИЕ: возвращать указатель на элемент массива опасно, если массив может измениться.
	// Но для мгновенного использования в рамках одного кадра/логики это допустимо.
	for (FInventoryEntry& Entry : InventoryArray.Items)
	{
		for (FItemMapping& Mapping : Entry.Locations.Mappings)
		{
			if (Mapping.InventoryID == InvID)
			{
				Result.Add(Entry.Item, &Mapping);
			}
		}
	}

	return Result;
}

TArray<UObject*> UItemCollection::GetAllItemsByCategory(FGameplayTag ItemCategory)
{
	TArray<UObject*> Result;

	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		if (!UInterfaceUtils::ImplementsObjectDataProvider(Entry.Item))
			continue;

		if (IObjectDataProvider::Execute_GetItemRef(Entry.Item).ItemCategory == ItemCategory)
		{
			Result.Add(Entry.Item);
		}
	}

	return Result;
}

UObject* UItemCollection::GetItemFromSlot(FGuid TargetSlotID, const FString& InventoryID)
{
	if (!TargetSlotID.IsValid() || InventoryID.IsEmpty()) return nullptr;

	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		for (const FItemMapping& Mapping : Entry.Locations.Mappings)
		{
			if (Mapping.InventoryID != InventoryID) continue;

			const bool bSlotFound = Mapping.OccupiedSlots.ContainsByPredicate(
				[TargetSlotID](const TObjectPtr<UInventorySlotData>& OcSlot)
				{
					return OcSlot && OcSlot->InventorySlotInfo.SlotGuid == TargetSlotID;
				});

			if (bSlotFound)
			{
				return Entry.Item;
			}
		}
	}

	return nullptr;
}

FItemMapping UItemCollection::AddItem(UObject* NewItem, const FItemMapping& ItemMapping)
{
	if (!GetOwner()->HasAuthority() || !NewItem) return FItemMapping();
	
	FInventoryEntry* FoundEntry = InventoryArray.Items.FindByPredicate([NewItem](const FInventoryEntry& Entry) {
		return Entry.Item == NewItem;
	});
	

	if (FoundEntry)
	{
		int32 Index = FoundEntry->Locations.Mappings.Add(ItemMapping);
		
		InventoryArray.MarkItemDirty(*FoundEntry);
        
		return FoundEntry->Locations.Mappings[Index]; 
	}
	else
	{
		FInventoryEntry& NewEntry = InventoryArray.Items.AddDefaulted_GetRef();
		NewEntry.Item = NewItem;
		NewEntry.Locations.Mappings.Add(ItemMapping);
		
		InventoryArray.MarkArrayDirty();

		return NewEntry.Locations.Mappings[0];
	}
}

void UItemCollection::RemoveItem(UObject* Item, FString ContainerID)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	
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

	int32 EntryIndex = InventoryArray.Items.IndexOfByPredicate([Item](const FInventoryEntry& Entry)
	{
	   return Entry.Item == Item;
	});

	if (EntryIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem: Item %s not found in InventoryArray."), *Item->GetName());
		return;
	}

	FInventoryEntry& Entry = InventoryArray.Items[EntryIndex];

	// Удаляем маппинги для конкретного контейнера
	const int32 RemovedCount = Entry.Locations.Mappings.RemoveAll(
	   [&](const FItemMapping& Mapping)
	   {
		  return Mapping.InventoryID == ContainerID;
	   });

	if (RemovedCount > 0)
	{
		if (Entry.Locations.Mappings.IsEmpty())
		{
			//Если маппингов больше нет, удаляем весь предмет из коллекции
			InventoryArray.Items.RemoveAt(EntryIndex);
			
			InventoryArray.MarkArrayDirty();
		}
		else
		{
			// Если предмет еще где-то лежит, просто помечаем элемент "грязным"
			// Это заставит репликацию обновить Locations на клиентах
			InventoryArray.MarkItemDirty(Entry);
		}
	}
}

void UItemCollection::RemoveItemFromAllContainers(UObject* Item)
{
 if (!GetOwner()->HasAuthority() || !Item)
        return;

    // Внешний входной параметр — строгая проверка.
    if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("RemoveItemFromAllContainers")))
        return;

    FInventoryEntry* FoundEntry = InventoryArray.Items.FindByPredicate([Item](const FInventoryEntry& Entry)
        {
            return Entry.Item == Item;
        });

    if (!FoundEntry)
        return;

    const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(Item);
    const TArray<FItemMapping> MappingsCopy = FoundEntry->Locations.Mappings;

    for (const FItemMapping& Mapping : MappingsCopy)
    {
        if (UInventoryBase* Container = GetInventoryByID(Mapping.InventoryID))
        {
            Container->HandleRemoveItem(StaticCast<UItemBase*>(Item), Quantity);
        }
    }
}

TArray<FGuid> UItemCollection::GetOccupatedSlotsIDByContainerName(FString InventoryID, UObject* Item)
{
	TArray<FGuid> ResultArray;
	
	auto ItemMapping = FindItemMappingByContainerName(Item, InventoryID);
	for (auto OccSlot : ItemMapping->OccupiedSlots)
	{
		ResultArray.Add(OccSlot->InventorySlotInfo.SlotGuid);
	}
	
	return ResultArray;
}

FItemMapping* UItemCollection::FindItemMappingByContainerName(UObject* Item, FString InventoryID)
{
	FInventoryEntry* DummyEntry = nullptr;

	auto ResultMapping = GetMappingMutable(Item, InventoryID, DummyEntry);

	return ResultMapping;
}

TArray<FItemMapping> UItemCollection::FindAllMappingsForItem(UObject* Item)
{
	TArray<FItemMapping> Results;
	if (auto* FoundEntry = InventoryArray.Items.FindByPredicate([Item](const FInventoryEntry& E) { return E.Item == Item; }))
	{
		Results = FoundEntry->Locations.Mappings;
	}
	return Results;
}

UInventoryBase* UItemCollection::FindMainInventoryForItem(UObject* Item)
{
	auto ResultMaps = FindAllMappingsForItem(Item);
	if (ResultMaps.IsEmpty())
		return nullptr;

	for (auto ResultMap : ResultMaps)
	{
		if (ResultMap.bIsReferenceContainer == false)
		{
			return GetInventoryByID(ResultMap.InventoryID);
		}
	}
	
	return nullptr;
}

bool UItemCollection::ItemHasInventory(UObject* Item, FString InventoryID)
{
	const FInventoryEntry* FoundEntry = InventoryArray.Items.FindByPredicate([Item](const FInventoryEntry& Entry)
	{
	   return Entry.Item == Item;
	});

	if (!FoundEntry) return false;
	
	return FoundEntry->Locations.Mappings.ContainsByPredicate(
	   [InventoryID](const FItemMapping& Mapping)
	   {
		  return Mapping.InventoryID == InventoryID;
	   });
}

bool UItemCollection::IsItemOwnedByActor(UObject* Item)
{
	if (!Item)
	{
		return false;
	}

	for (UInventoryBase* Inventory : ActorInventories)
	{
		if (!Inventory)
		{
			continue;
		}

		if (ItemHasInventory(Item, Inventory->GetInventoryContainerID()))
		{
			return true;
		}
	}

	return false;
}

void UItemCollection::SerializeForSave(TArray<FItemSaveEntry>& OutData, const TArray<FString>& InventoryFilter)
{
	OutData.Empty();

	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		UObject* Item = Entry.Item.Get();
		if (!UInterfaceUtils::ImplementsObjectDataProvider(Item))
			continue;

		FItemSaveEntry SaveEntry;
		SaveEntry.ItemID = IObjectDataProvider::Execute_GetItemID(Item);
		SaveEntry.Quantity = IObjectDataProvider::Execute_GetQuantity(Item);
		SaveEntry.SourceItemRow = IObjectDataProvider::Execute_GetItemRow(Item);

		// Обращаемся к маппингам внутри структуры Entry
		for (const FItemMapping& Mapping : Entry.Locations.Mappings)
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
			SaveEntry.Mappings.Add(MSave);
		}

		// Если у предмета остались валидные маппинги после фильтрации, добавляем в сохранение
		if (SaveEntry.Mappings.Num() > 0 || InventoryFilter.Num() == 0)
		{
			OutData.Add(SaveEntry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Serialize: OutData contains %d items"), OutData.Num());
}

void UItemCollection::DeserializeFromSave(const TArray<FItemSaveEntry>& InData, UInventoryBase* OverrideInventory,
	const TMap<FString, FString>& IDMapping)
{
	if (!GetOwner()->HasAuthority()) return;
    if (InData.IsEmpty()) return;
	
    InventoryArray.Items.Empty();

    for (const FItemSaveEntry& SaveEntry : InData)
    {
        UObject* NewItem = UItemFactory::CreateItemByHandle(this, SaveEntry.SourceItemRow, SaveEntry.Quantity);
        if (!NewItem) continue;
    	
        FInventoryEntry& NewEntry = InventoryArray.Items.AddDefaulted_GetRef();
        NewEntry.Item = NewItem;

        for (const FItemMappingSaveEntry& MSave : SaveEntry.Mappings)
        {
            FItemMapping NewMapping;
            
            FString TargetID = MSave.InventoryID;
            UInventoryBase* TargetInventory = nullptr;

            if (OverrideInventory)
            {
                TargetInventory = OverrideInventory;
                TargetID = OverrideInventory->GetInventoryContainerID();
            }
            else if (IDMapping.Contains(MSave.InventoryID))
            {
                TargetID = IDMapping[MSave.InventoryID];
                TargetInventory = GetInventoryByID(TargetID);
            }
            else
            {
                TargetInventory = GetInventoryByID(TargetID);
            }

            if (!TargetInventory) continue;

            NewMapping.InventoryID = TargetID;
            NewMapping.bIsReferenceContainer = MSave.bIsReferenceContainer;
            NewMapping.ItemOrientation = MSave.ItemOrientation;
            
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
        	
            NewEntry.Locations.Mappings.Add(NewMapping);
        }
    }
	
    InventoryArray.MarkArrayDirty();
}

void UItemCollection::NotifyUI_ItemChanged(UObject* Item, const FString& ContainerID, EInventoryActionType Action)
{
	if (!Item) return;

	// Внешний вызов (публичный API коллекции) — строгая проверка с логированием.
	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("NotifyUI_ItemChanged")))
		return;

	IObjectDataProvider* Provider = Cast<IObjectDataProvider>(Item);
	
	auto CastedItem = StaticCast<UItemBase*>(Item);

	auto Container = GetInventoryByID(ContainerID);
	auto ContainerWidget = InventoryContainerWidgetMap.FindRef(Container);
	UUInventoryBaseWidget* TargetWidget = nullptr;
	if (ContainerWidget)
	{
		TargetWidget = ContainerWidget->GetInventoryWidgetFromContainerSlot();
	}

	FItemMapping* Mapping = FindItemMappingByContainerName(Item, ContainerID);

	switch (Action)
	{
	case EInventoryActionType::Added:
		Provider->GetOnItemDataReplicatedDelegate().AddUniqueDynamic(this, &UItemCollection::OnItemDataReplicated);
		if (Mapping && TargetWidget) TargetWidget->AddItemToPanel(*Mapping, CastedItem);
		break;

	case EInventoryActionType::Updated:
		if (Mapping && TargetWidget) TargetWidget->UpdateItem(CastedItem);
		break;

	case EInventoryActionType::Removed:
		if (TargetWidget)
		{
			TargetWidget->RemoveItemFromPanel(Mapping ? *Mapping : FItemMapping(), CastedItem);
		}
		Provider->GetOnItemDataReplicatedDelegate().RemoveDynamic(this, &UItemCollection::OnItemDataReplicated);
		break;
	}
}

void UItemCollection::NotifyUI_ReDraw(const FString& ContainerID)
{
	auto Container = GetInventoryByID(ContainerID);
	auto ContainerWidget = InventoryContainerWidgetMap.FindRef(Container);
	if (!ContainerWidget)
		return;
	
	UUInventoryBaseWidget* TargetWidget = ContainerWidget->GetInventoryWidgetFromContainerSlot();
	if (!TargetWidget) return;

	TargetWidget->ReDrawAllItems();
}

void UItemCollection::OnRep_LinkedInventories()
{
	if (LinkedInventories.ExternalInventory)
	{
		InventoryArray.OwningManager->OpenExternalInventory(
			LinkedInventories.ExternalInventory
		);
	}
	else if (LinkedInventories.PrevExternalInventory)
	{
		InventoryArray.OwningManager->CloseExternalInventory(
			LinkedInventories.PrevExternalInventory
		);
	}
	
	if (LinkedInventories.VendorInventory)
	{
		InventoryArray.OwningManager->OpenVendorInventory(
			LinkedInventories.VendorInventory
		);
	}
	else if (LinkedInventories.PrevVendorInventory)
	{
		InventoryArray.OwningManager->CloseVendorInventory(
			LinkedInventories.PrevVendorInventory
		);
	}
}

void UItemCollection::OnRep_ActorInventories()
{
	if (!bWasInit)
		InitItemCollection();
	
	if (InventoryArray.OwningManager)
	{
		for (UInventoryBase* Inv : ActorInventories)
		{
			if (Inv)
			{
				InventoryArray.OwningManager->BindInventoryEvents(Inv);
			}
		}
	}
}

void UItemCollection::OnItemDataReplicated(UObject* Item)
{
	if (!Item) return;

	const TArray<FItemMapping>& Mappings = FindAllMappingsForItem(Item);

	for (const FItemMapping& Mapping : Mappings)
	{
		auto Container = GetInventoryByID(Mapping.InventoryID);
		OnInventoryItemsChanged.Broadcast(Mapping.InventoryID);
		if (auto ContainerWidget = InventoryContainerWidgetMap.FindRef(Container))
		{
			if (auto TargetWidget = ContainerWidget->GetInventoryWidgetFromContainerSlot())
			{
				TargetWidget->UpdateItem(StaticCast<UItemBase*>(Item));
			}
		}
	}
}

FItemMapping* UItemCollection::GetMappingMutable(UObject* Item, const FString& InventoryID,
                                                 FInventoryEntry*& OutEntry)
{
	OutEntry = InventoryArray.Items.FindByPredicate([Item](const FInventoryEntry& Entry) {
		return Entry.Item == Item;
	});

	if (OutEntry)
	{
		for (FItemMapping& Mapping : OutEntry->Locations.Mappings)
		{
			if (Mapping.InventoryID == InventoryID) return &Mapping;
		}
	}
	return nullptr;
}

bool UItemCollection::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	for (UInventoryBase* Inv : ActorInventories)
	{
		if (IsValid(Inv))
		{
			WroteSomething |= Channel->ReplicateSubobject(Inv, *Bunch, *RepFlags);
			WroteSomething |= Inv->ReplicateSubobjects(Channel, Bunch, RepFlags);
		}
	}
	
	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		if (IsValid(Entry.Item))
		{
			WroteSomething |= Channel->ReplicateSubobject(Entry.Item, *Bunch, *RepFlags);
		}
	}
	
	auto ReplicateInventory = [&](UInventoryBase* Inventory)
	{
		if (IsValid(Inventory))
		{
			WroteSomething |= Channel->ReplicateSubobject(Inventory, *Bunch, *RepFlags);
			WroteSomething |= Inventory->ReplicateSubobjects(Channel, Bunch, RepFlags);
		}
	};
	ReplicateInventory(LinkedInventories.ExternalInventory);
	ReplicateInventory(LinkedInventories.VendorInventory);
	ReplicateInventory(LinkedInventories.PrevExternalInventory);
	ReplicateInventory(LinkedInventories.PrevVendorInventory);

	return WroteSomething;
}

