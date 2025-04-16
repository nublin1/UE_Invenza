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

	SellerInvWidget->GetInventoryFromContainerSlot()->OnAddItemDelegate.AddDynamic(this, &UTradeComponent::TryBuyItem);
}

void UTradeComponent::CloseTradeMenu()
{
	SellerInvWidget->GetInventoryFromContainerSlot()->OnAddItemDelegate.RemoveDynamic(
		this, &UTradeComponent::TryBuyItem);
	SellerItemCollection = nullptr;
	BuyerItemCollection = nullptr;
	ManagerActor = nullptr;
}

void UTradeComponent::TryBuyItem(FItemMapping ItemSlots, UItemBase* BuyItem)
{
	if (bSellOnly)
		AbortDeal(BuyItem);
		
	if (BuyItem->GetItemRef().ItemCategory == EItemCategory::Money)
		AbortDeal(BuyItem);

	TArray<UItemBase*> SellerMoneyItems = SellerItemCollection->GetAllItemsByCategory(EItemCategory::Money);
	if (SellerMoneyItems.Num() <= 0)
	{
		AbortDeal(BuyItem);
		return;
	}

	float AvailableMoney = 0.0f;
	for (UItemBase* MoneyItem : SellerMoneyItems)
	{
		if (MoneyItem)
		{
			AvailableMoney += MoneyItem->GetItemRef().ItemNumeraticData.BasePrice * MoneyItem->GetQuantity();
		}
	}

	auto FullPrice = BuyItem->GetItemRef().ItemNumeraticData.BasePrice * BuyPriceFactor * BuyItem->GetQuantity();
	if (AvailableMoney > FullPrice)
	{
		//BuyItem(Item);
		float AccumulatedValue = 0.0f;
		for (UItemBase* MoneyItem : SellerMoneyItems)
		{
			int32 Quantity = MoneyItem->GetQuantity();
			float UnitValue = MoneyItem->GetItemRef().ItemNumeraticData.BasePrice;
			int32 NeededQuantity = FMath::CeilToInt((FullPrice - AccumulatedValue) / UnitValue);
			int32 RemoveQuantity = FMath::Min(Quantity, NeededQuantity);

			if (RemoveQuantity > 0)
			{
				AccumulatedValue += RemoveQuantity * UnitValue;

				SellerInvWidget->GetInventoryFromContainerSlot()->HandleRemoveItem(MoneyItem, RemoveQuantity);
				TObjectPtr<UItemBase> Item = NewObject<UItemBase>();
				Item->SetQuantity(RemoveQuantity);
				Item->SetItemRef(MoneyItem->GetItemRef());

				FItemMoveData MoneyMoveData;
				MoneyMoveData.SourceItem = Item;
				MoneyMoveData.TargetInventory = BuyerInvWidget->GetInventoryFromContainerSlot();
				ManagerActor->ItemTransferRequest(MoneyMoveData);
			}
		}
	}
	else
		AbortDeal(BuyItem);
}

void UTradeComponent::BuyItem(UItemBase* Item)
{
}

void UTradeComponent::Selltem(UItemBase* Item)
{
}

void UTradeComponent::AbortDeal(UItemBase* Item)
{
	SellerInvWidget->GetInventoryFromContainerSlot()->HandleRemoveItem(Item, Item->GetQuantity());

	FItemMoveData ItemMoveData;
	ItemMoveData.SourceItem = Item;
	ItemMoveData.TargetInventory = BuyerInvWidget->GetInventoryFromContainerSlot();
	ManagerActor->ItemTransferRequest(ItemMoveData);
}
