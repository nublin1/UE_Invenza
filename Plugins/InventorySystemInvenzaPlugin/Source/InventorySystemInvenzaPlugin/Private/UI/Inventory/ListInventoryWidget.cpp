//  Nublin Studio 2025 All Rights Reserved.

#include "UI/Inventory/ListInventoryWidget.h"
#include "TimerManager.h"

#include "ActorComponents/InventoryCollectionComponent.h"
#include "Data/Items/itemBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/ListView.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/ListInventory/ListInventory.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/ItemFiltersPanel/FiltersPanel.h"
#include "UI/Inventory/ListInventorySlotWidget.h"
#include "UI/Item/InventoryItemWidget.h"
#include "Utility/InterfaceUtils.h"

class UListInventorySlotWidget;

UListInventoryWidget::UListInventoryWidget()
{
}

void UListInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemFiltersPanel && ItemFiltersPanel->GetSearchText())
	{
		ItemFiltersPanel->GetSearchText()->OnTextChanged.AddUniqueDynamic(this,
																		  &UListInventoryWidget::SearchTextChanged);
	}

	if (ItemFiltersPanel)
	{
		for (auto FilterButton : ItemFiltersPanel->GetFilteredCategores())
		{
			FilterButton->OnButtonClicked.AddUniqueDynamic(this, &UListInventoryWidget::OnFilterStatusChanged);
		}

		if (ItemFiltersPanel->GetClearFiltersButton())
		{
			ItemFiltersPanel->GetClearFiltersButton()->MainButton->OnClicked.AddUniqueDynamic(
				this, &UListInventoryWidget::ClearFilters);
		}
	}
}

void UListInventoryWidget::InitializeInventoryWidget()
{
	if (!InventoryRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("UListInventoryWidget::InitializeInventoryWidget: InventoryRef is null!"));
		return;
	}

	CreateTooltipWidget();
}

void UListInventoryWidget::InitializeInventoryWidgetWithSettings()
{
	InitializeInventoryWidget();
	ApplyInventorySettings();
}

void UListInventoryWidget::BindDelegated()
{
	if (!IsValid(InventoryRef))
	{
		return;
	}
	InventoryRef->OnAddItemDelegate.AddUniqueDynamic(this, &UListInventoryWidget::AddItemToPanel);
	InventoryRef->OnItemRemovedDelegate.AddUniqueDynamic(this, &UListInventoryWidget::RemoveItemFromPanel);
	InventoryRef->OnStackedItemDelegate.AddUniqueDynamic(this, &UListInventoryWidget::UpdateItem);
	InventoryRef->OnUnstackedItemDelegate.AddUniqueDynamic(this, &UListInventoryWidget::UpdateItem);

	InventoryRef->OnWeightUpdatedDelegate.AddUniqueDynamic(this, &UListInventoryWidget::UpdateWeightInfo);
	InventoryRef->OnMoneyUpdatedDelegate.AddUniqueDynamic(this, &UListInventoryWidget::UpdateMoneyInfo);
	InventoryRef->OnInventoryRedrawRequested.AddUniqueDynamic(this, &UListInventoryWidget::ReDrawAllItems);
}

void UListInventoryWidget::ReDrawAllItems()
{
	ConfigureTradeFilter();
	if (!ListInventoryRef || !ItemsList)
	{
		return;
	}
	RebuildFilteredSlots();
	ItemsList->SetListItems(ListInventoryRef->FilteredInvSlotsArray);
}

void UListInventoryWidget::ApplyInventorySettings()
{
	if (!ListInventoryRef)
	{
		return;
	}

	auto InvSettings = ListInventoryRef->GetInventorySettings();

	if (!InvSettings.bShowFilterPanel && ItemFiltersPanel)
	{
		ItemFiltersPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UListInventoryWidget::ClearFilters()
{
	ActiveFilters.Empty();
	if (ItemFiltersPanel)
	{
		ItemFiltersPanel->DisableAllFilters();
	}
	RefreshFilteredItemsList();
}

void UListInventoryWidget::OnFilterStatusChanged(UUIButton* ItemCategoryButton)
{
	if (!ItemCategoryButton)
	{
		return;
	}

	const FGameplayTag Category = ItemCategoryButton->GetBtnTag();

	if (ItemCategoryButton->GetToggleStatus())
	{
		ActiveFilters.Add(Category);
	}
	else
	{
		ActiveFilters.Remove(Category);
	}

	RefreshFilteredItemsList();
}

void UListInventoryWidget::RebuildFilteredSlots()
{
	if (!ListInventoryRef)
	{
		return;
	}
	ListInventoryRef->FilteredInvSlotsArray.Reset();
	for (UInventoryListEntry* Entry : ListInventoryRef->InvSlotsArray)
	{
		if (!IsValid(Entry) || !IsValid(Entry->Item))
		{
			continue;
		}
		Entry->ParentInventoryWidget = this;
		if (PassesRecentSalesFilter(Entry->Item) && PassesCategoryAndSearch(Entry->Item))
		{
			ListInventoryRef->FilteredInvSlotsArray.Add(Entry);
		}
	}
}

void UListInventoryWidget::RefreshFilteredItemsList()
{
	ReDrawAllItems();
}

void UListInventoryWidget::SearchTextChanged(const FText& NewText)
{
	RefreshFilteredItemsList();
}

void UListInventoryWidget::SetInventoryBaseRef(UInventoryBase* NewInventoryRef)
{
	if (InventoryRef == NewInventoryRef)
	{
		return;
	}
	if (IsValid(ListInventoryRef))
	{
		for (UInventoryListEntry* Entry : ListInventoryRef->InvSlotsArray)
		{
			if (IsValid(Entry) && Entry->ParentInventoryWidget == this)
			{
				Entry->ParentInventoryWidget = nullptr;
			}
		}
	}
	if (IsValid(InventoryRef))
	{
		InventoryRef->OnAddItemDelegate.RemoveAll(this);
		InventoryRef->OnItemRemovedDelegate.RemoveAll(this);
		InventoryRef->OnStackedItemDelegate.RemoveAll(this);
		InventoryRef->OnUnstackedItemDelegate.RemoveAll(this);
		InventoryRef->OnWeightUpdatedDelegate.RemoveAll(this);
		InventoryRef->OnMoneyUpdatedDelegate.RemoveAll(this);
		InventoryRef->OnInventoryRedrawRequested.RemoveAll(this);
	}
	if (ItemsList)
	{
		ItemsList->ClearListItems();
	}
	ListInventoryRef = Cast<UListInventory>(NewInventoryRef);
	InventoryRef = ListInventoryRef;
	BindDelegated();
}

void UListInventoryWidget::AddItemToPanel(FItemMapping& ItemSlots, UObject* Item)
{
	if (!Item || !ListInventoryRef)
	{
		return;
	}

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("AddItemToPanel")))
	{
		return;
	}

	UInventoryListEntry* TargetEntry = nullptr;

	for (UInventoryListEntry* InvSlot : ListInventoryRef->InvSlotsArray)
	{
		if (InvSlot && InvSlot->Item == Item)
		{
			TargetEntry = InvSlot;
			break;
		}
	}

	if (!TargetEntry)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UListInventoryWidget::ReDrawAllItems);
		return;
	}

	RefreshFilteredItemsList();
}

void UListInventoryWidget::RemoveItemFromPanel(FItemMapping FromSlots, UObject* Item)
{
	if (!Item)
	{
		return;
	}

	UInventoryListEntry* ListEntry = nullptr;
	for (auto Element : ListInventoryRef->InvSlotsArray)
	{
		if (Element->Item == Item)
		{
			ListEntry = Element;
		}
	}

	if (ListEntry)
	{
		ItemsList->RemoveItem(ListEntry);
		ListInventoryRef->InvSlotsArray.Remove(ListEntry);
		ListInventoryRef->FilteredInvSlotsArray.Remove(ListEntry);
	}
}

void UListInventoryWidget::UpdateItem(UObject* Item)
{
	if (!Item)
	{
		return;
	}

	for (UInventoryListEntry* Entry : ListInventoryRef->InvSlotsArray)
	{
		if (Entry->Item == Item)
		{
			ItemsList->RequestRefresh();
			break;
		}
	}
}

void UListInventoryWidget::UpdateWeightInfo(float InventoryTotalWeight)
{
}

void UListInventoryWidget::UpdateMoneyInfo(int32 InventoryTotalMoney)
{
}

FReply UListInventoryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	TArray<UUserWidget*> VisibleWidgets = ItemsList->GetDisplayedEntryWidgets();

	for (UUserWidget* Widget : VisibleWidgets)
	{
		if (Widget && Widget->IsHovered())
		{
			SlotUnderMouse = Cast<UInventorySlot>(Widget);
			//UE_LOG(LogTemp, Log, TEXT("V widget: %s"), *SlotUnderMouse->GetName());
			break;
		}
	}

	return Reply;
}

FReply UListInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	return FReply::Unhandled();
}

FReply UListInventoryWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry,
															const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	return Reply;
}

void UListInventoryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
												UDragDropOperation*& OutOperation)
{
	UInventoryItemWidget* DraggedWidget =
		CreateWidget<UInventoryItemWidget>(GetOwningPlayer(), UISettings.DraggedWidgetClass);
	if (!DraggedWidget)
	{
		return;
	}
	DraggedWidget->SetVisibility(ESlateVisibility::Visible);

	UItemDragDropOperation* DragItemDragDropOperation = NewObject<UItemDragDropOperation>();
	DragItemDragDropOperation->DefaultDragVisual = DraggedWidget;
	DragItemDragDropOperation->Pivot = EDragPivot::CenterCenter;

	//->ItemMoveData.SourceItem = LinkedItem;
	DragItemDragDropOperation->ItemMoveData.SourceInventory = InventoryRef;
	//DragItemDragDropOperation->ItemMoveData.SourceItemPivotSlot = this;

	OutOperation = DragItemDragDropOperation;
}

bool UListInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
										UDragDropOperation* InOperation)
{
	if (!InOperation)
	{
		return false;
	}

	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	auto DragOp = Cast<UItemDragDropOperation>(InOperation);
	DragOp->ItemMoveData.TargetInventory = InventoryRef;

	if (OnItemDroppedDelegate.IsBound())
	{
		OnItemDroppedDelegate.Broadcast(DragOp->ItemMoveData);
	}

	return true;
}
