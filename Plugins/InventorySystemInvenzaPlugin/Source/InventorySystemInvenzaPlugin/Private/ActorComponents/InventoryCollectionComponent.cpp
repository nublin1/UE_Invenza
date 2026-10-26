//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/InventoryCollectionComponent.h"

#include "ActorComponents/InteractionComponent.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "Engine/GameInstance.h"
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

UInventoryCollectionComponent::UInventoryCollectionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void UInventoryCollectionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!bWasInit)
	{
		InitItemCollection();
	}
	if (bPersistInventories && GetOwner()->HasAuthority() && GetWorld()->IsGameWorld())
	{
		if (auto* Instance = GetWorld()->GetGameInstance())
		{
			Instance->GetSubsystem<UInvenzaSaveSubsystem>()->RegisterCollection(this);
		}
	}
}

void UInventoryCollectionComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (!bWasInit)
	{
		InitItemCollection();
	}
}

void UInventoryCollectionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryCollectionComponent, ActorInventories);
	DOREPLIFETIME(UInventoryCollectionComponent, ResolvedSaveKey);
	DOREPLIFETIME(UInventoryCollectionComponent, LinkedInventories);
	DOREPLIFETIME(UInventoryCollectionComponent, LinkedCraftInventories);
	DOREPLIFETIME(UInventoryCollectionComponent, InventoryArray);
}

void UInventoryCollectionComponent::InitItemCollection()
{
	InventoryArray.OwningManager = Cast<UIInventoryManager>(GetOwner()->FindComponentByClass<UIInventoryManager>());
	InventoryArray.OwningCollection = this;

	bWasInit = true;
}

void UInventoryCollectionComponent::SetVendorInventory(UInventoryBase* InVendorInv)
{
	const FLinkedInventories PreviousInventories = LinkedInventories;
	LinkedInventories.SetVendor(InVendorInv);
	OnRep_LinkedInventories(PreviousInventories);
}

void UInventoryCollectionComponent::SetExternalInventory(UInventoryBase* InExternalInventory)
{
	const FLinkedInventories PreviousInventories = LinkedInventories;
	LinkedInventories.SetExternal(InExternalInventory);
	OnRep_LinkedInventories(PreviousInventories);
}

void UInventoryCollectionComponent::SetCraftInventories(UInventoryBase* InInput, UInventoryBase* InFuel,
														UInventoryBase* InOutput)
{
	LinkedCraftInventories.SetAll(InInput, InFuel, InOutput);
	OnRep_LinkedCraftInventories();
}

void UInventoryCollectionComponent::Server_SetSlotBasedInventoryWidgetInitData_Implementation(
	const FString& ContainerID, FSlotBasedInventoryWidgetInitData InitData)
{
	if (ContainerID.IsEmpty())
	{
		return;
	}

	auto FindResult = GetInventoryByID(ContainerID);
	if (!FindResult)
	{
		return;
	}

	if (USlotbasedInventory* SlotBased = Cast<USlotbasedInventory>(FindResult))
	{
		SlotBased->SetWidgetInitData(InitData);
	}
}

UInventoryBase* UInventoryCollectionComponent::GetInventoryByTag(const FGameplayTag& Tag)
{
	for (auto Element : ActorInventories)
	{
		if (IsValid(Element) && Element->GetInventorySettings().InventoryTag == Tag)
		{
			return Element;
		}
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

TArray<UInventoryBase*> UInventoryCollectionComponent::GetAllInventoriesByTag(const FGameplayTag& Tag)
{
	TArray<UInventoryBase*> Result;

	for (auto Element : ActorInventories)
	{
		if (IsValid(Element) && Element->GetInventorySettings().InventoryTag == Tag)
		{
			Result.Add(Element);
		}
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

UInventoryBase* UInventoryCollectionComponent::GetInventoryByID(FString ContainerID)
{
	if (ContainerID.IsEmpty())
	{
		return nullptr;
	}

	for (auto Element : ActorInventories)
	{
		if (IsValid(Element) && Element->GetInventoryContainerID() == ContainerID)
		{
			return Element;
		}
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

	for (auto* Inventory : {LinkedCraftInventories.InputInventory.Get(), LinkedCraftInventories.FuelInventory.Get(),
							LinkedCraftInventories.OutputInventory.Get()})
	{
		if (IsValid(Inventory) && Inventory->GetInventoryContainerID() == ContainerID)
		{
			return Inventory;
		}
	}
	if (InventoryArray.OwningManager)
	{
		return InventoryArray.OwningManager->FindStorageInventory(ContainerID);
	}
	return nullptr;
}

void UInventoryCollectionComponent::AddPawnInventory_Internal(UInventoryBase* InInventory)
{
	ActorInventories.Add(InInventory);
	OnActorInventoriesChanged.Broadcast();
}

void UInventoryCollectionComponent::RegisterContainerWidget(UInventoryBase* Inventory,
															UInventoryContainerWidget* Widget)
{
	if (!Inventory || !Widget)
	{
		return;
	}

	InventoryContainerWidgetMap.Add(Inventory, Widget);

	auto InvCollection = Inventory->GetItemCollectionLinked();
	if (InvCollection && InvCollection != this)
	{
		InvCollection->OnInventoryItemsChanged.AddUniqueDynamic(this, &UInventoryCollectionComponent::NotifyUI_ReDraw);
	}
}

void UInventoryCollectionComponent::UnregisterContainerWidget(UInventoryBase* Inventory)
{
	if (!Inventory)
	{
		return;
	}

	InventoryContainerWidgetMap.Remove(Inventory);

	auto InvCollection = Inventory->GetItemCollectionLinked();
	if (InvCollection && InvCollection != this)
	{
		for (const auto& Pair : InventoryContainerWidgetMap)
		{
			if (IsValid(Pair.Key) && Pair.Key->GetItemCollectionLinked() == InvCollection)
			{
				return;
			}
		}
		InvCollection->OnInventoryItemsChanged.RemoveDynamic(this, &UInventoryCollectionComponent::NotifyUI_ReDraw);
	}
}

UInventoryContainerWidget* UInventoryCollectionComponent::GetContainerWidget(UInventoryBase* Inventory) const
{
	if (!Inventory)
	{
		return nullptr;
	}

	if (const TObjectPtr<UInventoryContainerWidget>* Found = InventoryContainerWidgetMap.Find(Inventory))
	{
		return Found->Get();
	}

	return nullptr;
}

void UInventoryCollectionComponent::MarkItemAsDirty(UObject* Item)
{
	if (GetWorld() && GetWorld()->GetGameInstance() && GetOwner()->HasAuthority())
	{
		GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->MarkSessionChanged();
	}
	if (!GetOwner()->HasAuthority() || !Item)
	{
		return;
	}

	FInventoryEntry* FoundEntry = InventoryArray.Items.FindByPredicate(
		[Item](const FInventoryEntry& Entry)
		{
			return Entry.Item == Item;
		});

	if (FoundEntry)
	{
		InventoryArray.MarkItemDirty(*FoundEntry);
	}
}

float UInventoryCollectionComponent::CalculateAvailableMoney()
{
	float AvailableMoney = 0.0f;

	const auto* MySettings = UInvenzaInventorySettingsSubsystem::GetSettingsStatic(this);
	if (!MySettings)
	{
		return AvailableMoney;
	}

	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		if (!UInterfaceUtils::ImplementsObjectDataProvider(Entry.Item))
		{
			continue;
		}

		const FItemMetaData& ItemData = IObjectDataProvider::Execute_GetItemRef(Entry.Item);
		if (ItemData.ItemCategory != MySettings->CurrencyGameplayTag)
		{
			continue;
		}

		for (const FItemMapping& Mapping : Entry.Locations.Mappings)
		{
			if (!Mapping.bIsReferenceContainer)
			{
				AvailableMoney +=
					ItemData.ItemTradeData.BasePrice * IObjectDataProvider::Execute_GetQuantity(Entry.Item);
			}
		}
	}

	return AvailableMoney;
}

void UInventoryCollectionComponent::UpdateItemMapping(UObject* Item, const FString& InventoryID,
													  const TArray<UInventorySlotData*>& NewSlots,
													  EItemOrientationType NewOrientation)
{
	if (!GetOwner()->HasAuthority() || !Item)
	{
		return;
	}

	FInventoryEntry* Entry = nullptr;
	if (FItemMapping* Mapping = GetMappingMutable(Item, InventoryID, Entry))
	{
		Mapping->OccupiedSlots = NewSlots;
		Mapping->ItemOrientation = NewOrientation;

		InventoryArray.MarkItemDirty(*Entry);
	}
}

void UInventoryCollectionComponent::UpdateItemVisualLinks(UObject* Item, const FString& InventoryID,
														  UInventoryItemWidget* InWidget,
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

TArray<FItemIDEntry> UInventoryCollectionComponent::CollectItemsAggregated(FString InvID)
{
	TArray<FItemIDEntry> Result;
	TArray<UObject*> Instances = GetAllItemsByContainer(InvID);

	TMap<FName, int32> AggregatedMap;

	for (UObject* Instance : Instances)
	{
		if (!UInterfaceUtils::ImplementsObjectDataProvider(Instance))
		{
			continue;
		}

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

int32 UInventoryCollectionComponent::GetStackCountInContainer(FString InvID)
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

TArray<UObject*> UInventoryCollectionComponent::GetAllItemsByContainer(FString InvID)
{
	TArray<UObject*> Result;

	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		for (const FItemMapping& Mapping : Entry.Locations.Mappings)
		{
			if (Mapping.InventoryID == InvID)
			{
				if (Entry.Item)
				{
					Result.AddUnique(Entry.Item);
				}
				break;
			}
		}
	}

	return Result;
}

TArray<UObject*>
UInventoryCollectionComponent::GetAllSameItemsInContainerByItemSample(const FString& InvID,
																	  const UObject* ReferenceItem) const
{
	TArray<UObject*> SameItems;
	if (InvID.IsEmpty() || !ReferenceItem)
	{
		return SameItems;
	}

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(
			ReferenceItem, TEXT("UInventoryCollectionComponent::GetAllSameItemsInContainerByItemSample")))
	{
		return SameItems;
	}

	const FName RefItemID = IObjectDataProvider::Execute_GetItemID(const_cast<UObject*>(ReferenceItem));

	return GetAllSameItemsInContainerByID(InvID, RefItemID);
}

TArray<UObject*> UInventoryCollectionComponent::GetAllSameItemsInContainerByID(const FString& InvID,
																			   const FName ReferenceID) const
{
	TArray<UObject*> SameItems;
	if (InvID.IsEmpty() || ReferenceID.IsNone())
	{
		return SameItems;
	}

	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		if (!UInterfaceUtils::ImplementsObjectDataProvider(Entry.Item))
		{
			continue;
		}

		if (IObjectDataProvider::Execute_GetItemID(Entry.Item) != ReferenceID)
		{
			continue;
		}

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

TArray<FItemMapping> UInventoryCollectionComponent::GetAllMappingsByContainer(const FString& InvID)
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

TMap<UObject*, FItemMapping*> UInventoryCollectionComponent::GetItemsWithMappingsByContainer(const FString& InvID)
{
	TMap<UObject*, FItemMapping*> Result;

	// WARNING: Array mutations can invalidate a pointer returned to one of its elements.
	// Use the pointer immediately, without modifying the array in between.
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

TArray<UObject*> UInventoryCollectionComponent::GetAllItemsByCategory(FGameplayTag ItemCategory)
{
	TArray<UObject*> Result;

	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		if (!UInterfaceUtils::ImplementsObjectDataProvider(Entry.Item))
		{
			continue;
		}

		if (IObjectDataProvider::Execute_GetItemRef(Entry.Item).ItemCategory == ItemCategory)
		{
			Result.Add(Entry.Item);
		}
	}

	return Result;
}

UObject* UInventoryCollectionComponent::GetItemFromSlot(FGuid TargetSlotID, const FString& InventoryID)
{
	if (!TargetSlotID.IsValid() || InventoryID.IsEmpty())
	{
		return nullptr;
	}

	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		for (const FItemMapping& Mapping : Entry.Locations.Mappings)
		{
			if (Mapping.InventoryID != InventoryID)
			{
				continue;
			}

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

FItemMapping UInventoryCollectionComponent::AddItem(UObject* NewItem, const FItemMapping& ItemMapping)
{
	if (GetWorld() && GetWorld()->GetGameInstance() && GetOwner()->HasAuthority())
	{
		GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->MarkSessionChanged();
	}
	if (!GetOwner()->HasAuthority() || !NewItem)
	{
		return FItemMapping();
	}

	FInventoryEntry* FoundEntry = InventoryArray.Items.FindByPredicate(
		[NewItem](const FInventoryEntry& Entry)
		{
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

void UInventoryCollectionComponent::RemoveItem(UObject* Item, FString ContainerID)
{
	if (GetWorld() && GetWorld()->GetGameInstance() && GetOwner()->HasAuthority())
	{
		GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->MarkSessionChanged();
	}
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

	int32 EntryIndex = InventoryArray.Items.IndexOfByPredicate(
		[Item](const FInventoryEntry& Entry)
		{
			return Entry.Item == Item;
		});

	if (EntryIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem: Item %s not found in InventoryArray."), *Item->GetName());
		return;
	}

	FInventoryEntry& Entry = InventoryArray.Items[EntryIndex];

	const int32 RemovedCount = Entry.Locations.Mappings.RemoveAll(
		[&](const FItemMapping& Mapping)
		{
			return Mapping.InventoryID == ContainerID;
		});

	if (RemovedCount > 0)
	{
		if (Entry.Locations.Mappings.IsEmpty())
		{
			// Remove the item from the collection when it has no remaining mappings.
			InventoryArray.Items.RemoveAt(EntryIndex);

			InventoryArray.MarkArrayDirty();
		}
		else
		{
			// Otherwise, mark the entry dirty while the item still has another location.
			// Replication will then update Locations on clients.
			InventoryArray.MarkItemDirty(Entry);
		}
	}
}

void UInventoryCollectionComponent::RemoveItemFromAllContainers(UObject* Item)
{
	if (GetWorld() && GetWorld()->GetGameInstance() && GetOwner()->HasAuthority())
	{
		GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->MarkSessionChanged();
	}
	if (!GetOwner()->HasAuthority() || !Item)
	{
		return;
	}

	// Validate external input before using it.
	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("RemoveItemFromAllContainers")))
	{
		return;
	}

	FInventoryEntry* FoundEntry = InventoryArray.Items.FindByPredicate(
		[Item](const FInventoryEntry& Entry)
		{
			return Entry.Item == Item;
		});

	if (!FoundEntry)
	{
		return;
	}

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

TArray<FGuid> UInventoryCollectionComponent::GetOccupatedSlotsIDByContainerName(FString InventoryID, UObject* Item)
{
	TArray<FGuid> ResultArray;

	auto ItemMapping = FindItemMappingByContainerName(Item, InventoryID);
	for (auto OccSlot : ItemMapping->OccupiedSlots)
	{
		ResultArray.Add(OccSlot->InventorySlotInfo.SlotGuid);
	}

	return ResultArray;
}

FItemMapping* UInventoryCollectionComponent::FindItemMappingByContainerName(UObject* Item, FString InventoryID)
{
	FInventoryEntry* DummyEntry = nullptr;

	auto ResultMapping = GetMappingMutable(Item, InventoryID, DummyEntry);

	return ResultMapping;
}

TArray<FItemMapping*> UInventoryCollectionComponent::FindAllMappingsForItem(UObject* Item)
{
	TArray<FItemMapping*> Results;

	auto* FoundEntry = InventoryArray.Items.FindByPredicate(
		[Item](const FInventoryEntry& E)
		{
			return E.Item == Item;
		});

	if (!FoundEntry)
	{
		return Results;
	}

	Results.Reserve(FoundEntry->Locations.Mappings.Num());
	for (FItemMapping& Mapping : FoundEntry->Locations.Mappings)
	{
		Results.Add(&Mapping);
	}

	return Results;
}

FItemMapping* UInventoryCollectionComponent::FindMainInventoryMappingForItem(UObject* Item)
{
	for (FItemMapping* ResultMap : FindAllMappingsForItem(Item))
	{
		if (ResultMap && ResultMap->bIsReferenceContainer == false)
		{
			return ResultMap;
		}
	}

	return nullptr;
}

bool UInventoryCollectionComponent::ItemHasInventory(UObject* Item, FString InventoryID)
{
	const FInventoryEntry* FoundEntry = InventoryArray.Items.FindByPredicate(
		[Item](const FInventoryEntry& Entry)
		{
			return Entry.Item == Item;
		});

	if (!FoundEntry)
	{
		return false;
	}

	return FoundEntry->Locations.Mappings.ContainsByPredicate(
		[InventoryID](const FItemMapping& Mapping)
		{
			return Mapping.InventoryID == InventoryID;
		});
}

bool UInventoryCollectionComponent::IsItemOwnedByActor(UObject* Item)
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

void UInventoryCollectionComponent::RequestSortInventory(const FString& ContainerID, EInventorySortCriteria Criteria)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (UInventoryBase* Inventory = GetInventoryByID(ContainerID))
		{
			Inventory->HandleSortItems(Criteria);
		}
	}
	else
	{
		Server_SortItems(ContainerID, Criteria);
	}
}

void UInventoryCollectionComponent::Server_SortItems_Implementation(const FString& ContainerID,
																	EInventorySortCriteria Criteria)
{
	if (UInventoryBase* Inventory = GetInventoryByID(ContainerID))
	{
		Inventory->HandleSortItems(Criteria);
	}
}

void UInventoryCollectionComponent::SerializeForSave(TArray<FItemSaveEntry>& OutData,
													 const TArray<FString>& InventoryFilter)
{
	OutData.Empty();

	for (const FInventoryEntry& Entry : InventoryArray.Items)
	{
		UObject* Item = Entry.Item.Get();
		if (!UInterfaceUtils::ImplementsObjectDataProvider(Item))
		{
			continue;
		}

		FItemSaveEntry SaveEntry;
		SaveEntry.ItemID = IObjectDataProvider::Execute_GetItemID(Item);
		SaveEntry.Quantity = IObjectDataProvider::Execute_GetQuantity(Item);
		SaveEntry.SourceItemRow = IObjectDataProvider::Execute_GetItemRow(Item);

		// Read mappings from the entry itself.
		for (const FItemMapping& Mapping : Entry.Locations.Mappings)
		{
			if (InventoryFilter.Num() > 0 && !InventoryFilter.Contains(Mapping.InventoryID))
			{
				continue;
			}

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

		// Save the item only if valid mappings remain after filtering.
		if (SaveEntry.Mappings.Num() > 0 || InventoryFilter.Num() == 0)
		{
			OutData.Add(SaveEntry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Serialize: OutData contains %d items"), OutData.Num());
}

void UInventoryCollectionComponent::DeserializeFromSave(const TArray<FItemSaveEntry>& InData,
														UInventoryBase* OverrideInventory,
														const TMap<FString, FString>& IDMapping)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	if (InData.IsEmpty())
	{
		return;
	}

	InventoryArray.Items.Empty();

	for (const FItemSaveEntry& SaveEntry : InData)
	{
		UObject* NewItem = UItemFactory::CreateItemByHandle(this, SaveEntry.SourceItemRow, SaveEntry.Quantity);
		if (!NewItem)
		{
			continue;
		}

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

			if (!TargetInventory)
			{
				continue;
			}

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

void UInventoryCollectionComponent::NotifyUI_ItemChanged(UObject* Item, const FString& ContainerID,
														 EInventoryActionType Action)
{
	if (!Item)
	{
		return;
	}

	// Validate and log external calls to the public collection API.
	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("NotifyUI_ItemChanged")))
	{
		return;
	}

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
		Provider->GetOnItemDataReplicatedDelegate().AddUniqueDynamic(
			this, &UInventoryCollectionComponent::OnItemDataReplicated);
		if (Mapping && TargetWidget)
		{
			TargetWidget->AddItemToPanel(*Mapping, CastedItem);
		}
		break;

	case EInventoryActionType::Updated:
		if (Mapping && TargetWidget)
		{
			TargetWidget->UpdateItem(CastedItem);
		}
		break;

	case EInventoryActionType::Removed:
		if (TargetWidget)
		{
			TargetWidget->RemoveItemFromPanel(Mapping ? *Mapping : FItemMapping(), CastedItem);
		}
		Provider->GetOnItemDataReplicatedDelegate().RemoveDynamic(this,
																  &UInventoryCollectionComponent::OnItemDataReplicated);
		break;
	}
}

void UInventoryCollectionComponent::NotifyUI_ReDraw(const FString& ContainerID)
{
	auto Container = GetInventoryByID(ContainerID);
	auto ContainerWidget = InventoryContainerWidgetMap.FindRef(Container);
	if (!ContainerWidget)
	{
		return;
	}

	UUInventoryBaseWidget* TargetWidget = ContainerWidget->GetInventoryWidgetFromContainerSlot();
	if (!TargetWidget)
	{
		return;
	}

	TargetWidget->ReDrawAllItems();
}

void UInventoryCollectionComponent::OnRep_InventoryArray()
{
	bool NewStatus = InventoryArray.Items.IsEmpty();
	OnCollectionItemsIsEmpty.Broadcast(NewStatus);
}

void UInventoryCollectionComponent::OnRep_LinkedInventories(const FLinkedInventories& PreviousInventories)
{
	UIInventoryManager* Manager = InventoryArray.OwningManager;
	if (!Manager)
	{
		return;
	}

	const FLinkedInventories CurrentInventories = LinkedInventories;
	const bool bExternalChanged = CurrentInventories.ExternalInventory != PreviousInventories.ExternalInventory;
	const bool bVendorChanged = CurrentInventories.VendorInventory != PreviousInventories.VendorInventory;
	if (!bExternalChanged && !bVendorChanged)
	{
		return;
	}

	// Close replaced views first. Prev* fields are historical data, not the previous replication state.
	if (bExternalChanged && PreviousInventories.ExternalInventory)
	{
		Manager->CloseSecondaryInventory(EInteractableType::Container);
	}
	if (bVendorChanged && PreviousInventories.VendorInventory)
	{
		Manager->CloseSecondaryInventory(EInteractableType::Vendor);
	}

	if (CurrentInventories.ExternalInventory)
	{
		Manager->OpenSecondaryInventory(CurrentInventories.ExternalInventory, EInteractableType::Container);
	}
	if (CurrentInventories.VendorInventory)
	{
		Manager->OpenSecondaryInventory(CurrentInventories.VendorInventory, EInteractableType::Vendor);
	}
}

void UInventoryCollectionComponent::OnRep_LinkedCraftInventories()
{
	UIInventoryManager* Manager = InventoryArray.OwningManager;
	if (!Manager)
	{
		return;
	}

	Manager->HandleCraftInventoriesChanged(LinkedCraftInventories);
}

void UInventoryCollectionComponent::OnRep_ActorInventories()
{
	if (!bWasInit)
	{
		InitItemCollection();
	}

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
	OnActorInventoriesChanged.Broadcast();
}

void UInventoryCollectionComponent::OnItemDataReplicated(UObject* Item)
{
	if (!Item)
	{
		return;
	}

	TArray<FItemMapping*> Mappings = FindAllMappingsForItem(Item);

	for (auto Mapping : Mappings)
	{
		auto Container = GetInventoryByID(Mapping->InventoryID);
		OnInventoryItemsChanged.Broadcast(Mapping->InventoryID);
		if (auto ContainerWidget = InventoryContainerWidgetMap.FindRef(Container))
		{
			if (auto TargetWidget = ContainerWidget->GetInventoryWidgetFromContainerSlot())
			{
				TargetWidget->UpdateItem(StaticCast<UItemBase*>(Item));
			}
		}
	}
}

FItemMapping* UInventoryCollectionComponent::GetMappingMutable(UObject* Item, const FString& InventoryID,
															   FInventoryEntry*& OutEntry)
{
	OutEntry = InventoryArray.Items.FindByPredicate(
		[Item](const FInventoryEntry& Entry)
		{
			return Entry.Item == Item;
		});

	if (OutEntry)
	{
		for (FItemMapping& Mapping : OutEntry->Locations.Mappings)
		{
			if (Mapping.InventoryID == InventoryID)
			{
				return &Mapping;
			}
		}
	}
	return nullptr;
}

bool UInventoryCollectionComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
														FReplicationFlags* RepFlags)
{
	NormalizeLinkedStorageItemOwnership();
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

void UInventoryCollectionComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (GetWorld() && GetWorld()->GetGameInstance() && GetOwner()->HasAuthority())
	{
		GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->UnregisterCollection(this, Reason);
	}
	Super::EndPlay(Reason);
}
