//  Nublin Studio 2025 All Rights Reserved.


#include "Service/TradeService.h"

#include "ActorComponents/TradeComponent.h"
#include "ActorComponents/Items/itemBase.h"
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

	Request.Vendor->BuyItem(Request.Item);
	Request.BuyerInv->HandleRemoveItem(Request.Item, Request.Item->GetQuantity());
	
	return ETradeResult::Success;
}

ETradeResult UTradeService::ExecuteSell(const FTradeRequest& Request)
{
	return ETradeResult::Success;
}
