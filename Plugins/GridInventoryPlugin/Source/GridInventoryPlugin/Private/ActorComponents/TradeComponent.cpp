//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/TradeComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Inventory/UInventoryWidgetBase.h"
#include "World/AUIManagerActor.h"

void UTradeComponent::OpenTradeMenu(AActor* Vendor, AActor* Buyer)
{
	this->VendorActor = Vendor;
	this->BuyerActor = Buyer;
	
	VendorItemCollection = Cast<UItemCollection>(this->VendorActor->FindComponentByClass(UItemCollection::StaticClass()));
	BuyerItemCollection = Cast<UItemCollection>(this->BuyerActor->FindComponentByClass(UItemCollection::StaticClass()));
	
	
	/*SellerInvWidget->GetInventoryFromContainerSlot()->AddCheck(
			EInventoryCheckType::PreAdd,
			TEXT("TryBuyItemCheck"),
			[this](UItemBase* Item) -> bool
			{
				return this->TryBuyItem(Item);
			});
	SellerInvWidget->GetInventoryFromContainerSlot()->AddCheck(
		EInventoryCheckType::PreTransfer,
		TEXT("TrySellItemCheck"),
		[this](UItemBase* Item) -> bool
		{
			return this->TrySellItem(Item);
		});*/
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
		
	if (ItemToBuy->GetItemRef().ItemCategory == EItemCategory::Money)
		return false;

	auto FullPrice = ItemToBuy->GetItemRef().ItemNumeraticData.BasePrice * BuyPriceFactor * ItemToBuy->GetQuantity();
	auto Result =  AccumulatePayment(VendorItemCollection, FullPrice);

	if (Result.bHasEnough)
	{
		//BuyItem(ItemToBuy, Result);
		return true;
	}

	if (OnFaildToBuyItemDelegate.IsBound())
		OnFaildToBuyItemDelegate.Broadcast(ItemToBuy);
	
	return false;
}


void UTradeComponent::BuyItem(UItemBase* ItemToBuy, UUInventoryWidgetBase* VendorInv, UUInventoryWidgetBase* BuyerInv)
{
	auto Manager = Cast<AUIManagerActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AUIManagerActor::StaticClass()));
	if (!Manager)
		return;
	
	FItemMoveData ItemToBuyMoveData;
	ItemToBuyMoveData.SourceItem = ItemToBuy;
	ItemToBuyMoveData.SourceInventory = BuyerInv;
	ItemToBuyMoveData.TargetInventory = VendorInv;
	
	VendorInv->HandleAddItem(ItemToBuyMoveData);

	if (OnBoughtItemDelegate.IsBound())
		OnBoughtItemDelegate.Broadcast(ItemToBuy);
}

bool UTradeComponent::TrySellItem(UItemBase* ItemForSale)
{
	if (ItemForSale->GetItemRef().ItemCategory == EItemCategory::Money)
		return false;
	
	auto FullPrice = ItemForSale->GetItemRef().ItemNumeraticData.BasePrice * BuyPriceFactor * ItemForSale->GetQuantity();
	auto Result =  AccumulatePayment(BuyerItemCollection, FullPrice);
	
	if (Result.bHasEnough)
	{
		//Selltem(ItemForSale, Result);
		return true;
	}

	if (OnFaildToSellItemDelegate.IsBound())
		OnFaildToSellItemDelegate.Broadcast(ItemForSale);
	
	return false;
}

void UTradeComponent::Selltem(UItemBase* ItemsToSell, FMoneyCalculationResult Result)
{
	/*TArray<UItemBase*> BuyerMoneyItems = BuyerItemCollection->GetAllItemsByCategory(EItemCategory::Money);
	auto MoneyItem = BuyerMoneyItems[0]->DuplicateItem();
	MoneyItem->SetQuantity(Result.AccumulatedRequiredValue);
	BuyerInvWidget->GetInventoryFromContainerSlot()->HandleRemoveItem(BuyerMoneyItems[0], Result.AccumulatedRequiredValue);
	
	FItemMoveData MoneyMoveData;
	MoneyMoveData.SourceItem = MoneyItem;
	MoneyMoveData.TargetInventory = SellerInvWidget->GetInventoryFromContainerSlot();
	ManagerActor->ItemTransferRequest(MoneyMoveData);
	
	if (OnSoldItemDelegate.IsBound())
		OnSoldItemDelegate.Broadcast(ItemForSale);*/
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

void UTradeComponent::AbortDeal(UItemBase* Item)
{
	
}
