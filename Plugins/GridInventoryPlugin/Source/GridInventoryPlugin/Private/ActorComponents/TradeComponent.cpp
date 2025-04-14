//  Nublin Studio 2025 All Rights Reserved.


#include "ActorComponents/TradeComponent.h"

#include "ActorComponents/ItemCollection.h"


void UTradeComponent::OpenTradeMenu(AActor* Seller, AActor* Buyer)
{
	auto SellerItemCollection = Seller->FindComponentByClass(UItemCollection::StaticClass());
	auto BuyerItemCollection = Buyer->FindComponentByClass(UItemCollection::StaticClass());
	if (!SellerItemCollection || !BuyerItemCollection)
	{
		return;
	}

	
}

void UTradeComponent::BuyItem(UItemBase* Item)
{
}

void UTradeComponent::Selltem(UItemBase* Item)
{
}
