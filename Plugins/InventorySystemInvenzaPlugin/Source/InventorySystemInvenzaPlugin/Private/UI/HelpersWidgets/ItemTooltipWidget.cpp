//  Nublin Studio 2026 All Rights Reserved.

#include "UI/HelpersWidgets/ItemTooltipWidget.h"
#include "Data/Items/itemBase.h"
#include "Components/TextBlock.h"
#include "Data/ItemDataStructures.h"
#include "UI/Core/LabelBaseText.h"


UItemTooltipWidget::UItemTooltipWidget()
{
}

void UItemTooltipWidget::SetTooltipData(UItemBase* Item, UInventoryBase* Inventory)
{
	if (!Item || !ItemName || !ItemType || !ItemDescription)
		return;
	
	auto ItemData = Item->GetItemRef();
	
	ItemName->UpdateText(ItemData.ItemTextData.DisplayName);
	ItemType->UpdateText(FText::FromString(Item->CategoryToString()));
	ItemDescription->UpdateText(ItemData.ItemTextData.ItemDescription);
	StackWeightValue->UpdateText(FText::AsNumber(Item->GetItemStackWeight()));

	const FString WeightInfo = {"Weight: " + FString::SanitizeFloat(Item->GetItemStackWeight())};
	StackWeightValue->UpdateText(FText::FromString(WeightInfo));

	if (Item->IsStackable())
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

	if (PriceText && PriceValue)
	{
		PriceValue->UpdateText(FText::AsNumber(Item->GetItemRef().ItemTradeData.BasePrice * Item->GetQuantity()));
	}
}
