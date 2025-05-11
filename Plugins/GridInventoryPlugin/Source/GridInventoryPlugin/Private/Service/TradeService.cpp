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

	if (Request.Vendor->GetTradeSettings().bSellOnly)
		return ETradeResult::VendorDoesNotBuy;
	
	if (!Request.Vendor->TryBuyItem(Request.Item))
		return ETradeResult::NotEnoughMoney;

	auto VendorInv = Request.VendorContainer->GetInventoryFromContainerSlot();
	auto BuyerInv = Request.BuyerContainer->GetInventoryFromContainerSlot();

	FItemMoveData MoveData (Request.Item, BuyerInv, VendorInv);
	auto AddResult = VendorInv->HandleAddItem(MoveData, true);
	if (AddResult.OperationResult != EItemAddResult::IAR_AllItemAdded)
		return ETradeResult::NoSpaceInInventory;

	auto FullPrice = Request.Vendor->GetTotalBuyPrice(Request.Item);

	FItemMoveData ItemToBuyMoveData;
	ItemToBuyMoveData.SourceItem = Request.Item;
	ItemToBuyMoveData.SourceInventory =BuyerInv;
	ItemToBuyMoveData.TargetInventory = VendorInv;
	VendorInv->HandleAddItem(ItemToBuyMoveData);
	
	Request.Vendor->BuyItem(Request.Item);
	BuyerInv->HandleRemoveItem(Request.Item, Request.Item->GetQuantity());
	
	UItemBase* MoneyItem = UItemFactory::CreateItemByID(Request.Vendor, FName("Money"), FullPrice);
	FItemMoveData MoneyData (MoneyItem, VendorInv, BuyerInv);
	BuyerInv->HandleAddItem(MoneyData, false);

	auto Moneys = VendorInv->GetInventoryData().ItemCollectionLink->
		GetAllSameItemsInContainer(Request.VendorContainer, MoneyItem);

	float RemainingAmount = FullPrice;
	for (auto Money : Moneys)
	{
		if (!Money || RemainingAmount <= 0.f)
			break;
		
		const float TotalValue = Money->GetQuantity();
		if (TotalValue <= RemainingAmount)
		{
			VendorInv->HandleRemoveItem(Money, Money->GetQuantity());
		}
		else
		{
			VendorInv->HandleRemoveItem(Money, RemainingAmount);
		}
		
		RemainingAmount -= TotalValue;
	}
	
	return ETradeResult::Success;
}

ETradeResult UTradeService::ExecuteSell(const FTradeRequest& Request)
{
	if (Request.Item->GetItemRef().ItemNumeraticData.bCanBeSold == false )
		return ETradeResult::ItemCantBeSold;
	
	auto VendorInv = Request.VendorContainer->GetInventoryFromContainerSlot();
	auto BuyerInv = Request.BuyerContainer->GetInventoryFromContainerSlot();
	
	FItemMoveData ItemMoveData (Request.Item, VendorInv, BuyerInv);
	auto AddResult = BuyerInv->HandleAddItem(ItemMoveData, true);
	if (AddResult.OperationResult != EItemAddResult::IAR_AllItemAdded)
		return ETradeResult::NoSpaceInInventory;
	
	if (!Request.Vendor->TrySellItem(Request.Item))
		return ETradeResult::NotEnoughMoney;

	Request.Vendor->Selltem(Request.Item);

	auto FullPrice = Request.Vendor->GetTotalSellPrice(Request.Item);
	BuyerInv->HandleAddItem(ItemMoveData, false);
	if (Request.Vendor->GetTradeSettings().RemoveItemAfterPurchase)
		VendorInv->HandleRemoveItem(Request.Item, Request.Quantity);	

	UItemBase* MoneyItem = UItemFactory::CreateItemByID(VendorInv, FName("Money"), FullPrice);
	FItemMoveData MoneyData (MoneyItem, VendorInv, BuyerInv);
	VendorInv->HandleAddItem(MoneyData, false);

	auto Moneys = BuyerInv->GetInventoryData().ItemCollectionLink->
		GetAllSameItemsInContainer(Request.BuyerContainer, MoneyItem);

	float RemainingAmount = FullPrice;
	for (auto Money : Moneys)
	{
		if (!Money || RemainingAmount <= 0.f)
			break;
		
		const float TotalValue = Money->GetQuantity();
		if (TotalValue <= RemainingAmount)
		{
			BuyerInv->HandleRemoveItem(Money, Money->GetQuantity());
		}
		else
		{
			BuyerInv->HandleRemoveItem(Money, RemainingAmount);
		}
		
		RemainingAmount -= TotalValue;
	}
	
	return ETradeResult::Success;
}
