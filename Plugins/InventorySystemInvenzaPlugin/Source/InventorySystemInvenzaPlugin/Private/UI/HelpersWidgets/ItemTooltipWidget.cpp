//  Nublin Studio 2026 All Rights Reserved.

#include "UI/HelpersWidgets/ItemTooltipWidget.h"
#include "Data/Items/itemBase.h"
#include "Components/TextBlock.h"
#include "Data/ItemDataStructures.h"
#include "Data/Inventory/InventoryBase.h"
#include "UI/Core/LabelBaseText.h"


UItemTooltipWidget::UItemTooltipWidget()
{
}

void UItemTooltipWidget::SetTooltipData(UItemBase* InItem, UInventoryBase* InInventory)
{
	if (!InItem || !ItemName || !ItemType || !ItemDescription)
		return;

	Item = InItem;
	if (InInventory)
	{
		Inventory = InInventory;	
		Inventory->OnTradeContextUpdated.AddUniqueDynamic(this, &UItemTooltipWidget::UpdatePrice);
	}
	
	auto ItemData = Item->GetItemRef();
	
	ItemName->UpdateText(ItemData.ItemTextData.DisplayName);
	ItemType->UpdateText(FText::FromString(Item->CategoryToString()));
	ItemDescription->UpdateText(ItemData.ItemTextData.ItemDescription);
	StackWeightValue->UpdateText(FText::AsNumber(Item->Execute_GetItemStackWeight(Item)));

	const FString WeightInfo = {"Weight: " + FString::SanitizeFloat(Item->Execute_GetItemStackWeight(Item))};
	StackWeightValue->UpdateText(FText::FromString(WeightInfo));

	if (Item->Execute_IsStackable(Item))
	{
		const FString StackInfo = {FString::FromInt(ItemData.ItemNumeraticData.MaxStackSizeInCharacter)};
		MaxStackSize->UpdateText(FText::FromString("Max Stack Size: "));
		StackSizeValue->UpdateText(FText::FromString(StackInfo));
		MaxStackSize->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		MaxStackSize->SetVisibility(ESlateVisibility::Collapsed);
		StackSizeValue->UpdateText(FText::FromString("Unstackable"));
	}

	UpdatePrice();
}

void UItemTooltipWidget::UpdatePrice()
{
	if (!PriceText || !PriceValue)
		return;
	
	float BasePrice = Item->GetItemRef().ItemTradeData.BasePrice * Item->GetQuantity();

	if (!Inventory)
	{
		PriceValue->UpdateText(FText::AsNumber(Item->GetItemRef().ItemTradeData.BasePrice * Item->GetQuantity()));
	}
	
	float PriceMod = 1.0f;
	auto TradeContext = Inventory->GetTradeContext();
	if (TradeContext.Buyer != nullptr && TradeContext.Vendor !=nullptr)
	{
		bool bIsVendor = false;
		Inventory->GetInventoryOwnerActor() == TradeContext.Vendor ? bIsVendor = true : bIsVendor = false;
		bIsVendor ? PriceMod = TradeContext.TradeSettings.SellPriceFactor : PriceMod = TradeContext.TradeSettings.BuyPriceFactor;
	}

	auto FullPrice = FMath::FloorToInt(PriceMod * BasePrice);

	PriceValue->UpdateText(FText::AsNumber(FullPrice));
}
