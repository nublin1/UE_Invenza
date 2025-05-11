// Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/ListInventorySlotWidget.h"

#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/Items/itemBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HelpersWidgets/ItemTooltipWidget.h"
#include "UI/Inventory/ListInventoryWidget.h"
#include "UI/Item/InventoryItemWidget.h"
#include "UI/ModalWidgets/BinaryPromptButtons.h"
#include "UI/ModalWidgets/ModalTradeWidget.h"

void UListInventorySlotWidget::UpdateVisual(UItemBase* Item)
{
	if (ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(Item->GetItemRef().ItemAssetData.Icon);
	}

	if (ItemName)
	{
		if (Item->IsStackable())
		{
			FString ItemNameWithCount = FString::Printf(TEXT("%s (%d)"), 
				*Item->GetItemRef().ItemTextData.Name.ToString(), 
				Item->GetQuantity());

			ItemName->SetText(FText::FromString(ItemNameWithCount));
		}
		else
			ItemName->SetText(Item->GetItemRef().ItemTextData.Name);
	}
}

void UListInventorySlotWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	if (UInventoryListEntry* ListEntry = Cast<UInventoryListEntry>(ListItemObject))
	{
		UpdateVisual(ListEntry->Item);
		ParentInventoryWidget = ListEntry->ParentInventoryWidget;
		LinkedItem = ListEntry->Item;
	}
}

FReply UListInventorySlotWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	if (!ParentInventoryWidget)
		return Reply;

	if (LinkedItem && ParentInventoryWidget->GetInventoryData().ItemTooltipWidget)
	{
		ParentInventoryWidget->GetInventoryData().ItemTooltipWidget->SetTooltipData(LinkedItem);
		 ParentInventoryWidget->GetInventoryData().ItemTooltipWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else if ( ParentInventoryWidget->GetInventoryData().ItemTooltipWidget)
		 ParentInventoryWidget->GetInventoryData().ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);

	return Reply;
}

void UListInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if ( ParentInventoryWidget->GetInventoryData().ItemTooltipWidget)
		ParentInventoryWidget->GetInventoryData().ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
}

FReply UListInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	
	UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager || !ParentInventoryWidget || !LinkedItem)
		return FReply::Unhandled();
	
	if (InMouseEvent.IsMouseButtonDown(ParentInventoryWidget->GetUISettings().ItemSelectKey))
	{
		if (InventoryManager->GetInventoryModifierStates().bIsQuickGrabModifierActive)
		{
			FItemMoveData ItemMoveData;
			ItemMoveData.SourceInventory = ParentInventoryWidget;
			ItemMoveData.SourceItemPivotSlot = this;
			ItemMoveData.SourceItem = LinkedItem;

			InventoryManager->OnQuickTransferItem(ItemMoveData);
				
			return FReply::Handled();
		}

		return FReply::Handled().DetectDrag(TakeWidget(), ParentInventoryWidget->GetUISettings().ItemSelectKey);
	}

	if (InMouseEvent.IsMouseButtonDown(ParentInventoryWidget->GetUISettings().ItemUseKey)
		&& ParentInventoryWidget->GetInventorySettings().bCanUseItems)
	{
		if (ParentInventoryWidget->HandleTradeModalOpening(LinkedItem))
			return FReply::Handled();
		
		LinkedItem->UseItem();
	}
	
	return FReply::Unhandled();
}

void UListInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager || !ParentInventoryWidget)
		return;

	UInventoryItemWidget* DraggedWidget = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(), InventoryManager->GetUISettings().DraggedWidgetClass);
	if (!DraggedWidget) return;
	DraggedWidget->SetVisibility(ESlateVisibility::Hidden);
	
	UItemDragDropOperation* DragItemDragDropOperation = NewObject<UItemDragDropOperation>();
	DragItemDragDropOperation->DefaultDragVisual = DraggedWidget;
	DragItemDragDropOperation->Pivot = EDragPivot::CenterCenter;

	DragItemDragDropOperation->ItemMoveData.SourceItem = LinkedItem;
	DragItemDragDropOperation->ItemMoveData.SourceInventory = ParentInventoryWidget;
	DragItemDragDropOperation->ItemMoveData.SourceItemPivotSlot = this;
	
	auto ShowDragVisual = [DraggedWidget]()
	{
		DraggedWidget->SetVisibility(ESlateVisibility::Visible);
	};
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	const FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda(ShowDragVisual);
	FTimerHandle TimerHandle;
	TimerManager.SetTimer(TimerHandle, TimerDelegate, 0.125f, false);

	OutOperation = DragItemDragDropOperation;
}
