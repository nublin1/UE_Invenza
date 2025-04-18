// Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/ListInventorySlotWidget.h"

#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Items/itemBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Inventory/ListInventoryWidget.h"
#include "UI/Item/InventoryItemWidget.h"
#include "World/AUIManagerActor.h"

bool UListInventorySlotWidget::ExecuteItemChecks(EInventoryCheckType CheckType, UItemBase* Item)
{
	if (ParentInventoryWidget->GetInventoryData().Checks.IsEmpty())
		return true;
	
	for (const FInventoryCheck& Check : ParentInventoryWidget->GetInventoryData().Checks)
	{
		if (Check.CheckType == CheckType)
		{
			if (!Check.CheckFunction(Item))
			{
				return false;
			}
		}
	}
	return true;
}

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

FReply UListInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	
	AUIManagerActor* ManagerActor = Cast<AUIManagerActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AUIManagerActor::StaticClass()));
	if (!ManagerActor || !ParentInventoryWidget || !LinkedItem)
		return FReply::Unhandled();
	
	if (InMouseEvent.IsMouseButtonDown(ParentInventoryWidget->GetUISettings().ItemSelectKey))
	{
		if (ManagerActor->GetInventoryModifierStates().bIsQuickGrabModifierActive
			&& ExecuteItemChecks(EInventoryCheckType::PreTransfer, LinkedItem))
		{
			FItemMoveData ItemMoveData;
			ItemMoveData.SourceInventory = ParentInventoryWidget;
			ItemMoveData.SourceItemPivotSlot = this;
			ItemMoveData.SourceItem = LinkedItem;

			ManagerActor->OnQuickTransferItem(ItemMoveData);
				
			return FReply::Unhandled();
		}

		if (ExecuteItemChecks(EInventoryCheckType::PreTransfer, LinkedItem))
			return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	if (InMouseEvent.IsMouseButtonDown(ParentInventoryWidget->GetUISettings().ItemUseKey))
	{
		
	}

	return FReply::Unhandled();
}

void UListInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	AUIManagerActor* ManagerActor = Cast<AUIManagerActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AUIManagerActor::StaticClass()));
	if (!ManagerActor || !ParentInventoryWidget)
		return;

	UInventoryItemWidget* DraggedWidget = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(), ManagerActor->GetUISettings().DraggedWidgetClass);
	if (!DraggedWidget) return;
	DraggedWidget->SetVisibility(ESlateVisibility::Visible);
	
	UItemDragDropOperation* DragItemDragDropOperation = NewObject<UItemDragDropOperation>();
	DragItemDragDropOperation->DefaultDragVisual = DraggedWidget;
	DragItemDragDropOperation->Pivot = EDragPivot::CenterCenter;

	DragItemDragDropOperation->ItemMoveData.SourceItem = LinkedItem;
	DragItemDragDropOperation->ItemMoveData.SourceInventory = ParentInventoryWidget;
	DragItemDragDropOperation->ItemMoveData.SourceItemPivotSlot = this;

	OutOperation = DragItemDragDropOperation;
}
