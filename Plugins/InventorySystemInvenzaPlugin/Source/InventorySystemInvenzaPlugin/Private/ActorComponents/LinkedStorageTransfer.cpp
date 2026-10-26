//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Items/itemBase.h"
#include "Factory/ItemFactory.h"
#include "World/LinkedStorageState.h"

void UIInventoryManager::NormalizeLinkedStorageTransferOwnership(const FItemMoveData& Move)
{
	for (auto* Inventory : {Move.SourceInventory.Get(), Move.TargetInventory.Get()})
	{
		if (IsValid(Inventory) && Inventory->GetItemCollectionLinked())
		{
			Inventory->GetItemCollectionLinked()->NormalizeLinkedStorageItemOwnership();
		}
	}
}

bool UIInventoryManager::TryApplyLinkedStorageTransfer(FItemMoveData Move)
{
	auto* Source = Move.SourceInventory.Get();
	auto* Target = Move.TargetInventory.Get();
	if (!IsValid(Source) || !IsValid(Target) || Source->GetItemCollectionLinked() == Target->GetItemCollectionLinked() ||
		(!Cast<ALinkedStorageState>(Source->GetInventoryOwnerActor()) &&
		 !Cast<ALinkedStorageState>(Target->GetInventoryOwnerActor())) ||
		Source->GetInventorySettings().bIsReferenceContainer || Target->GetInventorySettings().bIsReferenceContainer)
	{
		return false;
	}
	// Occupied-slot swaps retain the existing transaction path and normalize ownership afterwards.
	if (Move.TargetSlotID.IsValid() && Target->GetItemCollectionLinked()->GetItemFromSlot(
		Move.TargetSlotID, Target->GetInventoryContainerID()))
	{
		return false;
	}
	auto* Original = Cast<UItemBase>(Move.SourceItem);
	if (!GetOwner()->HasAuthority() || !Original || !IsInventoryAccessible(Source) || !IsInventoryAccessible(Target) ||
		!Source->GetItemCollectionLinked()->FindItemMappingByContainerName(Original, Source->GetInventoryContainerID()))
	{
		return true;
	}
	FInventorySavedItem Saved;
	Saved.InstanceID = FGuid::NewGuid();
	Saved.InstanceClass = FSoftClassPath(Original->GetClass());
	Saved.Definition = IObjectDataProvider::Execute_GetItemRow(Original);
	Saved.Quantity = IObjectDataProvider::Execute_GetQuantity(Original);
	Saved.SetCustomState(Original->CaptureSaveState());
	auto* OwnedItem = UItemFactory::CreateItemFromSave(Target->GetInventoryOwnerActor(), Saved);
	if (!OwnedItem)
	{
		return true;
	}
	Move.SourceItem = OwnedItem;
	const FItemAddResult Result = Target->HandleAddItem(Move);
	if (Result.ActualAmountAdded > 0 && Result.ActualAmountAdded <= Saved.Quantity)
	{
		if (Result.ActualAmountAdded == Saved.Quantity)
		{
			OwnedItem->SetPersistentInstanceID(Original->GetPersistentInstanceID());
			Source->GetItemCollectionLinked()->RemoveItemFromAllContainers(Original);
		}
		else
		{
			Source->HandleRemoveItem(Original, Result.ActualAmountAdded);
			Source->GetItemCollectionLinked()->MarkItemAsDirty(Original);
		}
		Source->RequestToResetItemVisual(Original);
		for (auto* Inventory : {Source, Target})
		{
			Inventory->UpdateWeightInfo();
			Inventory->UpdateMoneyInfo();
			Inventory->GetItemCollectionLinked()->OnInventoryItemsChanged.Broadcast(Inventory->GetInventoryContainerID());
			Inventory->GetItemCollectionLinked()->GetOwner()->ForceNetUpdate();
		}
	}
	return true;
}
