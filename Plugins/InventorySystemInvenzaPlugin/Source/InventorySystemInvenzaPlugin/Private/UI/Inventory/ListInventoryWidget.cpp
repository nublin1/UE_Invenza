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
	
	CreateTooltipWidget();
}

void UListInventoryWidget::InitializeInventoryWidgetWithSettings(FInventorySettings InventoryStartupData)
{
	InitializeInventoryWidget();
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
	if (!ListInventoryRef || !ItemsList) return;
	
	ItemsList->ClearListItems();
	ListInventoryRef->FilteredInvSlotsArray.Reset();
	
	for (UInventoryListEntry* InvSlot : ListInventoryRef->InvSlotsArray)
	{
		if (!IsValid(InvSlot) || !IsValid(InvSlot->Item))
			continue;
		
		InvSlot->ParentInventoryWidget = this;
		bool bPassesFilter = (ActiveFilters.Num() == 0) || 
							 ActiveFilters.Contains(InvSlot->Item->GetItemRef().ItemCategory);

		if (bPassesFilter)
		{
			ListInventoryRef->FilteredInvSlotsArray.Add(InvSlot);
		}
	}
	
	if (ItemFiltersPanel)
	{
		if (auto SearchBox = ItemFiltersPanel->GetSearchText())
		{
			FText SearchText = SearchBox->GetText();
			if (!SearchText.IsEmpty())
			{
				SearchTextChanged(SearchText);
				return; 
			}
		}
	}
	
	ItemsList->SetListItems(ListInventoryRef->FilteredInvSlotsArray);
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
			ListInventoryRef->FilteredInvSlotsArray.Add(InvSlot);
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

	TargetEntry->ParentInventoryWidget = this;
	bool bPassesFilter = (ActiveFilters.Num() == 0) || 
						 ActiveFilters.Contains(Item->GetItemRef().ItemCategory);
    
	if (bPassesFilter)
	{
		ListInventoryRef->FilteredInvSlotsArray.AddUnique(TargetEntry);
		ItemsList->AddItem(TargetEntry);
		
		if (ItemFiltersPanel && ItemFiltersPanel->GetSearchText())
		{
			SearchTextChanged(ItemFiltersPanel->GetSearchText()->GetText());
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

void UListInventoryWidget::UpdateItem(UItemBase* Item)
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

FReply UListInventoryWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	return Reply;
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
	DragOp->ItemMoveData.TargetSlotCoordinate = FIntPoint(-1);

	if (OnItemDroppedDelegate.IsBound())
			OnItemDroppedDelegate.Broadcast(DragOp->ItemMoveData);
	
	return true;
}
