//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/TradeComponent.h"
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

float UTradeComponent::CalculateAvailableMoney(UItemCollection* Collection)
{	
	TArray<UItemBase*> SellerMoneyItems = Collection->GetAllItemsByCategory(EItemCategory::Money);
	if (SellerMoneyItems.Num() <= 0)
	{
		return 0;
	}

	float AvailableMoney = 0.0f;
	for (UItemBase* MoneyItem : SellerMoneyItems)
	{
		if (MoneyItem)
		{
			AvailableMoney += MoneyItem->GetItemRef().ItemNumeraticData.BasePrice * MoneyItem->GetQuantity();
		}
	}

	return AvailableMoney;
}

FMoneyCalculationResult UTradeComponent::AccumulatePayment(UItemCollection* ItemCollection, const float FullPrice)
{
	FMoneyCalculationResult Result;
	Result.AvailableMoney  = CalculateAvailableMoney(ItemCollection);
	
	TArray<UItemBase*> SellerMoneyItems = ItemCollection->GetAllItemsByCategory(EItemCategory::Money);
	if (SellerMoneyItems.IsEmpty())
	{
		Result.bHasEnough =false;
	}
	
	for (UItemBase* MoneyItem : SellerMoneyItems)
	{
		int32 Quantity = MoneyItem->GetQuantity();
		float UnitValue = MoneyItem->GetItemRef().ItemNumeraticData.BasePrice;
		int32 NeededQuantity = FMath::CeilToInt((FullPrice - Result.AccumulatedRequiredValue) / UnitValue);
		int32 RemoveQuantity = FMath::Min(Quantity, NeededQuantity);

		if (RemoveQuantity > 0)
		{
			Result.AccumulatedRequiredValue += RemoveQuantity * UnitValue;
		}
	}

	Result.bHasEnough = Result.AvailableMoney >= Result.AccumulatedRequiredValue;

	return Result;
}

bool UTradeComponent::TryBuyItem(UItemBase* ItemToBuy)
{
	if (bSellOnly)
		return false;
	
	auto Result =  AccumulatePayment(VendorItemCollection, GetTotalBuyPrice(ItemToBuy));

	if (Result.bHasEnough)
	{
		return true;
	}

	if (OnFaildToBuyItemDelegate.IsBound())
		OnFaildToBuyItemDelegate.Broadcast(ItemToBuy);
	
	return false;
}

void UTradeComponent::BuyItem(UItemBase* ItemToBuy)
{
	if (OnBoughtItemDelegate.IsBound())
		OnBoughtItemDelegate.Broadcast(ItemToBuy);
}

bool UTradeComponent::TrySellItem(UItemBase* ItemForSale)
{	
	auto Result = AccumulatePayment(BuyerItemCollection, GetTotalSellPrice(ItemForSale));
	
	if (Result.bHasEnough)
	{
		return true;
	}

	if (OnFaildToSellItemDelegate.IsBound())
		OnFaildToSellItemDelegate.Broadcast(ItemForSale);
	
	return false;
}

void UTradeComponent::Selltem(UItemBase* ItemsToSell)
{
	if (OnSoldItemDelegate.IsBound())
		OnSoldItemDelegate.Broadcast(ItemsToSell);
}

float UTradeComponent::GetTotalBuyPrice(UItemBase* ItemToBuy)
{
	auto FullPrice = ItemToBuy->GetItemRef().ItemNumeraticData.BasePrice * BuyPriceFactor * ItemToBuy->GetQuantity();
	return FullPrice;
}

float UTradeComponent::GetTotalSellPrice(UItemBase* ItemsToSell)
{
	auto FullPrice = ItemsToSell->GetItemRef().ItemNumeraticData.BasePrice * BuyPriceFactor * ItemsToSell->GetQuantity();
	return FullPrice;
}