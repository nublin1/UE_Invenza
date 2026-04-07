//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Trade/TradeComponent.h"
#include "GameFramework/Actor.h"
#include "ActorComponents/ItemCollection.h"

void UTradeComponent::OpenTradeMenu(AActor* Vendor, AActor* Buyer)
{
	this->VendorActor = Vendor;
	this->BuyerActor = Buyer;
	
	VendorItemCollection = Cast<UItemCollection>(this->VendorActor->FindComponentByClass(UItemCollection::StaticClass()));
	BuyerItemCollection = Cast<UItemCollection>(this->BuyerActor->FindComponentByClass(UItemCollection::StaticClass()));
}

void UTradeComponent::CloseTradeMenu()
{
	VendorItemCollection = nullptr;
	BuyerItemCollection = nullptr;
}



bool UTradeComponent::TryBuyItem(UItemBase* ItemToBuy)
{
	/*if (TradeSettings.bSellOnly)
		return false;

	if (ItemToBuy->GetItemRef().ItemCategory == EItemCategory::Money)
		return false;
	
	auto Result =  AccumulatePayment(VendorItemCollection, GetTotalBuyPrice(ItemToBuy));

	if (Result.bHasEnough)
	{
		return true;
	}

	if (OnFaildToBuyItemDelegate.IsBound())
		OnFaildToBuyItemDelegate.Broadcast(ItemToBuy);*/
	
	return false;
}

void UTradeComponent::BuyItem(UItemBase* ItemToBuy)
{
	if (OnBoughtItemDelegate.IsBound())
		OnBoughtItemDelegate.Broadcast(ItemToBuy);
}

bool UTradeComponent::TrySellItem(UItemBase* ItemForSale)
{
	/*if (ItemForSale->GetItemRef().ItemCategory == EItemCategory::Money)
		return false;
	
	auto Result = AccumulatePayment(BuyerItemCollection, GetTotalSellPrice(ItemForSale));
	
	if (Result.bHasEnough)
	{
		return true;
	}

	if (OnFaildToSellItemDelegate.IsBound())
		OnFaildToSellItemDelegate.Broadcast(ItemForSale);*/
	
	return false;
}

void UTradeComponent::Selltem(UItemBase* ItemsToSell)
{
	if (OnSoldItemDelegate.IsBound())
		OnSoldItemDelegate.Broadcast(ItemsToSell);
}

float UTradeComponent::GetTotalBuyPrice(UItemBase* ItemToBuy)
{
	auto FullPrice = ItemToBuy->GetItemRef().ItemTradeData.BasePrice * TradeSettings.BuyPriceFactor * ItemToBuy->GetQuantity();
	return FMath::RoundToInt(FullPrice);
}

float UTradeComponent::GetTotalSellPrice(UItemBase* ItemsToSell)
{
	auto FullPrice = ItemsToSell->GetItemRef().ItemTradeData.BasePrice * TradeSettings.SellPriceFactor * ItemsToSell->GetQuantity();
	return FMath::RoundToInt(FullPrice);
}