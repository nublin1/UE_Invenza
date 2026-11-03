// Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/UIInventoryManager.h"

#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Inventory/ListInventory/ListInventory.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/Items/itemBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "Subsystems/ModalWindowManager.h"
#include "UI/Core/Modal/ModalTypes.h"
#include "Utility/InvenzayUtility.h"

static APlayerController* FindTakeAmountController(const UIInventoryManager* Manager)
{
	const APawn* Pawn = Cast<APawn>(Manager->GetOwner());
	return Pawn ? Cast<APlayerController>(Pawn->GetController()) : Cast<APlayerController>(Manager->GetOwner());
}

static void NotifyTakeAmountInventory(UInventoryBase* Inventory)
{
	if (!IsValid(Inventory) || !IsValid(Inventory->GetItemCollectionLinked()))
	{
		return;
	}
	UInventoryCollectionComponent* Collection = Inventory->GetItemCollectionLinked();
	if (UListInventory* List = Cast<UListInventory>(Inventory))
	{
		List->InventorySlots.Reset();
		for (const FItemMapping& Mapping : Collection->GetAllMappingsByContainer(Inventory->GetInventoryContainerID()))
		{
			List->InventorySlots.Append(Mapping.OccupiedSlots);
		}
		List->RefreshItemEntries();
	}
	Inventory->UpdateWeightInfo();
	Inventory->UpdateMoneyInfo();
	Inventory->OnInventoryRedrawRequested.Broadcast();
	Collection->OnInventoryItemsChanged.Broadcast(Inventory->GetInventoryContainerID());
	Collection->GetOwner()->ForceNetUpdate();
}

bool UIInventoryManager::CanTakeAmountFromInventory(UInventoryBase* Inventory) const
{
	if (bEndingPlay || !IsValid(Inventory) || !IsValid(ItemCollectionRef) || !IsValid(MainPawnInventoryRef) ||
		!IsInventoryAccessible(Inventory) || Inventory->GetInventorySettings().bIsReferenceContainer ||
		Inventory->GetItemCollectionLinked() == ItemCollectionRef ||
		(!bUsePortableBackpacks && MainPawnInventoryRef->GetItemCollectionLinked() != ItemCollectionRef) ||
		MainPawnInventoryRef->GetInventorySettings().bIsReferenceContainer ||
		(!bUsePortableBackpacks && !ItemCollectionRef->GetActorInventories().Contains(MainPawnInventoryRef)) || CharacterInventoryAccess)
	{
		return false;
	}
	UInventoryCollectionComponent* SourceCollection = Inventory->GetItemCollectionLinked();
	if (!IsValid(SourceCollection) || !IsValid(SourceCollection->GetOwner()) ||
		SourceCollection->GetOwner()->IsActorBeingDestroyed() ||
		Inventory->GetInventoryOwnerActor() != SourceCollection->GetOwner() ||
		ItemCollectionRef->GetOwner() != GetOwner())
	{
		return false;
	}
	const FLinkedInventories Linked = ItemCollectionRef->GetLinkedInventories();
	if (Inventory == Linked.VendorInventory || IsTradeSessionInventory(Inventory) ||
		(Inventory != Linked.ExternalInventory && !IsStorageInventoryAccessible(Inventory)))
	{
		return false;
	}
	if (GetOwner()->HasAuthority())
	{
		return IsValid(TrackedInteractionTarget) && TrackedInteractionTarget->IsInteractionOwnedBy(GetOwner()) &&
			UInteractionComponent::IsWithinInteractionDistance(GetOwner(), TrackedInteractionTarget);
	}
	return true;
}

EInventoryTakeAmountResult UIInventoryManager::HandleTakeAmount(const FInventoryTakeAmountRequest& Request)
{
	if (!GetOwner()->HasAuthority() || !Request.RequestID.IsValid() || Request.Move.RequestedAmount <= 0)
	{
		return EInventoryTakeAmountResult::InvalidRequest;
	}
	if (bApplyingAmountTransfer || bApplyingEquipmentTransfer ||
		(GetWorld()->GetGameInstance() && GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->IsApplyingSnapshot()))
	{
		return EInventoryTakeAmountResult::Busy;
	}
	TGuardValue<bool> TransferGuard(bApplyingAmountTransfer, true);
	UInventoryBase* Source = Request.Move.SourceInventory;
	UInventoryBase* Target = MainPawnInventoryRef;
	if (!CanTakeAmountFromInventory(Source) || !IsValid(Target))
	{
		return EInventoryTakeAmountResult::AccessDenied;
	}
	UInventoryCollectionComponent* SourceCollection = Source->GetItemCollectionLinked();
	UInventoryCollectionComponent* TargetCollection = Target->GetItemCollectionLinked();
	UItemBase* Original = Cast<UItemBase>(Request.Move.SourceItem);
	if (!IsValid(SourceCollection) || !IsValid(TargetCollection) || !IsValid(Original))
	{
		return EInventoryTakeAmountResult::UnsupportedItem;
	}
	const FItemMapping* Mapping = SourceCollection->FindItemMappingByContainerName(Original, Source->GetInventoryContainerID());
	if (!Mapping || Mapping->bIsReferenceContainer || !Mapping->OccupiedSlots.ContainsByPredicate(
		[&](UInventorySlotData* Slot)
		{
			return Slot && Slot->InventorySlotInfo.SlotGuid == Request.Move.SourceSlotID;
		}))
	{
		return EInventoryTakeAmountResult::SourceChanged;
	}
	const FItemMapping SourceMapping = *Mapping;
	const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(Original);
	const int32 Available = IObjectDataProvider::Execute_GetFreeAmount(Original);
	const int32 Amount = Request.Move.RequestedAmount;
	if (Quantity != Request.ExpectedSourceQuantity || Available != Request.ExpectedAvailableAmount)
	{
		return EInventoryTakeAmountResult::SourceChanged;
	}
	if (Amount > Quantity || Amount > Available)
	{
		return EInventoryTakeAmountResult::AmountReserved;
	}
	if (!Target->CanReceiveItem(Original))
	{
		return EInventoryTakeAmountResult::NoCapacity;
	}
	const FInventorySettings TargetSettings = Target->GetInventorySettings();
	const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(Original);
	const int32 MaxStack = FMath::Max(1, Meta.ItemNumeraticData.MaxStackSizeInCharacter);
	const double SingleWeight = IObjectDataProvider::Execute_GetItemSingleWeight(Original);
	if ((!IObjectDataProvider::Execute_IsStackable(Original) && Amount != 1) ||
		!FMath::IsFinite(SingleWeight) || SingleWeight < 0 ||
		(!Cast<USlotbasedInventory>(Target) && !Cast<UListInventory>(Target)))
	{
		return EInventoryTakeAmountResult::UnsupportedItem;
	}

	const TArray<UObject*> TargetItems = TargetCollection->GetAllItemsByContainer(Target->GetInventoryContainerID());
	TMap<UItemBase*, int32> QuantitiesBefore;
	TMap<UItemBase*, int32> MergeAmounts;
	TMap<UItemBase*, FItemMapping> NewMappings;
	TSet<UInventorySlotData*> Occupied;
	double FinalWeight = SingleWeight * Amount;
	int32 Remaining = Amount;
	const TMap<FName, FString> ItemState = Original->CaptureSaveState();
	for (UObject* Item : TargetItems)
	{
		if (!IsValid(Item) || !Item->Implements<UObjectDataProvider>())
		{
			return EInventoryTakeAmountResult::UnsupportedItem;
		}
		FinalWeight += static_cast<double>(IObjectDataProvider::Execute_GetItemStackWeight(Item));
		if (const FItemMapping* ExistingMapping = TargetCollection->FindItemMappingByContainerName(Item, Target->GetInventoryContainerID()))
		{
			for (UInventorySlotData* Slot : ExistingMapping->OccupiedSlots)
			{
				Occupied.Add(Slot);
			}
		}
		UItemBase* Stack = Cast<UItemBase>(Item);
		if (!Stack)
		{
			continue;
		}
		const int32 StackQuantity = IObjectDataProvider::Execute_GetQuantity(Stack);
		QuantitiesBefore.Add(Stack, StackQuantity);
		const FItemMetaData StackMeta = IObjectDataProvider::Execute_GetItemRef(Stack);
		const bool bCompatible = Stack->GetClass() == Original->GetClass() &&
			UInvenzayUtility::bIsSameItems(Stack, Original) &&
			FItemMetaData::StaticStruct()->CompareScriptStruct(&Meta, &StackMeta, PPF_None) &&
			ItemState.OrderIndependentCompareEqual(Stack->CaptureSaveState());
		if (Remaining > 0 && bCompatible && IObjectDataProvider::Execute_IsStackable(Stack))
		{
			const int32 Added = FMath::Min(Remaining, FMath::Max(0, MaxStack - StackQuantity));
			if (Added > 0)
			{
				MergeAmounts.Add(Stack, Added);
				Remaining -= Added;
			}
		}
	}
	if (!FMath::IsFinite(FinalWeight) || FinalWeight < 0 ||
		(TargetSettings.InventoryMaxWeightCapacity >= 0 && FinalWeight > TargetSettings.InventoryMaxWeightCapacity + KINDA_SMALL_NUMBER))
	{
		return EInventoryTakeAmountResult::NoCapacity;
	}

	while (Remaining > 0)
	{
		if (TargetSettings.MaxStackCount >= 0 && TargetItems.Num() + NewMappings.Num() >= TargetSettings.MaxStackCount)
		{
			return EInventoryTakeAmountResult::NoCapacity;
		}
		FItemMapping NewMapping;
		NewMapping.InventoryID = Target->GetInventoryContainerID();
		NewMapping.InventoryType = TargetSettings.InventoryType;
		if (USlotbasedInventory* Grid = Cast<USlotbasedInventory>(Target))
		{
			for (int32 Rotation = 0; Rotation < 2 && NewMapping.OccupiedSlots.IsEmpty(); ++Rotation)
			{
				const EItemOrientationType Orientation = Rotation == 0 ? EItemOrientationType::Horizontal : EItemOrientationType::Vertical;
				const FIntPoint Footprint = TargetSettings.InventorySlotBasedSettings.bIgnoreItemSize ?
					FIntPoint(1, 1) : IObjectDataProvider::Execute_GetItemSize(Original, Orientation);
				if (Footprint.X <= 0 || Footprint.Y <= 0)
				{
					continue;
				}
				for (UInventorySlotData* Candidate : Grid->GetInventorySlots())
				{
					if (!Candidate)
					{
						continue;
					}
					const TArray<UInventorySlotData*> Slots = Grid->GetSlotsForItemAt(Candidate->InventorySlotInfo.CellPosition, Original, Orientation);
					if (static_cast<int64>(Slots.Num()) != static_cast<int64>(Footprint.X) * Footprint.Y || Slots.ContainsByPredicate(
						[&](UInventorySlotData* Slot)
						{
							return !Slot || Occupied.Contains(Slot) || Grid->IsSlotReservedForPlacement(Slot) ||
								!Meta.ItemCategory.MatchesTag(Slot->InventorySlotInfo.AllowedCategory);
						}))
					{
						continue;
					}
					NewMapping.OccupiedSlots = Slots;
					NewMapping.ItemOrientation = Orientation;
					break;
				}
			}
		}
		else
		{
			NewMapping.OccupiedSlots.Add(UInventorySlotData::Create(Target));
		}
		if (NewMapping.OccupiedSlots.IsEmpty())
		{
			return EInventoryTakeAmountResult::NoCapacity;
		}
		const int32 StackAmount = FMath::Min(Remaining, MaxStack);
		UItemBase* Copy = Original->CreateTransferCopy(TargetCollection->GetOwner(), StackAmount);
		if (!Copy)
		{
			return EInventoryTakeAmountResult::UnsupportedItem;
		}
		NewMappings.Add(Copy, NewMapping);
		for (UInventorySlotData* Slot : NewMapping.OccupiedSlots)
		{
			Occupied.Add(Slot);
		}
		Remaining -= StackAmount;
	}

	// Recheck after authored item-state hooks, before the event-free commit.
	if (!CanTakeAmountFromInventory(Source) || IObjectDataProvider::Execute_GetQuantity(Original) != Quantity ||
		IObjectDataProvider::Execute_GetFreeAmount(Original) != Available ||
		TargetCollection->GetAllItemsByContainer(Target->GetInventoryContainerID()).Num() != TargetItems.Num())
	{
		return EInventoryTakeAmountResult::SourceChanged;
	}
	for (const auto& Pair : QuantitiesBefore)
	{
		if (!TargetCollection->ItemHasInventory(Pair.Key, Target->GetInventoryContainerID()) ||
			IObjectDataProvider::Execute_GetQuantity(Pair.Key) != Pair.Value)
		{
			return EInventoryTakeAmountResult::SourceChanged;
		}
	}
	TSet<UInventoryBase*> ChangedInventories = {Source, Target};
	for (const FItemMapping* Existing : SourceCollection->FindAllMappingsForItem(Original))
	{
		if (UInventoryBase* Inventory = SourceCollection->GetInventoryByID(Existing->InventoryID))
		{
			ChangedInventories.Add(Inventory);
		}
	}
	TArray<FItemMapping> RemovedMappings;
	if (Amount == Quantity)
	{
		for (const FItemMapping* Existing : SourceCollection->FindAllMappingsForItem(Original))
		{
			RemovedMappings.Add(*Existing);
		}
		if (NewMappings.Num() == 1)
		{
			for (const auto& Pair : NewMappings)
			{
				Pair.Key->SetPersistentInstanceID(Original->GetPersistentInstanceID());
			}
		}
	}
	for (const auto& Pair : MergeAmounts)
	{
		Pair.Key->SetQuantityForTransfer(QuantitiesBefore.FindChecked(Pair.Key) + Pair.Value);
		TargetCollection->MarkItemAsDirty(Pair.Key);
	}
	for (const auto& Pair : NewMappings)
	{
		TargetCollection->AddItem(Pair.Key, Pair.Value);
	}
	if (Amount == Quantity)
	{
		for (const FItemMapping& Removed : RemovedMappings)
		{
			if (UInventoryBase* Inventory = SourceCollection->GetInventoryByID(Removed.InventoryID))
			{
				ChangedInventories.Add(Inventory);
			}
			SourceCollection->RemoveItem(Original, Removed.InventoryID);
		}
	}
	else
	{
		Original->SetQuantityForTransfer(Quantity - Amount);
		SourceCollection->MarkItemAsDirty(Original);
	}

	// Notifications observe the completed source and target state, never an intermediate split.
	if (Amount == Quantity)
	{
		Source->OnItemRemovedDelegate.Broadcast(SourceMapping, Original);
	}
	else
	{
		Source->OnUnstackedItemDelegate.Broadcast(Original);
	}
	for (const auto& Pair : MergeAmounts)
	{
		Target->OnStackedItemDelegate.Broadcast(Pair.Key);
	}
	for (const auto& Pair : NewMappings)
	{
		FItemMapping AddedMapping = Pair.Value;
		Target->OnAddItemDelegate.Broadcast(AddedMapping, Pair.Key);
	}
	for (UInventoryBase* Inventory : ChangedInventories)
	{
		NotifyTakeAmountInventory(Inventory);
	}
	return EInventoryTakeAmountResult::Success;
}

void UIInventoryManager::SubmitTakeAmount(const FModalResult& Result)
{
	if (PendingTakeAmountRequestID.IsValid() || !PendingContextItem || !PendingContextInv)
	{
		return;
	}
	if (UInventoryCollectionComponent* Collection = PendingContextInv->GetItemCollectionLinked())
	{
		if (const FItemMapping* Mapping = Collection->FindItemMappingByContainerName(
			PendingContextItem, PendingContextInv->GetInventoryContainerID()))
		{
			if (!Mapping->OccupiedSlots.IsEmpty() && Mapping->OccupiedSlots[0])
			{
				PendingContextSlotGuid = Mapping->OccupiedSlots[0]->InventorySlotInfo.SlotGuid;
			}
		}
	}
	FInventoryTakeAmountRequest Request;
	Request.RequestID = FGuid::NewGuid();
	Request.Move.SourceItem = PendingContextItem;
	Request.Move.SourceInventory = PendingContextInv;
	Request.Move.SourceSlotID = PendingContextSlotGuid;
	Request.Move.RequestedAmount = Result.HeaderResult.SelectedQuantity;
	Request.ExpectedSourceQuantity = Result.HeaderResult.ExpectedSourceQuantity;
	Request.ExpectedAvailableAmount = Result.HeaderResult.ExpectedAvailableAmount;
	PendingTakeAmountRequestID = Request.RequestID;
	APlayerController* Controller = FindTakeAmountController(this);
	if (Controller && Controller->GetLocalPlayer())
	{
		Controller->GetLocalPlayer()->GetSubsystem<UModalWindowManager>()->SetTakeAmountSubmitting();
	}
	Server_TakeItemAmount(Request);
}

void UIInventoryManager::Server_TakeItemAmount_Implementation(FInventoryTakeAmountRequest Request)
{
	const EInventoryTakeAmountResult Result = HandleTakeAmount(Request);
	UE_LOG(LogTemp, Verbose, TEXT("Take amount: owner=%s amount=%d result=%d source=%s slot=%s"),
		*GetNameSafe(GetOwner()), Request.Move.RequestedAmount, static_cast<int32>(Result),
		*GetNameSafe(Request.Move.SourceInventory), *Request.Move.SourceSlotID.ToString());
	const bool bMayRead = CanTakeAmountFromInventory(Request.Move.SourceInventory) &&
		IsValid(Request.Move.SourceItem) && Request.Move.SourceItem->Implements<UObjectDataProvider>() &&
		Request.Move.SourceInventory->GetItemCollectionLinked()->ItemHasInventory(
			Request.Move.SourceItem, Request.Move.SourceInventory->GetInventoryContainerID());
	Client_TakeItemAmountResult(Request.RequestID, Result,
		bMayRead ? IObjectDataProvider::Execute_GetQuantity(Request.Move.SourceItem) : 0,
		bMayRead ? IObjectDataProvider::Execute_GetFreeAmount(Request.Move.SourceItem) : 0);
}

void UIInventoryManager::Client_TakeItemAmountResult_Implementation(FGuid RequestID, EInventoryTakeAmountResult Result,
	int32 SourceQuantity, int32 AvailableAmount)
{
	if (RequestID != PendingTakeAmountRequestID)
	{
		return;
	}
	PendingTakeAmountRequestID.Invalidate();
	APlayerController* Controller = FindTakeAmountController(this);
	UModalWindowManager* Modal = Controller && Controller->GetLocalPlayer() ?
		Controller->GetLocalPlayer()->GetSubsystem<UModalWindowManager>() : nullptr;
	if (Modal)
	{
		Modal->CompleteTakeAmountStep(Result == EInventoryTakeAmountResult::Success,
			GlobalSettings ? GlobalSettings->TakeAmountMessages.FindRef(Result) : FText::GetEmpty(),
			SourceQuantity, AvailableAmount);
	}
}
