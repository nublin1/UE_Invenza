// Nublin Studio 2026 All Rights Reserved.

#include "UI/Inventory/InventoryListViewRowWidget.h"

#include "Components/Image.h"
#include "Data/Inventory/InventoryBase.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "InputCoreTypes.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/HelpersWidgets/ItemTooltipWidget.h"
#include "UI/Inventory/InventoryViewEntry.h"

void UInventoryListViewRowWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (ViewEntry)
	{
		ViewEntry->OnEntryUpdated.AddUniqueDynamic(this, &UInventoryListViewRowWidget::RefreshRow);
	}
	RefreshRow();
}

void UInventoryListViewRowWidget::NativeDestruct()
{
	ReleaseEntry();
	Super::NativeDestruct();
}

void UInventoryListViewRowWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	ReleaseEntry();
	ViewEntry = Cast<UInventoryViewEntry>(ListItemObject);
	if (ViewEntry)
	{
		ViewEntry->OnEntryUpdated.AddUniqueDynamic(this, &UInventoryListViewRowWidget::RefreshRow);
	}
	RefreshRow();
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
}

void UInventoryListViewRowWidget::NativeOnEntryReleased()
{
	ReleaseEntry();
	IUserObjectListEntry::NativeOnEntryReleased();
}

void UInventoryListViewRowWidget::RefreshRow()
{
	if (ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(ViewEntry ? ViewEntry->Icon.Get() : nullptr);
	}
	if (ItemName)
	{
		ItemName->UpdateText(ViewEntry ? ViewEntry->DisplayName : FText::GetEmpty());
	}
	if (ItemQuantity)
	{
		ItemQuantity->UpdateText(ViewEntry ? FText::AsNumber(ViewEntry->Quantity) : FText::GetEmpty());
	}

	UInventoryBase* Inventory = ViewEntry ? ViewEntry->GetSourceInventory() : nullptr;
	if (!IsValid(Inventory) || !IsValid(ViewEntry->GetItem()) ||
		!Inventory->GetInventorySettings().bShowItemTooltips || !ItemTooltipWidgetClass)
	{
		ReleaseTooltip();
		return;
	}
	if (!ItemTooltip)
	{
		ItemTooltip = CreateWidget<UItemTooltipWidget>(this, ItemTooltipWidgetClass);
		SetToolTip(ItemTooltip);
	}
	if (ItemTooltip)
	{
		ItemTooltip->SetTooltipData(ViewEntry->GetItem(), Inventory);
	}
}

void UInventoryListViewRowWidget::ReleaseTooltip()
{
	if (ItemTooltip && ViewEntry && IsValid(ViewEntry->GetSourceInventory()))
	{
		ViewEntry->GetSourceInventory()->OnTradeContextUpdated.RemoveAll(ItemTooltip);
	}
	SetToolTip(nullptr);
	ItemTooltip = nullptr;
}

void UInventoryListViewRowWidget::ReleaseEntry()
{
	ReleaseTooltip();
	if (ViewEntry)
	{
		ViewEntry->OnEntryUpdated.RemoveAll(this);
	}
	ViewEntry = nullptr;
}

FReply UInventoryListViewRowWidget::NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event)
{
	if (Event.GetEffectingButton() == EKeys::RightMouseButton)
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void UInventoryListViewRowWidget::NativeOnDragDetected(const FGeometry& Geometry, const FPointerEvent& Event,
	UDragDropOperation*& OutOperation)
{
	OutOperation = nullptr;
}

bool UInventoryListViewRowWidget::NativeOnDrop(const FGeometry& Geometry, const FDragDropEvent& Event,
	UDragDropOperation* Operation)
{
	// Consume item drops without applying them or forwarding them to an inventory underneath.
	return IsValid(Cast<UItemDragDropOperation>(Operation));
}
