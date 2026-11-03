// Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/UIInventoryManager.h"
#include "Utility/InventoryGrantService.h"

#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "Data/Inventory/Equipment/EquipmentComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/Items/itemBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Engine/GameInstance.h"
#include "Factory/ItemFactory.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "Utility/InvenzayUtility.h"

void UIInventoryManager::PrepareStartingResources()
{
	if (!GetOwner()->HasAuthority() || bStartingResourcesResolved)
	{
		return;
	}
	auto Queue = [this](UInventoryBase* Inventory, const FInitItemsEntry& Entry, bool bGeneral, bool bDropOnly)
	{
		if (Entry.Amount <= 0 || Entry.Item.IsNull())
		{
			return;
		}
		auto& Grant = PendingStartingGrants.AddDefaulted_GetRef();
		Grant.Inventory = Inventory;
		Grant.Entry = Entry;
		Grant.bGeneral = bGeneral;
		Grant.bDropOnly = bDropOnly;
	};
	// Equipment comes first. Within it, backpacks must precede ordinary reference equipment.
	for (bool bBackpackPass : {true, false})
	{
		for (const auto& Startup : StartupInventories)
		{
			if (Startup.Settings.InventoryTag != GlobalSettings->EquipmentInvTag)
			{
				continue;
			}
			auto* Inventory = ItemCollectionRef->GetInventoryByTag(Startup.Settings.InventoryTag);
			for (const auto& Entry : Startup.StartItems)
			{
				auto* Sample = UItemFactory::CreateItemByHandle(GetOwner(), Entry.Item, 1);
				if (Sample && Sample->IsPortableInventory() == bBackpackPass)
				{
					Queue(Inventory, Entry, false, false);
				}
				else if (!Sample && !bBackpackPass)
				{
					Queue(Inventory, Entry, false, false);
				}
			}
		}
	}
	for (const auto& Pair : StartingItems)
	{
		if (Pair.Key && Pair.Key->GetInventorySettings().InventoryTag != GlobalSettings->EquipmentInvTag)
		{
			for (const auto& Entry : Pair.Value.Items)
			{
				Queue(Pair.Key, Entry, false, false);
			}
		}
	}
	for (const auto& Entry : GeneralStartingItems)
	{
		Queue(nullptr, Entry, true, false);
	}
	if (bUsePortableBackpacks)
	{
		for (const auto& Entry : ExampleGroundBackpacks)
		{
			Queue(nullptr, Entry, false, true);
		}
	}
	StartingItems.Empty();
}

void UIInventoryManager::ResolveStartingResources(bool bRestored)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	bStartingResourcesResolved = true;
	if (bRestored)
	{
		PendingStartingGrants.Empty();
		StartingItems.Empty();
		RefreshPortableBackpack();
		return;
	}
	SetupStartingResources();
}

void UIInventoryManager::SetupStartingResources()
{
	if (!GetOwner()->HasAuthority() || !bManagerInitialized || !bStartingResourcesResolved ||
		bProcessingStartingResources || bEndingPlay || !GlobalSettings)
	{
		return;
	}
	if (PendingStartingGrants.IsEmpty())
	{
		return;
	}
	const auto* Instance = GetWorld()->GetGameInstance();
	const auto* Save = Instance ? Instance->GetSubsystem<UInvenzaSaveSubsystem>() : nullptr;
	if (Save && Save->IsApplyingSnapshot())
	{
		return;
	}
	TGuardValue<bool> Guard(bProcessingStartingResources, true);
	for (auto& Grant : PendingStartingGrants)
	{
		while (Grant.Entry.Amount > 0)
		{
			auto* Target = Grant.bGeneral ? MainPawnInventoryRef.Get() : Grant.Inventory.Get();
			const bool bEquipment = Target && Target->GetInventorySettings().InventoryTag == GlobalSettings->EquipmentInvTag;
			if (!Grant.PendingItem)
			{
				Grant.PendingItem = UItemFactory::CreateItemByHandle(GetOwner(), Grant.Entry.Item, 1);
				if (!Grant.PendingItem)
				{
					if (!Grant.bFailureReported)
					{
						UE_LOG(LogTemp, Error, TEXT("Startup item %s is unavailable; grant retained for retry."), *Grant.Entry.Item.RowName.ToString());
						Grant.bFailureReported = true;
					}
					break;
				}
				const auto Meta = IObjectDataProvider::Execute_GetItemRef(Grant.PendingItem);
				const int32 Limit = bEquipment || !IObjectDataProvider::Execute_IsStackable(Grant.PendingItem)
					? 1 : FMath::Max(1, Meta.ItemNumeraticData.MaxStackSizeInCharacter);
				IObjectDataProvider::Execute_SetQuantity(Grant.PendingItem, FMath::Min(Grant.Entry.Amount, Limit));
			}
			auto* Item = Grant.PendingItem.Get();
			const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(Item);
			FInventoryGrantResult Result;
			Result.Remaining = Quantity;
			if (!Grant.bDropOnly && bEquipment && TryEquipStartingItem(Target, Item))
			{
				Result.Added = Quantity;
				Result.Remaining = 0;
			}
			else
			{
				FInventoryGrantPolicy Policy;
				Policy.bLogOverflowFailure = false;
				Policy.Overflow = EInventoryGrantOverflow::DropAndRetainOnFailure;
				Policy.DropTransform = FTransform(GetOwner()->GetActorRotation(), GetOwner()->GetActorLocation() +
					GetOwner()->GetActorRotation().RotateVector(StartingItemsDropOffset));
				TArray<UInventoryBase*> Targets;
				if (!Grant.bDropOnly && !bEquipment)
				{
					Targets = Grant.bGeneral ? GetReceivingInventories() : TArray<UInventoryBase*>{Target};
				}
				Result = FInventoryGrantService::Grant(Item, Targets, GetOwner(), Policy);
			}
			Grant.Entry.Amount -= Result.Added + Result.Dropped;
			if (Result.Remaining > 0)
			{
				if (!Grant.bFailureReported)
				{
					UE_LOG(LogTemp, Error, TEXT("Cannot drop startup item %s (%d remaining); grant retained for retry."),
						*Grant.Entry.Item.RowName.ToString(), Grant.Entry.Amount);
					Grant.bFailureReported = true;
				}
				break;
			}
			Grant.PendingItem = nullptr;
		}
	}
	PendingStartingGrants.RemoveAll([](const auto& Grant)
	{
		return Grant.Entry.Amount <= 0;
	});
	RefreshPortableBackpack();
}

bool UIInventoryManager::TryEquipStartingItem(UInventoryBase* Inventory, UItemBase* Item)
{
	if (!EquipmentComponentRef || !Inventory || !Item)
	{
		return false;
	}
	if (Item->IsPortableInventory())
	{
		return TryEquipStartingBackpack(Item);
	}
	for (const FGuid ID : Inventory->GetSlotsWithLinkedEquipment())
	{
		auto* Slot = Inventory->GetSlotByGuid(ID);
		if (!Slot)
		{
			continue;
		}
		const auto Tag = Slot->InventorySlotInfo.LinkedEquipmentSlot;
		if (EquipmentComponentRef->IsSlotOccupied(Tag) ||
			!EquipmentComponentRef->IsCategoryCompatibleWithSlot(Tag, IObjectDataProvider::Execute_GetItemRef(Item).ItemCategory))
		{
			continue;
		}
		UInventoryBase* Physical = Inventory->GetInventorySettings().bIsReferenceContainer ? MainPawnInventoryRef.Get() : Inventory;
		FItemMoveData Move;
		Move.SourceItem = Item;
		Move.TargetInventory = Physical;
		Move.TargetSlotID = Physical == Inventory ? ID : FGuid();
		if (!Physical || Physical->HandleAddItem(Move).ActualAmountAdded != 1)
		{
			return false;
		}
		if (Physical != Inventory)
		{
			FItemMapping Reference;
			Reference.InventoryID = Inventory->GetInventoryContainerID();
			Reference.bIsReferenceContainer = true;
			Reference.OccupiedSlots.Add(Slot);
			ItemCollectionRef->AddItem(Item, Reference);
		}
		EquipmentComponentRef->Server_EquipItemToSlot(Tag, Item);
		if (EquipmentComponentRef->IsItemEquippedInSlot(Item, Tag))
		{
			return true;
		}
		Physical->GetItemCollectionLinked()->RemoveItemFromAllContainers(Item);
		ItemCollectionRef->RemoveItemFromAllContainers(Item);
		return false;
	}
	return false;
}

void UIInventoryManager::RetryStartingResources(const FString& InventoryID)
{
	SetupStartingResources();
	if (!GetOwner()->HasAuthority())
	{
		QueueInventoryUIRefresh();
	}
}
