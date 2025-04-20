//  Nublin Studio 2025 All Rights Reserved.

#include "UI/HelpersWidgets/ItemTooltipWidget.h"

#include "ActorComponents/Items/itemBase.h"
#include "Components/TextBlock.h"
#include "Data/ItemDataStructures.h"

UItemTooltipWidget::UItemTooltipWidget()
{
}

void UItemTooltipWidget::SetTooltipData(UItemBase* Item)
{
	if (!Item || !ItemName || !ItemType || !ItemDescription)
		return;
	
	auto ItemData = Item->GetItemRef();
	
	ItemName->SetText(ItemData.ItemTextData.Name);
	ItemType->SetText(Item->GetItemCategoryString());
	ItemDescription->SetText(ItemData.ItemTextData.ItemDescription);
	StackWeightValue->SetText(FText::AsNumber(Item->GetItemStackWeight()));

	const FString WeightInfo = {"Weight: " + FString::SanitizeFloat(Item->GetItemStackWeight())};
	StackWeightValue->SetText(FText::FromString(WeightInfo));

	if (Item->IsStackable())
	{
		const FString StackInfo = {FString::FromInt(ItemData.ItemNumeraticData.MaxStackSize)};
		MaxStackSize->SetText(FText::FromString("Max Stack Size: "));
		StackSizeValue->SetText(FText::FromString(StackInfo));
		MaxStackSize->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		MaxStackSize->SetVisibility(ESlateVisibility::Collapsed);
		StackSizeValue->SetText(FText::FromString("Unstackable"));
	}
}
