// Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/UIInventoryManager.h"

#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "ActorComponents/Crafting/CraftingComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Data/Inventory/Equipment/EquipmentComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/Items/itemBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Interface/Interaction/VendorProvider.h"
#include "Interface/Inventory/InvUIProvider.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"
#include "World/PortableInventoryState.h"
#include "Utility/InvenzayUtility.h"

static const FString PortableBackpackMountID(TEXT("PortableBackpackMount"));

static UInventoryBase* FindPortableBackpackMount(UInventoryCollectionComponent* Collection)
{
	for (auto* Inventory : Collection->GetActorInventories())
	{
		if (Inventory && Inventory->GetInventorySettings().CustomInventoryID == PortableBackpackMountID)
		{
			return Inventory;
		}
	}
	return nullptr;
}

static UInventoryBase* CreatePortableBackpackMount(UInventoryCollectionComponent* Collection, FGameplayTag SlotTag,
	FGameplayTag Category)
{
	if (auto* Existing = FindPortableBackpackMount(Collection))
	{
		return Existing;
	}
	FInventoryStartupData Startup;
	Startup.Settings.InventoryClass = USlotbasedInventory::StaticClass();
	Startup.Settings.CustomInventoryID = PortableBackpackMountID;
	Startup.Settings.bReceiveGrantedItems = false;
	Startup.Settings.bCollectInvDataFromWidget = false;
	Startup.Settings.bAllowItemReferencing = true;
	Startup.Settings.InventorySlotBasedSettings.bIgnoreItemSize = true;
	Startup.Settings.InventorySlotBasedSettings.InitNumberRows = 1;
	Startup.Settings.InventorySlotBasedSettings.InitNumColumns = 1;
	auto* Inventory = Cast<USlotbasedInventory>(UInventoryBase::CreateInventoryAdvanced(
		Collection->GetOwner(), Startup, Collection->GetOwner(), Collection));
	if (!Inventory || Inventory->GetInventorySlots().IsEmpty())
	{
		return nullptr;
	}
	Inventory->GetInventorySlots()[0]->InventorySlotInfo.LinkedEquipmentSlot = SlotTag;
	Inventory->GetInventorySlots()[0]->InventorySlotInfo.AllowedCategory = Category;
	Collection->AddPawnInventory_Internal(Inventory);
	return Inventory;
}

static UInventorySlotData* FindPortableBackpackEquipmentSlot(UInventoryBase* Gear,
	const UEquipmentComponent* Equipment, FGameplayTag Category, FGuid PreferredSlot)
{
	if (!Gear || !Equipment)
	{
		return nullptr;
	}
	for (const FGuid ID : Gear->GetSlotsWithLinkedEquipment())
	{
		auto* Slot = Gear->GetSlotByGuid(ID);
		if (Slot && (!PreferredSlot.IsValid() || PreferredSlot == ID) &&
			Equipment->IsCategoryCompatibleWithSlot(Slot->InventorySlotInfo.LinkedEquipmentSlot, Category))
		{
			return Slot;
		}
	}
	return nullptr;
}

static AActor* PreparePortableBackpackDrop(UItemBase* Item, AActor* Carrier)
{
	if (!Item || !Carrier || !Carrier->HasAuthority() || !Item->IsPortableInventory())
	{
		return nullptr;
	}
	auto* State = Item->GetPortableState();
	const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(Item);
	if (!Meta.bIsDroppable || (State && State->HasCraftObligations()))
	{
		return nullptr;
	}
	auto* PickupActor = UInvenzayUtility::DropItem(Carrier->GetWorld(), Carrier,
		IObjectDataProvider::Execute_GetItemRow(Item), 1,
		Carrier->GetActorLocation() + Carrier->GetActorForwardVector() * 100.f, Carrier->GetActorRotation());
	auto* Pickup = PickupActor ? PickupActor->FindComponentByClass<UPickupComponent>() : nullptr;
	if (!Pickup)
	{
		if (PickupActor)
		{
			PickupActor->Destroy();
		}
		return nullptr;
	}
	auto* Copy = Item->CreateIdentityTransferCopy(Pickup);
	if (!Copy)
	{
		PickupActor->Destroy();
		return nullptr;
	}
	Pickup->RestorePersistentItem(Copy);
	return PickupActor;
}

static void NotifyPortableBackpackCollection(UInventoryCollectionComponent* Collection)
{
	for (auto* Inventory : Collection->GetActorInventories())
	{
		if (Inventory)
		{
			Inventory->UpdateWeightInfo();
			Inventory->OnInventoryRedrawRequested.Broadcast();
			Collection->OnInventoryItemsChanged.Broadcast(Inventory->GetInventoryContainerID());
		}
	}
	Collection->OnActorInventoriesChanged.Broadcast();
	Collection->GetOwner()->ForceNetUpdate();
	if (auto* Manager = Collection->GetOwner()->FindComponentByClass<UIInventoryManager>())
	{
		Manager->RefreshPortableBackpack();
	}
}

bool UIInventoryManager::TryEquipStartingBackpack(UItemBase* Item)
{
	if (!bProcessingStartingResources || !GetOwner()->HasAuthority() || !ItemCollectionRef || !EquipmentComponentRef)
	{
		return false;
	}
	for (const auto& Slot : EquipmentComponentRef->GetEquipmentSlotsArray())
	{
		const auto* Equipped = Cast<UItemBase>(Slot.EquippedItem);
		if (Equipped && Equipped->IsPortableInventory())
		{
			return false;
		}
	}
	auto* Gear = ItemCollectionRef->GetInventoryByTag(GlobalSettings->EquipmentInvTag);
	const auto Category = IObjectDataProvider::Execute_GetItemRef(Item).ItemCategory;
	auto* Slot = FindPortableBackpackEquipmentSlot(Gear, EquipmentComponentRef, Category, {});
	if (!Slot || EquipmentComponentRef->IsSlotOccupied(Slot->InventorySlotInfo.LinkedEquipmentSlot))
	{
		return false;
	}
	// Stage the newly created item in an owned physical source and use the normal equip transaction.
	auto* Mount = CreatePortableBackpackMount(ItemCollectionRef, Slot->InventorySlotInfo.LinkedEquipmentSlot, Category);
	auto* MountSlot = Mount ? Mount->GetSlotByPosition(FIntPoint::ZeroValue) : nullptr;
	if (!MountSlot)
	{
		return false;
	}
	FItemMapping Mapping;
	Mapping.InventoryID = Mount->GetInventoryContainerID();
	Mapping.OccupiedSlots.Add(MountSlot);
	ItemCollectionRef->AddItem(Item, Mapping);
	const bool bEquipped = TryEquipPortableBackpack(Item, Mount, nullptr, nullptr, Slot->InventorySlotInfo.SlotGuid);
	if (!bEquipped)
	{
		ItemCollectionRef->RemoveItemFromAllContainers(Item);
	}
	return bEquipped;
}

bool UIInventoryManager::TryEquipPortableBackpack(UItemBase* Item, UInventoryBase* Source,
	UPickupComponent* Pickup, AActor* TargetOwner, FGuid TargetSlotID)
{
	if (!GetOwner()->HasAuthority() || bApplyingEquipmentTransfer || !Item || !Item->IsPortableInventory() ||
		IObjectDataProvider::Execute_GetQuantity(Item) != 1)
	{
		return false;
	}
	TargetOwner = TargetOwner ? TargetOwner : GetOwner();
	auto* TargetCollection = TargetOwner->FindComponentByClass<UInventoryCollectionComponent>();
	auto* Equipment = TargetOwner->FindComponentByClass<UEquipmentComponent>();
	if (!TargetCollection || !Equipment || !GlobalSettings)
	{
		return false;
	}
	auto* Gear = TargetCollection->GetInventoryByTag(GlobalSettings->EquipmentInvTag);
	if (!IsInventoryAccessible(Gear) || !Gear->CanReceiveItem(Item))
	{
		return false;
	}
	auto* GearSlot = FindPortableBackpackEquipmentSlot(Gear, Equipment,
		IObjectDataProvider::Execute_GetItemRef(Item).ItemCategory, TargetSlotID);
	if (!GearSlot)
	{
		return false;
	}
	const FGameplayTag EquipmentSlot = GearSlot->InventorySlotInfo.LinkedEquipmentSlot;
	auto* SourceCollection = Source ? Source->GetItemCollectionLinked() : nullptr;
	if (Pickup)
	{
		if (Pickup->PeekItemData() != Item || !UInteractionComponent::IsWithinInteractionDistance(GetOwner(), Pickup) ||
			Pickup->IsInteractionLockedByOther(GetOwner()))
		{
			return false;
		}
	}
	else if (!SourceCollection || !IsInventoryAccessible(Source) ||
		!SourceCollection->ItemHasInventory(Item, Source->GetInventoryContainerID()))
	{
		return false;
	}
	auto* State = Item->EnsurePortableState();
	if (!State || State->HasCraftObligations())
	{
		return false;
	}
	UItemBase* Previous = nullptr;
	for (const auto& Slot : Equipment->GetEquipmentSlotsArray())
	{
		if (auto* Candidate = Cast<UItemBase>(Slot.EquippedItem); Candidate && Candidate->IsPortableInventory())
		{
			Previous = Candidate;
		}
	}
	if (Previous == Item)
	{
		return true;
	}
	const auto* TargetManager = TargetOwner->FindComponentByClass<UIInventoryManager>();
	const auto* SourceManager = SourceCollection
		? SourceCollection->GetOwner()->FindComponentByClass<UIInventoryManager>() : nullptr;
	if ((TargetManager && TargetManager->IsPortableBackpackRemovalBlocked(Previous)) ||
		(SourceManager && SourceManager->IsPortableBackpackRemovalBlocked(Item)))
	{
		return false;
	}
	auto* Mount = CreatePortableBackpackMount(TargetCollection, EquipmentSlot,
		IObjectDataProvider::Execute_GetItemRef(Item).ItemCategory);
	auto* MountSlot = Mount ? Mount->GetSlotByPosition(FIntPoint::ZeroValue) : nullptr;
	auto* OwnedItem = Item->CreateIdentityTransferCopy(TargetOwner);
	if (!MountSlot || !OwnedItem)
	{
		return false;
	}
	auto* DroppedPrevious = Previous ? PreparePortableBackpackDrop(Previous, TargetOwner) : nullptr;
	if (Previous && !DroppedPrevious)
	{
		return false;
	}
	// Spawning can execute game code. Validate the source and equipment again before committing.
	if ((Pickup && (Pickup->PeekItemData() != Item || Pickup->GetOwner()->IsActorBeingDestroyed())) ||
		(SourceCollection && !SourceCollection->ItemHasInventory(Item, Source->GetInventoryContainerID())) ||
		(Previous && !Equipment->IsItemEquipped(Previous)) ||
		(TargetManager && TargetManager->IsPortableBackpackRemovalBlocked(Previous)) ||
		(SourceManager && SourceManager->IsPortableBackpackRemovalBlocked(Item)))
	{
		if (DroppedPrevious)
		{
			DroppedPrevious->Destroy();
		}
		return false;
	}
	{
		TGuardValue<bool> Guard(bApplyingEquipmentTransfer, true);
		if (Previous)
		{
			TargetCollection->RemoveItemFromAllContainers(Previous);
			for (const auto& Slot : Equipment->GetEquipmentSlotsArray())
			{
				if (Slot.EquippedItem == Previous)
				{
					Equipment->Server_UnequipItemFromSlot(Slot.SlotTag);
				}
			}
		}
		if (SourceCollection)
		{
			SourceCollection->RemoveItemFromAllContainers(Item);
			if (auto* SourceEquipment = SourceCollection->GetOwner()->FindComponentByClass<UEquipmentComponent>())
			{
				FGameplayTag SourceSlot;
				if (SourceEquipment->FindSlotByItem(Item, SourceSlot))
				{
					SourceEquipment->Server_UnequipItemFromSlot(SourceSlot);
				}
			}
		}
		MountSlot->InventorySlotInfo.LinkedEquipmentSlot = EquipmentSlot;
		MountSlot->InventorySlotInfo.AllowedCategory = IObjectDataProvider::Execute_GetItemRef(Item).ItemCategory;
		FItemMapping Physical;
		Physical.InventoryID = Mount->GetInventoryContainerID();
		Physical.OccupiedSlots.Add(MountSlot);
		TargetCollection->AddItem(OwnedItem, Physical);
		FItemMapping Reference;
		Reference.InventoryID = Gear->GetInventoryContainerID();
		Reference.bIsReferenceContainer = Gear->GetInventorySettings().bIsReferenceContainer;
		Reference.OccupiedSlots.Add(GearSlot);
		// Non-reference equipment stores the item directly; it needs no additional physical mount mapping.
		if (!Reference.bIsReferenceContainer)
		{
			TargetCollection->RemoveItem(OwnedItem, Physical.InventoryID);
		}
		TargetCollection->AddItem(OwnedItem, Reference);
		Equipment->Server_EquipItemToSlot(EquipmentSlot, OwnedItem);
	}
	if (Pickup)
	{
		Pickup->OnPickedUp();
	}
	if (SourceCollection && SourceCollection != TargetCollection)
	{
		NotifyPortableBackpackCollection(SourceCollection);
	}
	NotifyPortableBackpackCollection(TargetCollection);
	return true;
}

bool UIInventoryManager::TryDropPortableBackpack(UItemBase* Item, UInventoryCollectionComponent* Collection)
{
	if (!GetOwner()->HasAuthority() || !Item || !Collection || bApplyingEquipmentTransfer)
	{
		return false;
	}
	const auto* Physical = Collection->FindMainInventoryMappingForItem(Item);
	const auto* CarrierManager = Collection->GetOwner()->FindComponentByClass<UIInventoryManager>();
	const bool bHasAccessibleMapping = Collection->GetActorInventories().ContainsByPredicate([&](auto* Inventory)
	{
		return Inventory && IsInventoryAccessible(Inventory) &&
			Collection->ItemHasInventory(Item, Inventory->GetInventoryContainerID());
	});
	if (!Physical || !bHasAccessibleMapping ||
		(CarrierManager && CarrierManager->IsPortableBackpackRemovalBlocked(Item)))
	{
		return false;
	}
	auto* Carrier = Collection->GetOwner()->IsA<APortableInventoryState>() ? GetOwner() : Collection->GetOwner();
	auto* Dropped = PreparePortableBackpackDrop(Item, Carrier);
	if (!Dropped)
	{
		return false;
	}
	if (!Collection->FindMainInventoryMappingForItem(Item) ||
		(CarrierManager && CarrierManager->IsPortableBackpackRemovalBlocked(Item)))
	{
		Dropped->Destroy();
		return false;
	}
	{
		TGuardValue<bool> Guard(bApplyingEquipmentTransfer, true);
		Collection->RemoveItemFromAllContainers(Item);
		if (auto* Equipment = Collection->GetOwner()->FindComponentByClass<UEquipmentComponent>())
		{
			FGameplayTag Slot;
			if (Equipment->FindSlotByItem(Item, Slot))
			{
				Equipment->Server_UnequipItemFromSlot(Slot);
			}
		}
	}
	NotifyPortableBackpackCollection(Collection);
	return true;
}

bool UIInventoryManager::TryApplyPortableBackpackTransfer(FItemMoveData Move, bool bOnlyCheck, bool* OutCanTransfer)
{
	auto* Item = Cast<UItemBase>(Move.SourceItem);
	if (!Item || !Item->IsPortableInventory())
	{
		return false;
	}
	if (OutCanTransfer)
	{
		*OutCanTransfer = false;
	}
	const auto* SourceOwner = Move.SourceInventory ? Move.SourceInventory->GetInventoryOwnerActor() : nullptr;
	const auto* SourceManager = SourceOwner ? SourceOwner->FindComponentByClass<UIInventoryManager>() : nullptr;
	if (SourceManager && SourceManager->IsPortableBackpackRemovalBlocked(Item))
	{
		return true;
	}
	const bool bGearTarget = Move.TargetInventory && GlobalSettings &&
		Move.TargetInventory->GetInventorySettings().InventoryTag == GlobalSettings->EquipmentInvTag;
	if (bGearTarget)
	{
		const auto* TargetOwner = Move.TargetInventory->GetInventoryOwnerActor();
		const auto* TargetManager = TargetOwner ? TargetOwner->FindComponentByClass<UIInventoryManager>() : nullptr;
		const auto* TargetEquipment = TargetOwner ? TargetOwner->FindComponentByClass<UEquipmentComponent>() : nullptr;
		const auto* State = Item->GetPortableState();
		const bool bBlockedReplacement = TargetEquipment && TargetEquipment->GetEquipmentSlotsArray().ContainsByPredicate(
			[&](const auto& Entry)
			{
				const auto* Previous = Cast<UItemBase>(Entry.EquippedItem);
				return Previous && Previous != Item && Previous->IsPortableInventory() &&
					((TargetManager && TargetManager->IsPortableBackpackRemovalBlocked(Previous)) ||
					 !IObjectDataProvider::Execute_GetItemRef(Entry.EquippedItem).bIsDroppable);
			});
		const auto* Slot = FindPortableBackpackEquipmentSlot(Move.TargetInventory, TargetEquipment,
			IObjectDataProvider::Execute_GetItemRef(Item).ItemCategory, Move.TargetSlotID);
		const bool bCompatible = Slot && !bBlockedReplacement &&
			(!State || !State->HasCraftObligations()) && IsInventoryAccessible(Move.TargetInventory) &&
			IsInventoryAccessible(Move.SourceInventory);
		if (bCompatible && !bOnlyCheck)
		{
			const bool bEquipped = TryEquipPortableBackpack(Item, Move.SourceInventory, nullptr,
				Move.TargetInventory->GetInventoryOwnerActor(), Move.TargetSlotID);
			if (OutCanTransfer)
			{
				*OutCanTransfer = bEquipped;
			}
		}
		else if (OutCanTransfer)
		{
			*OutCanTransfer = bCompatible;
		}
		return true;
	}
	if (!Item->CanStoreInInventory(Move.TargetInventory))
	{
		return true;
	}
	return false;
}

bool UIInventoryManager::IsPortableBackpackRemovalBlocked(const UItemBase* Item) const
{
	if (!Item || !Item->IsPortableInventory())
	{
		return false;
	}
	const auto* State = Item->GetPortableState();
	const auto* Equipment = GetOwner()->FindComponentByClass<UEquipmentComponent>();
	return (State && State->HasCraftObligations()) ||
		IObjectDataProvider::Execute_GetFreeAmount(const_cast<UItemBase*>(Item)) < 1 ||
		(HasActiveTradeSession() && Equipment && Equipment->IsItemEquipped(Item));
}

void UIInventoryManager::PortableEquipmentChanged(FGameplayTag Slot, UObject* Item)
{
	if (bApplyingEquipmentTransfer)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::RefreshPortableBackpack);
		return;
	}
	RefreshPortableBackpack();
}

void UIInventoryManager::RefreshPortableBackpack()
{
	if (!bUsePortableBackpacks || bApplyingEquipmentTransfer || bEndingPlay || !ItemCollectionRef || !EquipmentComponentRef)
	{
		return;
	}
	UInventoryBase* Current = GetOwner()->HasAuthority() ? nullptr : MainPawnInventoryRef.Get();
	for (const auto& Slot : EquipmentComponentRef->GetEquipmentSlotsArray())
	{
		if (auto* Item = Cast<UItemBase>(Slot.EquippedItem); Item && Item->IsPortableInventory())
		{
			if (GetOwner()->HasAuthority())
			{
				auto* State = Item->GetPortableState();
				Current = State ? State->GetInventory() : nullptr;
			}
		}
	}
	UInventoryBase* Previous = PresentedPortableInventory;
	PresentedPortableInventory = Current;
	MainPawnInventoryRef = Current;
	if (Previous && Previous != Current)
	{
		UWidgetBlueprintLibrary::CancelDragDrop();
		if (auto* Widget = ItemCollectionRef->GetContainerWidget(Previous))
		{
			if (UIInvProvider)
			{
				UIInvProvider->RemovePawnInvContainer(Widget);
			}
			Widget->RemoveFromParent();
		}
		if (auto* OldCollection = Previous->GetItemCollectionLinked())
		{
			OldCollection->OnInventoryItemsChanged.RemoveDynamic(this, &ThisClass::PortableContentChanged);
		}
		Previous->OnConfigurationChanged.RemoveDynamic(this, &ThisClass::RefreshPortableBackpack);
		Previous->OnSplitDelegate.RemoveDynamic(this, &ThisClass::ItemSplitRequest);
		ItemCollectionRef->UnregisterContainerWidget(Previous);
		InventoryWidgetInitMap.Remove(Previous);
		if (auto* ContentCollection = Previous->GetItemCollectionLinked(); ContentCollection && GetOwner()->HasAuthority())
		{
			const auto PreviousEntries = ItemCollectionRef->GetItemLocations().Items;
			for (const auto& Mapping : PreviousEntries)
			{
				if (ContentCollection->IsItemOwnedByActor(Mapping.Item))
				{
					FGameplayTag EquippedSlot;
					if (EquipmentComponentRef->FindSlotByItem(Mapping.Item, EquippedSlot))
					{
						EquipmentComponentRef->Server_UnequipItemFromSlot(EquippedSlot);
					}
					for (auto* Inventory : ItemCollectionRef->GetActorInventories())
					{
						if (Inventory && Inventory->GetInventorySettings().bIsReferenceContainer)
						{
							ItemCollectionRef->RemoveItem(Mapping.Item, Inventory->GetInventoryContainerID());
						}
					}
				}
			}
		}
	}
	if (Current)
	{
		BindInventoryEvents(Current);
		InventoryWidgetInitMap.AddUnique(Current);
		if (auto* Contents = Current->GetItemCollectionLinked())
		{
			Contents->OnInventoryItemsChanged.AddUniqueDynamic(this, &ThisClass::PortableContentChanged);
		}
		Current->OnConfigurationChanged.AddUniqueDynamic(this, &ThisClass::RefreshPortableBackpack);
	}
	if (GetOwner()->HasAuthority())
	{
		if (PawnCraftingComponentRef)
		{
			PawnCraftingComponentRef->SetInteractorInventory(Current);
		}
		if (ActiveCraftComponentRef)
		{
			ActiveCraftComponentRef->SetInteractorInventory(Current);
		}
		if (VendorProviderCurrent)
		{
			VendorProviderCurrent->SetTradePartnerInventory(Current);
			VendorProviderCurrent->SetTradePartnerItemCollection(Current ? Current->GetItemCollectionLinked() : ItemCollectionRef.Get());
		}
		GetOwner()->ForceNetUpdate();
	}
	QueueInventoryUIRefresh();
}


void UIInventoryManager::PortableContentChanged(const FString& InventoryID)
{
	if (!GetOwner()->HasAuthority() || !MainPawnInventoryRef || !ItemCollectionRef)
	{
		return;
	}
	auto* Contents = MainPawnInventoryRef->GetItemCollectionLinked();
	const auto ReferenceEntries = ItemCollectionRef->GetItemLocations().Items;
	for (const auto& Entry : ReferenceEntries)
	{
		if (ItemCollectionRef->FindMainInventoryMappingForItem(Entry.Item) ||
			Contents->ItemHasInventory(Entry.Item, MainPawnInventoryRef->GetInventoryContainerID()))
		{
			continue;
		}
		ItemCollectionRef->RemoveItemFromAllContainers(Entry.Item);
		if (EquipmentComponentRef)
		{
			FGameplayTag Slot;
			if (EquipmentComponentRef->FindSlotByItem(Entry.Item, Slot))
			{
				EquipmentComponentRef->Server_UnequipItemFromSlot(Slot);
			}
		}
	}
	for (auto* Inventory : ItemCollectionRef->GetActorInventories())
	{
		if (Inventory)
		{
			Inventory->UpdateWeightInfo();
			Inventory->OnInventoryRedrawRequested.Broadcast();
		}
	}
}
