//  Nublin Studio 2026 All Rights Reserved.

#include "Data/Trade/TradeTypes.h"
#include "Data/Inventory/InventoryBase.h"

void FTradeSessionItems::Reset(UInventoryBase* NewInventory)
{
	Inventory = NewInventory;
	SoldItems.Reset();
	++SessionId;
}

void FTradeSessionItems::RecordTrade(const FTradeResult& Result, const TMap<UObject*, int32>& Before,
									 const TMap<UObject*, int32>& After)
{
	if (!Inventory || Result.OperationResult != ETradeResult::TR_Success)
	{
		return;
	}
	SoldItems.RemoveAll(
		[&After](const TObjectPtr<UObject>& Item)
		{
			return !IsValid(Item) || After.FindRef(Item.Get()) <= 0;
		});
	if (Result.bWasBuyingFromVendor)
	{
		return;
	}
	for (const auto& Pair : After)
	{
		if (IsValid(Pair.Key) && Pair.Value > Before.FindRef(Pair.Key))
		{
			SoldItems.AddUnique(Pair.Key);
		}
	}
}
