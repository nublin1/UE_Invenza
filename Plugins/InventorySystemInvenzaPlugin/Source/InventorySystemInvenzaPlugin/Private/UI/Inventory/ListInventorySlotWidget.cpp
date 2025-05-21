// Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/ListInventorySlotWidget.h"

#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/TradeComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/Items/itemBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "UI/HelpersWidgets/ItemTooltipWidget.h"
#include "UI/Inventory/ListInventoryWidget.h"
#include "UI/Item/InventoryItemWidget.h"


void UListInventorySlotWidget::UpdateVisualWithItemInfo(UItemBase* Item)
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

void UListInventorySlotWidget::UpdatePriceText()
{
	if (!CachedEntry || !PriceText)
		return;

	PriceText->SetText(FText::AsNumber(CachedEntry->Item->GetItemRef().ItemTradeData.BasePrice * CachedEntry->Item->GetQuantity()));
	
	UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager || !InventoryManager->GetCurrentInteractInvWidget())
		return;

	if (InventoryManager->GetCurrentInteractInvWidget()->GetInventoryType() != EInventoryType::VendorInventory)
		return;
	
	if (!CachedEntry->ParentInventoryWidget->GetInventoryData().ItemCollectionLink) return;
	auto OwnerAc = InventoryManager->GetCurrentInteractInvWidget()->GetInventoryFromContainerSlot()->GetInventoryData().ItemCollectionLink->GetOwner();
	if (!OwnerAc) return;
	auto TradeComp = OwnerAc->FindComponentByClass<UTradeComponent>();
	if (!TradeComp)
	{
		return;
	}

	if (CachedEntry->ParentInventoryWidget == InventoryManager->GetCurrentInteractInvWidget()->GetInventoryFromContainerSlot())
	{
		FText FullPriceText = FText::AsNumber(TradeComp->GetTotalSellPrice(CachedEntry->Item));
		PriceText->SetText(FullPriceText);
		return;
	}

	else if (InventoryManager->GetMainInventory()->GetInventoryFromContainerSlot() == CachedEntry->ParentInventoryWidget)
	{
		FText FullPriceText = FText::AsNumber(TradeComp->GetTotalBuyPrice(CachedEntry->Item));
		PriceText->SetText(FullPriceText);
	}
}

void UListInventorySlotWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	
	if (UInventoryListEntry* ListEntry = Cast<UInventoryListEntry>(ListItemObject))
	{
		CachedEntry = ListEntry;
		UpdateVisualWithItemInfo(ListEntry->Item);
		UpdatePriceText();
	}
}

FReply UListInventorySlotWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	if (!CachedEntry || !CachedEntry->ParentInventoryWidget)
		return Reply;

	if (CachedEntry->Item && CachedEntry->ParentInventoryWidget->GetInventoryData().ItemTooltipWidget)
	{
		
		CachedEntry->ParentInventoryWidget->GetInventoryData().ItemTooltipWidget->SetTooltipData(CachedEntry->Item, CachedEntry->ParentInventoryWidget);
		 CachedEntry->ParentInventoryWidget->GetInventoryData().ItemTooltipWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else if ( CachedEntry->ParentInventoryWidget->GetInventoryData().ItemTooltipWidget)
		 CachedEntry->ParentInventoryWidget->GetInventoryData().ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);

	return Reply;
}

void UListInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if ( CachedEntry->ParentInventoryWidget->GetInventoryData().ItemTooltipWidget)
		CachedEntry->ParentInventoryWidget->GetInventoryData().ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
}

FReply UListInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	
	UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager ||  !CachedEntry || !CachedEntry->Item || !CachedEntry->ParentInventoryWidget)
		return FReply::Unhandled();
	
	if (InMouseEvent.IsMouseButtonDown(CachedEntry->ParentInventoryWidget->GetUISettings().ItemSelectKey))
	{
		if (InventoryManager->GetInventoryModifierStates().bIsQuickGrabModifierActive)
		{
			FItemMoveData ItemMoveData;
			ItemMoveData.SourceInventory = CachedEntry->ParentInventoryWidget;
			ItemMoveData.SourceItemPivotSlot = this;
			ItemMoveData.SourceItem = CachedEntry->Item;

			InventoryManager->OnQuickTransferItem(ItemMoveData);
				
			return FReply::Handled();
		}

		return FReply::Handled().DetectDrag(TakeWidget(), CachedEntry->ParentInventoryWidget->GetUISettings().ItemSelectKey);
	}

	if (InMouseEvent.IsMouseButtonDown(CachedEntry->ParentInventoryWidget->GetUISettings().ItemUseKey)
		&& CachedEntry->ParentInventoryWidget->GetInventorySettings().bCanUseItems)
	{
		if (CachedEntry->ParentInventoryWidget->HandleTradeModalOpening(CachedEntry->Item))
			return FReply::Handled();
		
		CachedEntry->Item->UseItem();
	}
	
	return FReply::Unhandled();
}

void UListInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager || !CachedEntry || !CachedEntry->ParentInventoryWidget)
		return;

	UInventoryItemWidget* DraggedWidget = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(), InventoryManager->GetUISettings().DraggedWidgetClass);
	if (!DraggedWidget) return;
	DraggedWidget->SetVisibility(ESlateVisibility::Hidden);
	
	UItemDragDropOperation* DragItemDragDropOperation = NewObject<UItemDragDropOperation>();
	DragItemDragDropOperation->DefaultDragVisual = DraggedWidget;
	DragItemDragDropOperation->Pivot = EDragPivot::CenterCenter;
	DragItemDragDropOperation->ItemMoveData.SourceItem = CachedEntry->Item;
	DragItemDragDropOperation->ItemMoveData.SourceInventory = CachedEntry->ParentInventoryWidget;
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


