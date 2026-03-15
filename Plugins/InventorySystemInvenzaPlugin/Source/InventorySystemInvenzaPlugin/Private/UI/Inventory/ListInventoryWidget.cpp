//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/ListInventoryWidget.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/Items/itemBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/ListView.h"
#include "Data/Inventory/InventoryBase.h"
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

void UListInventoryWidget::ClearFilters()
{
	ActiveFilters.Empty();
	FiltredInvSlotsArray.Empty();
	ItemsList->ClearListItems();
	for (auto InvSlot : InvSlotsArray)
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
	
	FiltredInvSlotsArray.Empty();

	if (ActiveFilters.Num() > 0)
	{
		for (auto InvSlot : InvSlotsArray)
		{
			if (ActiveFilters.Contains(InvSlot->Item->GetItemRef().ItemCategory))
			{
				FiltredInvSlotsArray.AddUnique(InvSlot);
			}
		}
	}

	RefreshFilteredItemsList();

	auto SearchText = ItemFiltersPanel->GetSearchText()->GetText();
	if (!SearchText.IsEmpty())
	{
		SearchTextChanged(SearchText);
	}
}

void UListInventoryWidget::RefreshFilteredItemsList()
{
	ItemsList->ClearListItems();	
	if (ActiveFilters.Num() == 0)
	{
		for (auto InvSlot : InvSlotsArray)
		{
			ItemsList->AddItem(InvSlot);
		}
	}
	else
	{
		for (auto FiltredInvSlot : FiltredInvSlotsArray)
		{
			ItemsList->AddItem(FiltredInvSlot);
		}
	}
}

void UListInventoryWidget::SearchTextChanged(const FText& NewText)
{
	const TArray<TObjectPtr<UInventoryListEntry>>& SourceArray = ItemFiltersPanel->IsSearchInFilteredSlots() ? FiltredInvSlotsArray : InvSlotsArray;

	ItemsList->ClearListItems();
	
	if (NewText.IsEmpty())
	{
		if (ActiveFilters.Num() > 0)
		{
			for (auto InvSlot : SourceArray)
			{
				ItemsList->AddItem(InvSlot);
			}
		}
		else
		{
			for (auto InvSlot : SourceArray)
			{
				ItemsList->AddItem(InvSlot);
			}
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

void UListInventoryWidget::InitializeInventoryWidget()
{
	auto InvSettings = InventoryRef->GetInventorySettings();
	
	if (!InvSettings.bShowItemTooltips || !UISettings.ItemTooltipWidgetClass)
		return;

	ItemTooltipWidget = CreateWidget<UItemTooltipWidget>(this, UISettings.ItemTooltipWidgetClass);
	SetToolTip(ItemTooltipWidget);
	ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UListInventoryWidget::SortInventory()
{
	if (FiltredInvSlotsArray.Num() > 1)
	{
		FiltredInvSlotsArray.Sort([](const TObjectPtr<UInventoryListEntry>& A, const TObjectPtr<UInventoryListEntry>& B)
		{
			if (!A || !B || !A->Item || !B->Item)
			{
				return false;
			}
			const FString NameA = A.Get()->Item->GetItemID().ToString();
			const FString NameB = B.Get()->Item->GetItemID().ToString();
			return NameA < NameB;
		});

		ItemsList->ClearListItems();
		for (auto FiltredInvSlot : FiltredInvSlotsArray)
		{
			ItemsList->AddItem(FiltredInvSlot);
		}
		
		return;
	}

	if (InvSlotsArray.Num() > 1)
	{
		InvSlotsArray.Sort([](const TObjectPtr<UInventoryListEntry>& A, const TObjectPtr<UInventoryListEntry>& B)
		{
			if (!A || !B || !A->Item || !B->Item)
			{
				return false;
			}
			const FString NameA = A.Get()->Item->GetItemRef().ItemTextData.DisplayName.ToString();
			const FString NameB = B.Get()->Item->GetItemRef().ItemTextData.DisplayName.ToString();
			return NameA < NameB;
		});
		
		ReDrawAllItems();
	}
}

void UListInventoryWidget::BindDelegated()
{
	InventoryRef->OnAddItemDelegate.AddDynamic(this, &UListInventoryWidget::AddItemToPanel);
	InventoryRef->OnItemRemovedDelegate.AddDynamic(this, &UListInventoryWidget::RemoveItemFromPanel);
	InventoryRef->OnItemReplaceDelegate.AddDynamic(this, &UListInventoryWidget::ReplaceItemInPanel);
	InventoryRef->OnStackedItemDelegate.AddDynamic(this, &UListInventoryWidget::UpdateItem);
	InventoryRef->OnUnstackedItemDelegate.AddDynamic(this, &UListInventoryWidget::UpdateItem);
	//InventoryRef->OnUseSlotDelegate.AddDynamic(this, &UListInventoryWidget::UsedItemInPanel);

	InventoryRef->OnWeightUpdatedDelegate.AddDynamic(this, &UListInventoryWidget::UpdateWeightInfo);
	InventoryRef->OnMoneyUpdatedDelegate.AddDynamic(this, &UListInventoryWidget::UpdateMoneyInfo);
}

void UListInventoryWidget::ReDrawAllItems()
{
	auto Items = InventoryRef->GetItemCollectionLinked()->GetAllItemsByContainer(InventoryRef->GetInventoryContainerID());
	if (Items.IsEmpty()) return;

	ItemsList->ClearListItems();
	InvSlotsArray.Empty();
	FiltredInvSlotsArray.Empty();
	for (auto Item : Items)
	{
		auto Mapping = InventoryRef->GetItemCollectionLinked()->FindItemMappingByContainerName(Item, InventoryRef->GetInventoryContainerID());
		AddItemToPanel(*Mapping, Item);
	}

	RefreshFilteredItemsList();
	
	ItemsList->RegenerateAllEntries();
	ItemsList->RequestRefresh();
}

void UListInventoryWidget::AddItemToPanel(FItemMapping ItemSlots, UItemBase* Item)
{
	UInventoryListEntry* EntryObject = NewObject<UInventoryListEntry>();
	EntryObject->Item = Item;
	EntryObject->ParentInventoryWidget = this;

	InvSlotsArray.Add(EntryObject);
	if (ActiveFilters.Num() == 0 || ActiveFilters.Contains(Item->GetItemRef().ItemCategory))
	{
		FiltredInvSlotsArray.Add(EntryObject);
	}
	
	RefreshFilteredItemsList();
	if (ItemFiltersPanel)
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
	
	if (InventoryRef->GetItemCollectionLinked())
	{
		InventoryRef->GetItemCollectionLinked()->RemoveItem(Item, InventoryRef->GetInventoryContainerID());
	}

	UInventoryListEntry* ListEntry = nullptr;
	for (auto Element : InvSlotsArray)
	{
		if (Element->Item == Item)
		{
			ListEntry = Element;
		}
	}

	if (ListEntry)
	{
		ItemsList->RemoveItem(ListEntry);
		InvSlotsArray.Remove(ListEntry);
		FiltredInvSlotsArray.Remove(ListEntry);
	}
	
}

void UListInventoryWidget::UpdateItem(UItemBase* Item, int32 ChangedAmount)
{
	
	if (!Item) return;

	auto Mapping = SlotBasedInventoryRef->GetItemCollectionLinked()->FindItemMappingByContainerName(Item, SlotBasedInventoryRef->GetInventoryContainerID());
	if (!Mapping)
	{
		return;
	}

	Mapping->ItemVisualLinked->UpdateQuantityText(Item->GetQuantity());
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
