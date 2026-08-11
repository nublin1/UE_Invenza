//  Nublin Studio 2026 All Rights Reserved.

#include "UI/HelpersWidgets/ItemTooltipWidget.h"
#include "Data/Items/itemBase.h"
#include "Components/TextBlock.h"
#include "Data/ItemDataStructures.h"
#include "Data/Inventory/InventoryBase.h"
#include "UI/Core/LabelBaseText.h"
#include "Utility/InterfaceUtils.h"


UItemTooltipWidget::UItemTooltipWidget()
{
}

void UItemTooltipWidget::SetTooltipData(UObject* InItem, UInventoryBase* InInventory)
{
	if (!InItem || !ItemName || !ItemType || !ItemDescription)
		return;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(InItem, TEXT("SetTooltipData")))
		return;

	Item = InItem;

	if (InInventory)
	{
		Inventory = InInventory;
		Inventory->OnTradeContextUpdated.AddUniqueDynamic(this, &UItemTooltipWidget::UpdatePrice);
	}

	const FItemMetaData ItemData = IObjectDataProvider::Execute_GetItemRef(Item);

	ItemName->UpdateText(ItemData.ItemTextData.DisplayName);
	ItemType->UpdateText(FText::FromString(IObjectDataProvider::Execute_CategoryToString(Item)));
	ItemDescription->UpdateText(ItemData.ItemTextData.ItemDescription);

	const float StackWeight = IObjectDataProvider::Execute_GetItemStackWeight(Item);
	StackWeightValue->UpdateText(FText::AsNumber(StackWeight));

	const FString WeightInfo = FString::Printf(TEXT("Weight: %.2f"), StackWeight);
	StackWeightValue->UpdateText(FText::FromString(WeightInfo));

	if (IObjectDataProvider::Execute_IsStackable(Item))
	{
		const FString StackInfo = FString::FromInt(ItemData.ItemNumeraticData.MaxStackSizeInCharacter);

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
	if (!PriceText || !PriceValue || !Item)
		return;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("UpdatePrice")))
		return;

	const FItemMetaData ItemData = IObjectDataProvider::Execute_GetItemRef(Item);
	const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(Item);

	float BasePrice = ItemData.ItemTradeData.BasePrice * Quantity;

	if (!Inventory)
	{
		PriceValue->UpdateText(FText::AsNumber(BasePrice));
		return;
	}

	float PriceMod = 1.0f;
	const FTradeContext TradeContext = Inventory->GetTradeContext();

	if (TradeContext.Buyer != nullptr && TradeContext.Vendor != nullptr)
	{
		const bool bIsVendor = (Inventory->GetInventoryOwnerActor() == TradeContext.Vendor);

		PriceMod = bIsVendor
			? TradeContext.TradeSettings.SellPriceFactor
			: TradeContext.TradeSettings.BuyPriceFactor;
	}

	const int32 FullPrice = FMath::FloorToInt(PriceMod * BasePrice);
	PriceValue->UpdateText(FText::AsNumber(FullPrice));
}
