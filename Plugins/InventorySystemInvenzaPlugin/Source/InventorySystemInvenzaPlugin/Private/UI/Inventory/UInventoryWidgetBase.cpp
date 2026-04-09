//  Nublin Studio 2026 All Rights Reserved.


#include "UI/Inventory/UInventoryWidgetBase.h"

#include "Data/Inventory/InventoryBase.h"
#include "UI/HelpersWidgets/ItemTooltipWidget.h"


UUInventoryWidgetBase::UUInventoryWidgetBase()
{
}

void UUInventoryWidgetBase::CreateTooltipWidget()
{
	if (!InventoryRef)
		return;

	auto InvSettings = InventoryRef->GetInventorySettings();
	
	if (!InvSettings.bShowItemTooltips || !UISettings.ItemTooltipWidgetClass)
		return;
	
	ItemTooltipWidget = CreateWidget<UItemTooltipWidget>(this, UISettings.ItemTooltipWidgetClass);
	SetToolTip(ItemTooltipWidget);
	ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
}
