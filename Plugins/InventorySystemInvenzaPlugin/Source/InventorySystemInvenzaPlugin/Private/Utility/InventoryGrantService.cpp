// Nublin Studio 2026 All Rights Reserved.

#include "Utility/InventoryGrantService.h"

#include "ActorComponents/Interactable/PickupComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Items/itemBase.h"
#include "GameFramework/Actor.h"
#include "UObject/StrongObjectPtr.h"
#include "Utility/InvenzayUtility.h"

FInventoryGrantResult FInventoryGrantService::Grant(UItemBase* Item,
	const TArray<UInventoryBase*>& Destinations, AActor* Recipient, const FInventoryGrantPolicy& Policy)
{
	FInventoryGrantResult Result;
	if (!IsValid(Item))
	{
		return Result;
	}
	Result.Remaining = IObjectDataProvider::Execute_GetQuantity(Item);
	if (!IsValid(Recipient) || !Recipient->HasAuthority() || Result.Remaining <= 0)
	{
		return Result;
	}
	TSet<UInventoryBase*> Visited;
	for (auto* Inventory : Destinations)
	{
		if (Result.Remaining <= 0 || !IsValid(Inventory) || Visited.Contains(Inventory) ||
			!IsValid(Inventory->GetInventoryOwnerActor()) ||
			Inventory->GetInventoryOwnerActor()->GetWorld() != Recipient->GetWorld() ||
			Inventory->GetInventorySettings().bIsReferenceContainer)
		{
			continue;
		}
		Visited.Add(Inventory);
		TStrongObjectPtr<UItemBase> Copy(Item->CreateTransferCopy(Inventory->GetInventoryOwnerActor(), Result.Remaining));
		if (!Copy.IsValid())
		{
			continue;
		}
		if (!Policy.bAllowPartial)
		{
			Inventory->UpdateWeightInfo();
			const float Capacity = Inventory->GetInventorySettings().InventoryMaxWeightCapacity;
			const float RequestedWeight = IObjectDataProvider::Execute_GetItemSingleWeight(Copy.Get()) * Result.Remaining;
			if (Capacity >= 0.f && Inventory->GetInventoryTotalWeight() + RequestedWeight > Capacity + KINDA_SMALL_NUMBER)
			{
				continue;
			}
			FItemMoveData Preview;
			Preview.SourceItem = Copy.Get();
			Preview.TargetInventory = Inventory;
			if (Inventory->HandleAddItem(Preview, true).ActualAmountAdded != Result.Remaining)
			{
				continue;
			}
		}
		const int32 Added = UInvenzayUtility::GrantStartingStack(Inventory, Copy.Get());
		Result.Added += Added;
		Result.Remaining -= Added;
		Item->SetQuantityForTransfer(Result.Remaining);
	}
	if (Result.Remaining > 0 && Policy.Overflow != EInventoryGrantOverflow::Retain)
	{
		auto* Drop = UInvenzayUtility::DropItem(Recipient->GetWorld(), Recipient,
			IObjectDataProvider::Execute_GetItemRow(Item), Result.Remaining,
			Policy.DropTransform.GetLocation(), Policy.DropTransform.Rotator());
		auto* Pickup = Drop ? Drop->FindComponentByClass<UPickupComponent>() : nullptr;
		auto* Copy = Pickup ? Item->CreateIdentityTransferCopy(Pickup) : nullptr;
		if (Copy)
		{
			Pickup->RestorePersistentItem(Copy);
			Result.Dropped = Result.Remaining;
			Result.Remaining = 0;
		}
		else
		{
			if (Drop)
			{
				Drop->Destroy();
			}
			if (Policy.Overflow == EInventoryGrantOverflow::DropAndDiscardOnFailure)
			{
				Result.Discarded = Result.Remaining;
				Result.Remaining = 0;
			}
			if (Policy.bLogOverflowFailure)
			{
				UE_LOG(LogTemp, Error, TEXT("Grant overflow failed for %s (%s), recipient %s: discarded %d, retained %d."),
					*IObjectDataProvider::Execute_GetItemID(Item).ToString(), *Item->GetPersistentInstanceID().ToString(),
					*GetNameSafe(Recipient), Result.Discarded, Result.Remaining);
			}
		}
		Item->SetQuantityForTransfer(Result.Remaining);
	}
	return Result;
}
