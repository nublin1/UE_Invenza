// Nublin Studio 2026 All Rights Reserved.


#include "UI/Inventory/ListInventorySlotWidget.h"

#include "ActorComponents/ItemCollection.h"

#include "ActorComponents/UIInventoryManager.h"
#include "Data/Items/itemBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"      
#include "Data/Inventory/InventoryBase.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "UI/HelpersWidgets/ItemTooltipWidget.h"
#include "UI/Inventory/ListInventoryWidget.h"
#include "UI/Item/InventoryItemWidget.h"
#include "Utility/InterfaceUtils.h"
#include "Utility/InvenzayUtility.h"


UListInventorySlotWidget::UListInventorySlotWidget()
{
}

void UListInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CachedEntry)
	{
		auto InvRef = CachedEntry->ParentInventoryWidget->GetInventoryRef();
		InvRef->OnTradeContextUpdated.AddDynamic(this, &UListInventorySlotWidget::UpdatePriceText);
	}
	
	if (!SlotData)
	{
		UInventorySlotData* NewSlotData = NewObject<UInventorySlotData>();
		SlotData = NewSlotData;
	}
}

void UListInventorySlotWidget::UpdateVisualWithItemInfo(UObject* Item)
{
	if (!Item || !UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("UpdateVisualWithItemInfo")))
		return;

	if (ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(
			IObjectDataProvider::Execute_GetItemRef(Item).ItemAssetData.Icon
		);
	}

	if (ItemName)
	{
		if (IObjectDataProvider::Execute_IsStackable(Item))
		{
			FString ItemNameWithCount = FString::Printf(
				TEXT("%s (%d)"),
				*IObjectDataProvider::Execute_GetItemRef(Item).ItemTextData.DisplayName.ToString(),
				IObjectDataProvider::Execute_GetQuantity(Item)
			);

			ItemName->SetText(FText::FromString(ItemNameWithCount));
		}
		else
		{
			ItemName->SetText(
				IObjectDataProvider::Execute_GetItemRef(Item).ItemTextData.DisplayName
			);
		}
	}
}

void UListInventorySlotWidget::UpdatePriceText()
{
	if (!CachedEntry || !PriceText)
		return;

	UObject* Item = CachedEntry->Item;
	if (!Item || !UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("UpdatePriceText")))
		return;

	float BasePrice =
		IObjectDataProvider::Execute_GetItemRef(Item).ItemTradeData.BasePrice *
		IObjectDataProvider::Execute_GetQuantity(Item);

	float PriceMod = 1.0f;

	auto InvRef = CachedEntry->ParentInventoryWidget->GetInventoryRef();
	auto TradeContext = InvRef->GetTradeContext();

	if (TradeContext.Buyer != nullptr && TradeContext.Vendor != nullptr)
	{
		bool bIsVendor = (InvRef->GetInventoryOwnerActor() == TradeContext.Vendor);
		PriceMod = bIsVendor
			? TradeContext.TradeSettings.SellPriceFactor
			: TradeContext.TradeSettings.BuyPriceFactor;
	}

	auto FullPrice = FMath::FloorToInt(PriceMod * BasePrice);
	PriceText->SetText(FText::AsNumber(FullPrice));
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

	if (CachedEntry->Item && CachedEntry->ParentInventoryWidget->GetItemTooltipWidget())
	{
		
		CachedEntry->ParentInventoryWidget->GetItemTooltipWidget()->SetTooltipData(CachedEntry->Item, CachedEntry->ParentInventoryWidget->GetInventoryRef());
		 CachedEntry->ParentInventoryWidget->GetItemTooltipWidget()->SetVisibility(ESlateVisibility::Visible);
	}
	else if ( CachedEntry->ParentInventoryWidget->GetItemTooltipWidget())
		 CachedEntry->ParentInventoryWidget->GetItemTooltipWidget()->SetVisibility(ESlateVisibility::Collapsed);

	return Reply;
}

void UListInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if ( CachedEntry->ParentInventoryWidget->GetItemTooltipWidget())
		CachedEntry->ParentInventoryWidget->GetItemTooltipWidget()->SetVisibility(ESlateVisibility::Collapsed);
}

FReply UListInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

    if (!CachedEntry || !CachedEntry->Item || !CachedEntry->ParentInventoryWidget)
        return FReply::Unhandled();

    UObject* Item = CachedEntry->Item;
    if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("NativeOnMouseButtonDown")))
        return FReply::Unhandled();

    auto Handler = UInvenzayUtility::FindInventoryHandler(GetOwningPlayerPawn());
    if (!Handler)
        return Reply;

    FInventoryModifierState Modifiers =
        IInventoryInteractionHandler::Execute_GetInventoryModifierStates(Handler->_getUObject());

    if (InMouseEvent.IsMouseButtonDown(CachedEntry->ParentInventoryWidget->GetUISettings().ItemSelectKey))
    {
        FItemMoveData ItemMoveData;
        ItemMoveData.SourceInventory = CachedEntry->ParentInventoryWidget->GetInventoryRef();

        if (this->GetSlotData())
            ItemMoveData.SourceItemPivotSlotCoordinate = this->GetSlotData()->InventorySlotInfo.CellPosition;

        ItemMoveData.SourceItem = Item;

        if (Modifiers.bIsQuickGrabModifierActive)
        {
            Handler->Execute_OnQuickTransferItem(Handler.GetObject(), ItemMoveData);
            return FReply::Handled();
        }

        if (Modifiers.bIsGrabAllSameModifierActive)
        {
            Handler->Execute_OnQuickTransferAllSameItems(Handler.GetObject(), ItemMoveData);
            return FReply::Handled();
        }

        return FReply::Handled().DetectDrag(TakeWidget(), CachedEntry->ParentInventoryWidget->GetUISettings().ItemSelectKey);
    }

    if (InMouseEvent.IsMouseButtonDown(CachedEntry->ParentInventoryWidget->GetUISettings().ItemUseKey))
    {
        if (CachedEntry->ParentInventoryWidget->GetInventoryRef()->GetInventorySettings().bAllowItemUsage)
            IObjectDataProvider::Execute_UseItem(Item);
    }

    if (InMouseEvent.GetEffectingButton() == CachedEntry->ParentInventoryWidget->GetUISettings().ItemMenuKey)
    {
        Handler->Execute_ItemContextMenuRequest(
            Handler.GetObject(),
            CachedEntry->ParentInventoryWidget->GetInventoryRef()->GetInventoryContainerID(),
            CachedEntry->SlotGuid,
            Item
        );

        return FReply::Handled();
    }

    return FReply::Unhandled();
}

FReply UListInventorySlotWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	UIInventoryManager* InventoryManager =
		GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();

	if (!InventoryManager || !CachedEntry || !CachedEntry->Item || !CachedEntry->ParentInventoryWidget)
		return FReply::Unhandled();

	UObject* Item = CachedEntry->Item;
	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("NativeOnMouseButtonDoubleClick")))
		return FReply::Unhandled();

	auto Inv = CachedEntry->ParentInventoryWidget->GetInventoryRef();
	if (!Inv)
		return FReply::Unhandled();

	auto UISettings = CachedEntry->ParentInventoryWidget->GetUISettings();

	if (InMouseEvent.GetEffectingButton() == UISettings.ItemSelectKey)
	{
		int32 Quantity = IObjectDataProvider::Execute_GetQuantity(Item);
		Inv->RequestSplitStack(Item, Quantity / 2);
	}

	return FReply::Unhandled();
}

void UListInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                                    UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    if (!CachedEntry || !CachedEntry->ParentInventoryWidget)
        return;

    UObject* Item = CachedEntry->Item;
    if (!Item || !UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("NativeOnDragDetected")))
        return;

    auto Inv = CachedEntry->ParentInventoryWidget->GetInventoryRef();
    if (!Inv)
        return;

    UInventoryItemWidget* DraggedWidget =
        CreateWidget<UInventoryItemWidget>(
            GetOwningPlayer(),
            CachedEntry->ParentInventoryWidget->GetUISettings().DraggedWidgetClass
        );

    if (!DraggedWidget)
        return;

    DraggedWidget->SetVisibility(ESlateVisibility::Hidden);
    DraggedWidget->AddToPlayerScreen(1);
    DraggedWidget->SetPositionInViewport(FVector2D(-10000, -10000));

    auto InitialItemOrientation =
        IObjectDataProvider::Execute_GetInitialItemOrientation(Item);

    FVector2D WidgetSlotSize =
        CachedEntry->ParentInventoryWidget->GetUISettings().DragWidgetSlotSize;

    auto TotalSize =
        UInvenzayUtility::CalculateItemVisualSize(
            Item,
            InitialItemOrientation,
            WidgetSlotSize,
            FMargin(0),
            false
        );

    DraggedWidget->UpdateItemVisual(
        Item,
        InitialItemOrientation,
        TotalSize,
        FVector2D(0.0f),
        true
    );

    UItemDragDropOperation* DragItemDragDropOperation =
        NewObject<UItemDragDropOperation>();

    DragItemDragDropOperation->DefaultDragVisual = DraggedWidget;
    DragItemDragDropOperation->Pivot = EDragPivot::CenterCenter;

    DragItemDragDropOperation->ItemMoveData.SourceItem = Item;
    DragItemDragDropOperation->ItemMoveData.SourceInventory = Inv;
    DragItemDragDropOperation->ItemMoveData.SourceItemPivotSlotCoordinate =
        this->GetSlotData()->InventorySlotInfo.CellPosition;

    DragItemDragDropOperation->ItemMoveData.SavedOrientation = InitialItemOrientation;
    DragItemDragDropOperation->ItemMoveData.TargetOrientation = InitialItemOrientation;

    DragItemDragDropOperation->SetUISettings(
        CachedEntry->ParentInventoryWidget->GetUISettings()
    );

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


