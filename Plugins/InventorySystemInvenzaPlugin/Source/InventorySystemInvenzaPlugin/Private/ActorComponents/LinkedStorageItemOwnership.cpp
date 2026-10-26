//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/InventoryCollectionComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/Equipment/EquipmentComponent.h"
#include "Data/Items/itemBase.h"
#include "Factory/ItemFactory.h"
#include "World/LinkedStorageState.h"

void UInventoryCollectionComponent::NormalizeLinkedStorageItemOwnership()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	TSet<FString> ChangedInventories;
	for (auto& Entry : InventoryArray.Items)
	{
		auto* Item = Cast<UItemBase>(Entry.Item);
		if (!Item || Item->GetTypedOuter<AActor>() == GetOwner() ||
			(!Cast<ALinkedStorageState>(GetOwner()) && !Item->GetTypedOuter<ALinkedStorageState>()))
		{
			continue;
		}
		const bool bHasPhysicalLocation = Entry.Locations.Mappings.ContainsByPredicate(
			[this](const FItemMapping& Mapping)
			{
				return !Mapping.bIsReferenceContainer && ActorInventories.ContainsByPredicate(
					[&Mapping](UInventoryBase* Inventory)
					{
						return Inventory && Inventory->GetInventoryContainerID() == Mapping.InventoryID;
					});
			});
		if (!bHasPhysicalLocation)
		{
			continue;
		}
		FInventorySavedItem Saved;
		Saved.InstanceID = Item->GetPersistentInstanceID();
		Saved.InstanceClass = FSoftClassPath(Item->GetClass());
		Saved.Definition = IObjectDataProvider::Execute_GetItemRow(Item);
		Saved.Quantity = IObjectDataProvider::Execute_GetQuantity(Item);
		Saved.SetCustomState(Item->CaptureSaveState());
		auto* OwnedItem = UItemFactory::CreateItemFromSave(GetOwner(), Saved);
		if (!OwnedItem)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not move linked-storage item %s to its physical owner."), *GetNameSafe(Item));
			continue;
		}
		FGameplayTag EquippedSlot;
		auto* Equipment = GetOwner()->FindComponentByClass<UEquipmentComponent>();
		const bool bEquipped = Equipment && Equipment->FindSlotByItem(Item, EquippedSlot);
		if (bEquipped)
		{
			Equipment->Server_UnequipItemFromSlot(EquippedSlot);
		}
		Item->OnItemDataReplicated.RemoveDynamic(this, &ThisClass::OnItemDataReplicated);
		OwnedItem->OnItemDataReplicated.AddUniqueDynamic(this, &ThisClass::OnItemDataReplicated);
		Entry.Item = OwnedItem;
		InventoryArray.MarkItemDirty(Entry);
		if (bEquipped)
		{
			Equipment->Server_EquipItemToSlot(EquippedSlot, OwnedItem);
		}
		for (const auto& Mapping : Entry.Locations.Mappings)
		{
			ChangedInventories.Add(Mapping.InventoryID);
		}
	}
	for (const FString& ID : ChangedInventories)
	{
		OnInventoryItemsChanged.Broadcast(ID);
		NotifyUI_ReDraw(ID);
	}
	if (!ChangedInventories.IsEmpty())
	{
		GetOwner()->ForceNetUpdate();
	}
}
