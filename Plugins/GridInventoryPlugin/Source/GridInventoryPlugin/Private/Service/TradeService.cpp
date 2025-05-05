//  Nublin Studio 2025 All Rights Reserved.


#include "Service/TradeService.h"

#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/TradeComponent.h"
#include "ActorComponents/Items/itemBase.h"
#include "Factory/ItemFactory.h"
#include "UI/Inventory/InventoryTypes.h"
#include "UI/Inventory/UInventoryWidgetBase.h"

ETradeResult UTradeService::ExecuteBuy(const FTradeRequest& Request)
{
	if (Request.Item->GetItemRef().ItemNumeraticData.bCanBeSold == false )
		return ETradeResult::ItemCantBeSold;
	
	if (!Request.Vendor->TryBuyItem(Request.Item))
		return ETradeResult::NotEnoughMoney;

	FItemMoveData MoveData (Request.Item, Request.BuyerInv, Request.VendorInv);
	auto AddResult = Request.VendorInv->HandleAddItem(MoveData, true);
	if (AddResult.OperationResult != EItemAddResult::IAR_AllItemAdded)
		return ETradeResult::NoSpaceInInventory;

	auto FullPrice = Request.Vendor->GetTotalBuyPrice(Request.Item);

	FItemMoveData ItemToBuyMoveData;
	ItemToBuyMoveData.SourceItem = Request.Item;
	ItemToBuyMoveData.SourceInventory =Request.BuyerInv;
	ItemToBuyMoveData.TargetInventory = Request.VendorInv;
	Request.VendorInv->HandleAddItem(ItemToBuyMoveData);
	
	Request.Vendor->BuyItem(Request.Item);
	Request.BuyerInv->HandleRemoveItem(Request.Item, Request.Item->GetQuantity());
	
	UItemBase* MoneyItem = UItemFactory::CreateItemByID(Request.Vendor, FName("Money"), FullPrice);
	FItemMoveData MoneyData (MoneyItem, Request.VendorInv, Request.BuyerInv);
	Request.BuyerInv->HandleAddItem(MoneyData, false);

	auto Moneys = Request.VendorInv->GetInventoryData().ItemCollectionLink->
		GetAllSameItemsInContainer(Request.VendorInv, MoneyItem);

	float RemainingAmount = FullPrice;
	for (auto Money : Moneys)
	{
		if (!Money || RemainingAmount <= 0.f)
			break;
		
		const float TotalValue = Money->GetQuantity();
		if (TotalValue <= RemainingAmount)
		{
			Request.VendorInv->HandleRemoveItem(Money, Money->GetQuantity());
		}
		else
		{
			Request.VendorInv->HandleRemoveItem(Money, RemainingAmount);
		}
		
		RemainingAmount -= TotalValue;
	}
	
	return ETradeResult::Success;
}

ETradeResult UTradeService::ExecuteSell(const FTradeRequest& Request)
{
	if (Request.Item->GetItemRef().ItemNumeraticData.bCanBeSold == false )
		return ETradeResult::ItemCantBeSold;

	FItemMoveData ItemMoveData (Request.Item, Request.VendorInv, Request.BuyerInv);
	auto AddResult = Request.BuyerInv->HandleAddItem(ItemMoveData, true);
	if (AddResult.OperationResult != EItemAddResult::IAR_AllItemAdded)
		return ETradeResult::NoSpaceInInventory;
	
	if (!Request.Vendor->TrySellItem(Request.Item))
		return ETradeResult::NotEnoughMoney;

	Request.Vendor->Selltem(Request.Item);

	auto FullPrice = Request.Vendor->GetTotalSellPrice(Request.Item);
	Request.BuyerInv->HandleAddItem(ItemMoveData, false);
	if (Request.Vendor->GetTradeSettings().RemoveItemAfterPurchase)
		Request.VendorInv->HandleRemoveItem(Request.Item, Request.Quantity);	

	UItemBase* MoneyItem = UItemFactory::CreateItemByID(Request.VendorInv, FName("Money"), FullPrice);
	FItemMoveData MoneyData (MoneyItem, Request.VendorInv, Request.BuyerInv);
	Request.VendorInv->HandleAddItem(MoneyData, false);

	auto Moneys = Request.BuyerInv->GetInventoryData().ItemCollectionLink->
		GetAllSameItemsInContainer(Request.BuyerInv, MoneyItem);

	float RemainingAmount = FullPrice;
	for (auto Money : Moneys)
	{
		if (!Money || RemainingAmount <= 0.f)
			break;
		
		const float TotalValue = Money->GetQuantity();
		if (TotalValue <= RemainingAmount)
		{
			Request.BuyerInv->HandleRemoveItem(Money, Money->GetQuantity());
		}
		else
		{
			Request.BuyerInv->HandleRemoveItem(Money, RemainingAmount);
		}
		
		RemainingAmount -= TotalValue;
	}
	
	return ETradeResult::Success;
}
