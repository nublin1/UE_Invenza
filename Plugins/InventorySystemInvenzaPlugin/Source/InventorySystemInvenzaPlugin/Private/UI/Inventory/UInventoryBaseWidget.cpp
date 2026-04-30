//  Nublin Studio 2026 All Rights Reserved.


#include "UI/Inventory/UInventoryBaseWidget.h"

#include "Data/Inventory/InventoryBase.h"
#include "UI/HelpersWidgets/ItemTooltipWidget.h"


UUInventoryBaseWidget::UUInventoryBaseWidget()
{
}

void UUInventoryBaseWidget::InitializeInventoryWidgetWithSettings()
{
}

void UUInventoryBaseWidget::CreateTooltipWidget()
{
	if (!InventoryRef)
		return;

	auto InvSettings = InventoryRef->GetInventorySettings();
	
	if (!InvSettings.bShowItemTooltips || !UISettings.ItemTooltipWidgetClass)
		return;

	if (ItemTooltipWidget)
	{
		ItemTooltipWidget->RemoveFromParent();
		SetToolTip(nullptr);
		ItemTooltipWidget = nullptr;
	}
	
	ItemTooltipWidget = CreateWidget<UItemTooltipWidget>(this, UISettings.ItemTooltipWidgetClass);
	SetToolTip(ItemTooltipWidget);
	ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
}
