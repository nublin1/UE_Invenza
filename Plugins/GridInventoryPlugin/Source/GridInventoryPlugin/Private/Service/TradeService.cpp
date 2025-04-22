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
	if (!Request.Vendor->TryBuyItem(Request.Item))
		return ETradeResult::VendorOutOfStock;

	FItemMoveData MoveData (Request.Item, Request.BuyerInv, Request.VendorInv);
	auto AddResult = Request.BuyerInv->HandleAddItem(MoveData, true);
	if (AddResult.OperationResult != EItemAddResult::IAR_AllItemAdded)
		return ETradeResult::NoSpaceInInventory;

	auto FullPrice = Request.Vendor->GetTotalBuyPrice(Request.Item);

	Request.Vendor->BuyItem(Request.Item, Request.VendorInv, Request.BuyerInv);
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
	return ETradeResult::Success;
}
