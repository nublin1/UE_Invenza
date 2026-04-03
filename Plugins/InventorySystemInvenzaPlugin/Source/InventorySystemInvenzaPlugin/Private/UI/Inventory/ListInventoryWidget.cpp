//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/ListInventoryWidget.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/Items/itemBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/ListView.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/ListInventory/ListInventory.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "UI/Core/Buttons/ItemCategoryButton.h"
#include "UI/Core/ItemFiltersPanel/ItemFiltersPanel.h"
#include "UI/HelpersWidgets/ItemTooltipWidget.h"
#include "UI/Inventory/ListInventorySlotWidget.h"
#include "UI/Item/InventoryItemWidget.h"

class UListInventorySlotWidget;

UListInventoryWidget::UListInventoryWidget()
{
}

void UListInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemFiltersPanel->GetSearchText())
	{
		ItemFiltersPanel->GetSearchText()->OnTextChanged.AddDynamic(this, &UListInventoryWidget::SearchTextChanged);
	}

	if (ItemFiltersPanel)
	{
		for (auto FilterButton : ItemFiltersPanel->GetFilteredCategores())
		{
			FilterButton->OnButtonClicked.AddDynamic(this, &UListInventoryWidget::OnFilterStatusChanged);
		}

		if (ItemFiltersPanel->GetClearFiltersButton())
		{			
			ItemFiltersPanel->GetClearFiltersButton()->MainButton->OnClicked.AddDynamic(this, &UListInventoryWidget::ClearFilters);
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
	auto InvSettings = InventoryRef->GetInventorySettings();
	
	if (!InvSettings.bShowItemTooltips || !UISettings.ItemTooltipWidgetClass)
		return;

	ItemTooltipWidget = CreateWidget<UItemTooltipWidget>(this, UISettings.ItemTooltipWidgetClass);
	SetToolTip(ItemTooltipWidget);
	ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UListInventoryWidget::BindDelegated()
{
	InventoryRef->OnAddItemDelegate.AddDynamic(this, &UListInventoryWidget::AddItemToPanel);
	InventoryRef->OnItemRemovedDelegate.AddDynamic(this, &UListInventoryWidget::RemoveItemFromPanel);
	InventoryRef->OnStackedItemDelegate.AddDynamic(this, &UListInventoryWidget::UpdateItem);
	InventoryRef->OnUnstackedItemDelegate.AddDynamic(this, &UListInventoryWidget::UpdateItem);

	InventoryRef->OnWeightUpdatedDelegate.AddDynamic(this, &UListInventoryWidget::UpdateWeightInfo);
	InventoryRef->OnMoneyUpdatedDelegate.AddDynamic(this, &UListInventoryWidget::UpdateMoneyInfo);
	InventoryRef->OnInventoryRedrawRequested.AddDynamic(this, &UListInventoryWidget::ReDrawAllItems);
}

void UListInventoryWidget::ReDrawAllItems()
{
	ItemsList->ClearListItems();
	ListInventoryRef->FilteredInvSlotsArray.Empty();
	
	for (auto InvSlot : ListInventoryRef->InvSlotsArray)
	{
		if (!InvSlot || !InvSlot->Item) continue;

		InvSlot->ParentInventoryWidget = this;

		if (ActiveFilters.Num() == 0 || ActiveFilters.Contains(InvSlot->Item->GetItemRef().ItemCategory))
		{
			ListInventoryRef->FilteredInvSlotsArray.AddUnique(InvSlot);
		}
	}
	
	RefreshFilteredItemsList();
	if (ItemFiltersPanel && ItemFiltersPanel->GetSearchText())
	{
		FText SearchText = ItemFiltersPanel->GetSearchText()->GetText();
		if (!SearchText.IsEmpty())
		{
			SearchTextChanged(SearchText);
		}
	}
    
	ItemsList->RegenerateAllEntries();
	ItemsList->RequestRefresh();
}

void UListInventoryWidget::ClearFilters()
{
	ActiveFilters.Empty();
	ListInventoryRef->FilteredInvSlotsArray.Empty();
	ItemsList->ClearListItems();
	for (auto InvSlot : ListInventoryRef->InvSlotsArray)
	{
		ItemsList->AddItem(InvSlot);
	}

	if (ItemFiltersPanel->GetSearchText())
		SearchTextChanged(ItemFiltersPanel->GetSearchText()->GetText());
}

void UListInventoryWidget::OnFilterStatusChanged(UUIButton* ItemCategoryButton)
{
	auto CastedCategoryButton = Cast<UItemCategoryButton>(ItemCategoryButton);
	if (!CastedCategoryButton)
		return;

	const EItemCategory Category = CastedCategoryButton->GetItemCategory();

	if (CastedCategoryButton->GetToggleStatus())
	{
		ActiveFilters.Add(Category);
	}
	else
	{
		ActiveFilters.Remove(Category);
	}
	
	RebuildFilteredSlots();
	RefreshFilteredItemsList();

	auto SearchText = ItemFiltersPanel->GetSearchText()->GetText();
	if (!SearchText.IsEmpty())
	{
		SearchTextChanged(SearchText);
	}
}

void UListInventoryWidget::RebuildFilteredSlots()
{
	ListInventoryRef->FilteredInvSlotsArray.Empty();

	if (ActiveFilters.Num() == 0)
		return;

	for (auto InvSlot : ListInventoryRef->InvSlotsArray)
	{
		if (!InvSlot || !InvSlot->Item)
			continue;
		if (ActiveFilters.Contains(InvSlot->Item->GetItemRef().ItemCategory))
			ListInventoryRef->FilteredInvSlotsArray.AddUnique(InvSlot);
	}
}

void UListInventoryWidget::RefreshFilteredItemsList()
{
	ItemsList->ClearListItems();	
	if (ActiveFilters.Num() == 0)
	{
		for (auto InvSlot : ListInventoryRef->InvSlotsArray)
		{
			ItemsList->AddItem(InvSlot);
		}
	}
	else
	{
		for (auto FiltredInvSlot : ListInventoryRef->FilteredInvSlotsArray)
		{
			ItemsList->AddItem(FiltredInvSlot);
		}
	}
}

void UListInventoryWidget::SearchTextChanged(const FText& NewText)
{
	const TArray<TObjectPtr<UInventoryListEntry>>& SourceArray = ItemFiltersPanel->IsSearchInFilteredSlots() ? ListInventoryRef->FilteredInvSlotsArray : ListInventoryRef->InvSlotsArray;

	ItemsList->ClearListItems();
	if (NewText.IsEmpty())
	{
		for (auto InvSlot : SourceArray)
		{
			ItemsList->AddItem(InvSlot);
		}
		return;
	}
	
	for (auto InvSlot : SourceArray)
	{
		FString StringName = InvSlot->Item->GetItemRef().ItemTextData.DisplayName.ToString();
		if (StringName.Contains(NewText.ToString(), ESearchCase::IgnoreCase))
		{
			ItemsList->AddItem(InvSlot);
		}
	}
}

void UListInventoryWidget::SetInventoryBaseRef(UInventoryBase* NewInventoryRef)
{
	if (UListInventory* ListInventory = Cast<UListInventory>(NewInventoryRef))
	{
		InventoryRef = NewInventoryRef;
		ListInventoryRef = ListInventory;
	}
}

void UListInventoryWidget::AddItemToPanel(FItemMapping& ItemSlots, UItemBase* Item)
{
	if (!Item || !ListInventoryRef) return;

	UInventoryListEntry* TargetEntry = nullptr;
	
	for (auto& InvSlot : ListInventoryRef->InvSlotsArray)
	{
		if (InvSlot->Item == Item)
		{
			TargetEntry = InvSlot;
			break;
		}
	}
	
	if (!TargetEntry)
	{
		TargetEntry = NewObject<UInventoryListEntry>(ListInventoryRef, ListInventoryRef->GetEntryClass());
		TargetEntry->Item = Item;
		ListInventoryRef->InvSlotsArray.Add(TargetEntry);
	}

	TargetEntry->ParentInventoryWidget = this;
	
	if (ActiveFilters.Num() == 0 || ActiveFilters.Contains(Item->GetItemRef().ItemCategory))
	{
		ListInventoryRef->FilteredInvSlotsArray.AddUnique(TargetEntry);
	}
	
	RefreshFilteredItemsList();
	
	if (ItemFiltersPanel && ItemFiltersPanel->GetSearchText())
	{
		auto SearchText = ItemFiltersPanel->GetSearchText()->GetText();
		if (!SearchText.IsEmpty())
		{
			SearchTextChanged(SearchText);
		}
	}
}

void UListInventoryWidget::RemoveItemFromPanel(FItemMapping FromSlots, UItemBase* Item)
{
	if (!Item) return;

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

void UListInventoryWidget::UpdateItem(UItemBase* Item, int32 ChangedAmount)
{
	if (!Item) return;

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

void UListInventoryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                                UDragDropOperation*& OutOperation)
{
	UInventoryItemWidget* DraggedWidget = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(), UISettings.DraggedWidgetClass);
	if (!DraggedWidget) return;
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
	if (!InOperation) return false;

	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	auto DragOp = Cast<UItemDragDropOperation>(InOperation);
	DragOp->ItemMoveData.TargetInventory = InventoryRef;
	DragOp->ItemMoveData.TargetSlot = nullptr;

	if (OnItemDroppedDelegate.IsBound())
			OnItemDroppedDelegate.Broadcast(DragOp->ItemMoveData);
	
	return true;
}
