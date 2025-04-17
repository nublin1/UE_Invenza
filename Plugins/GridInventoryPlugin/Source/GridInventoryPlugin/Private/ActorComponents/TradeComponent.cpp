//  Nublin Studio 2025 All Rights Reserved.


#include "ActorComponents/TradeComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "UI/Inventory/UInventoryWidgetBase.h"
#include "World/AUIManagerActor.h"


void UTradeComponent::OpenTradeMenu(AActor* Seller, AActor* Buyer, AUIManagerActor* Actor)
{
	SellerItemCollection = Cast<UItemCollection>(Seller->FindComponentByClass(UItemCollection::StaticClass()));
	BuyerItemCollection = Cast<UItemCollection>(Buyer->FindComponentByClass(UItemCollection::StaticClass()));
	ManagerActor = Actor;
	if (!SellerItemCollection || !BuyerItemCollection || !ManagerActor)
	{
		return;
	}

	SellerInvWidget = ManagerActor->GetCoreHUDWidget()->GetVendorInvWidget();
	BuyerInvWidget = ManagerActor->GetMainInventory();
	if (!SellerInvWidget || !BuyerInvWidget)
		return;

	SellerInvWidget->GetInventoryFromContainerSlot()->SetItemCollection(SellerItemCollection);
	if (SellerItemCollection->InitItems.Num() > 0)
	{
		for (const auto& Item : SellerItemCollection->InitItems)
		{
			FItemMoveData ItemMoveData;
			ItemMoveData.SourceItem = UItemBase::CreateFromDataTable(SellerItemCollection->ItemDataTable, Item.ItemName,
			                                                         Item.ItemCount);
			if (ItemMoveData.SourceItem)
			{
				SellerInvWidget->GetInventoryFromContainerSlot()->HandleAddItem(ItemMoveData);
			}
		}

		SellerItemCollection->InitItems.Empty();
	}
	
	SellerInvWidget->GetInventoryFromContainerSlot()->AddCheck(
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
		});
}

void UTradeComponent::CloseTradeMenu()
{
	SellerInvWidget->GetInventoryFromContainerSlot()->RemoveCheck(EInventoryCheckType::PreTransfer, TEXT("TrySellItemCheck"));
	SellerInvWidget->GetInventoryFromContainerSlot()->RemoveCheck(EInventoryCheckType::PreAdd, TEXT("TryBuyItemCheck"));
	
	SellerItemCollection = nullptr;
	BuyerItemCollection = nullptr;
	ManagerActor = nullptr;
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

FMoneyCalculationResult UTradeComponent::AccumulatePayment(UInvBaseContainerWidget* ContainerWidget, const float FullPrice)
{
	FMoneyCalculationResult Result;
	Result.AvailableMoney  = CalculateAvailableMoney(ContainerWidget->GetInventoryFromContainerSlot()->GetItemCollection());
	
	TArray<UItemBase*> SellerMoneyItems = ContainerWidget->GetInventoryFromContainerSlot()->GetItemCollection()->GetAllItemsByCategory(EItemCategory::Money);

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
	auto Result =  AccumulatePayment(SellerInvWidget, FullPrice);

	if (Result.bHasEnough)
	{
		BuyItem(ItemToBuy, Result);
		return true;
	}

	if (OnFaildToBuyItemDelegate.IsBound())
		OnFaildToBuyItemDelegate.Broadcast(ItemToBuy);
	
	return false;
}

void UTradeComponent::BuyItem(UItemBase* ItemToBuy, FMoneyCalculationResult Result)
{
	TArray<UItemBase*> SellerMoneyItems = SellerItemCollection->GetAllItemsByCategory(EItemCategory::Money);
	auto MoneyItem = SellerMoneyItems[0]->DuplicateItem();
	MoneyItem->SetQuantity(Result.AccumulatedRequiredValue);
	SellerInvWidget->GetInventoryFromContainerSlot()->HandleRemoveItem(SellerMoneyItems[0], Result.AccumulatedRequiredValue);
	
	FItemMoveData MoneyMoveData;
	MoneyMoveData.SourceItem = MoneyItem;
	MoneyMoveData.TargetInventory = BuyerInvWidget->GetInventoryFromContainerSlot();
	ManagerActor->ItemTransferRequest(MoneyMoveData);

	if (OnBoughtItemDelegate.IsBound())
		OnBoughtItemDelegate.Broadcast(ItemToBuy);
}

bool UTradeComponent::TrySellItem(UItemBase* ItemForSale)
{
	if (ItemForSale->GetItemRef().ItemCategory == EItemCategory::Money)
		return false;
	
	auto FullPrice = ItemForSale->GetItemRef().ItemNumeraticData.BasePrice * BuyPriceFactor * ItemForSale->GetQuantity();
	auto Result =  AccumulatePayment(BuyerInvWidget, FullPrice);
	
	if (Result.bHasEnough)
	{
		Selltem(ItemForSale, Result);
		return true;
	}

	if (OnFaildToSellItemDelegate.IsBound())
		OnFaildToSellItemDelegate.Broadcast(ItemForSale);
	
	return false;
}

void UTradeComponent::Selltem(UItemBase* ItemForSale, FMoneyCalculationResult Result)
{
	TArray<UItemBase*> BuyerMoneyItems = BuyerItemCollection->GetAllItemsByCategory(EItemCategory::Money);
	auto MoneyItem = BuyerMoneyItems[0]->DuplicateItem();
	MoneyItem->SetQuantity(Result.AccumulatedRequiredValue);
	BuyerInvWidget->GetInventoryFromContainerSlot()->HandleRemoveItem(BuyerMoneyItems[0], Result.AccumulatedRequiredValue);
	
	FItemMoveData MoneyMoveData;
	MoneyMoveData.SourceItem = MoneyItem;
	MoneyMoveData.TargetInventory = SellerInvWidget->GetInventoryFromContainerSlot();
	ManagerActor->ItemTransferRequest(MoneyMoveData);
	
	if (OnSoldItemDelegate.IsBound())
		OnSoldItemDelegate.Broadcast(ItemForSale);
}

void UTradeComponent::AbortDeal(UItemBase* Item)
{
	
}
