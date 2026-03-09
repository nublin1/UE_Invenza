//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/ListInventoryWidget.h"

#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Items/itemBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/ListView.h"
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
		FString StringName = InvSlot->Item->GetItemRef().ItemTextData.Name.ToString();
		if (StringName.Contains(NewText.ToString(), ESearchCase::IgnoreCase))
		{
			ItemsList->AddItem(InvSlot);
		}
	}
}

void UListInventoryWidget::InitializeInventoryWidget()
{
	if (!InventorySettings.bShowTooltips || !UISettings.ItemTooltipWidgetClass)
		return;
	
	InventoryData.ItemTooltipWidget = CreateWidget<UItemTooltipWidget>(this, UISettings.ItemTooltipWidgetClass);
	SetToolTip(InventoryData.ItemTooltipWidget);
	InventoryData.ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
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
			const FString NameA = A.Get()->Item->GetItemRef().ItemTextData.Name.ToString();
			const FString NameB = B.Get()->Item->GetItemRef().ItemTextData.Name.ToString();
			return NameA < NameB;
		});
		
		ReDrawAllItems();
	}
}

void UListInventoryWidget::ReDrawAllItems()
{
	auto Items = InventoryData.ItemCollectionLink->GetAllItemsByContainer(Cast<UInvBaseContainerWidget>(ParentWidget));
	if (Items.IsEmpty()) return;

	ItemsList->ClearListItems();
	InvSlotsArray.Empty();
	FiltredInvSlotsArray.Empty();
	for (auto Item : Items)
	{
		AddItemToPanel(Item);
	}

	RefreshFilteredItemsList();
	
	ItemsList->RegenerateAllEntries();
	ItemsList->RequestRefresh();
}

void UListInventoryWidget::HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity)
{
	if (!Item) return;

	Item->SetQuantity(Item->GetQuantity() - RemoveQuantity);	
	auto Slots = GetItemMapping(Item);
	if (Slots)
	{
		NotifyPreRemoveItem(*Slots, Item, RemoveQuantity);
		NotifyPostRemoveItem();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to find occupied slots for item %s"), *Item->GetName());
	}
	
	if (Item->GetQuantity() <=0)
		InventoryData.ItemCollectionLink->RemoveItemFromAllContainers(Item);
}

void UListInventoryWidget::HandleRemoveItemFromContainer(UItemBase* Item)
{
	if (!Item) return;

	auto Mapping = GetItemMapping(Item);
	if (!Mapping) return;

	NotifyPreRemoveItem(*Mapping, Item, Item->GetQuantity());
	if (InventoryData.ItemCollectionLink)
	{
		InventoryData.ItemCollectionLink->RemoveItem(Item, GetAsContainerWidget());
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

	UpdateWeightInfo();
	NotifyPostRemoveItem();
}

FItemAddResult UListInventoryWidget::HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	if (!ItemMoveData.SourceItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item is null. Nothing to add"));
		return FItemAddResult::AddedNone(FText::FromString("Item is null. Nothing to add"));
	}
	
	if (!ItemsList)
		UE_LOG(LogTemp, Warning, TEXT("ItemsList is null"));

	if (InventorySettings.bUseReferences)
		return HandleAddReferenceItem(ItemMoveData, bOnlyCheck);

	if (ItemMoveData.SourceInventory
		&& ItemMoveData.SourceInventory->GetInventorySettings().bUseReferences
		&& ItemMoveData.SourceInventory != ItemMoveData.TargetInventory)
	{
		auto Mapping = InventoryData.ItemCollectionLink->FindItemMappingForItemInContainer(ItemMoveData.SourceItem, GetAsContainerWidget());
		if (Mapping)
			return FItemAddResult::AddedNone(FText::FromString("Item already exists.")); 
	}

	if (!ItemMoveData.SourceItem->IsStackable())
		return HandleNonStackableItems(ItemMoveData, bOnlyCheck);

	const int32 InitialRequestedAddAmount = ItemMoveData.SourceItem->GetQuantity();
	const int32 StackableAmountAdded = HandleStackableItems(ItemMoveData, InitialRequestedAddAmount, bOnlyCheck);
	if (StackableAmountAdded == InitialRequestedAddAmount)
	{
		return FItemAddResult::AddedAll(StackableAmountAdded, false, FText::Format(
			FText::FromString("Successfully added {0} of {1} to inventory"),
			InitialRequestedAddAmount, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}
	else if (StackableAmountAdded < InitialRequestedAddAmount && StackableAmountAdded > 0)
	{
		return FItemAddResult::AddedPartial(StackableAmountAdded, false, FText::Format(
			FText::FromString("Partial amount of {0} added to inventory. Number added: {1}"),
			ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name, StackableAmountAdded));
	}
    
	return FItemAddResult::AddedNone(FText::Format(FText::FromString("Couldn't add {0} to inventory."),
												   ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
}

FItemAddResult UListInventoryWidget::HandleNonStackableItems(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (InventorySettings.InventoryMaxWeightCapacity >= 0)
	{
		if (InventoryData.InventoryTotalWeight + ItemMoveData.SourceItem->GetItemSingleWeight() > InventorySettings.InventoryMaxWeightCapacity)
		{
			return FItemAddResult::AddedNone(FText::Format(
				FText::FromString("Item {0} would overflow weight limit"),
				ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
		}
	}

	if (!bOnlyCheck)
	{
		UListInventorySlotWidget* ListInventorySlot = NewObject<UListInventorySlotWidget>();
		FItemMapping Slots (ListInventorySlot->GetSlotData());
		AddNewItem(ItemMoveData, Slots, 1);
	}

	return FItemAddResult::AddedAll(1, false, FText::Format(
										FText::FromString("Successfully added {0} of {1} to inventory"),
										1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
}

int32 UListInventoryWidget::HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck)
{
	int32 AmountToDistribute = RequestedAddAmount;
	int32 TotalAddedAmount = 0;

	auto SameItems = InventoryData.ItemCollectionLink->GetAllSameItemsInContainer(GetAsContainerWidget(), ItemMoveData.SourceItem);
	if (SameItems.Num()> 0)
	{
		for (auto& Item : SameItems)
		{
			if(AmountToDistribute<=0)
				break;
				
			if (Item->IsFullItemStack())
				continue;

			int32 AmountToAddToStack = FMath::Min(AmountToDistribute,
					Item->GetItemRef().ItemNumeraticData.MaxStackSize - Item->GetQuantity());
			int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack, ItemMoveData.SourceItem->GetItemSingleWeight());

			if (!bOnlyCheck)
				InsertToStackItem(Item, ActualAmountToAdd);
			AmountToDistribute -= ActualAmountToAdd;
			TotalAddedAmount += ActualAmountToAdd;
		}
	}

	if (AmountToDistribute<=0) return RequestedAddAmount;
	const int32 AmountToAddToStack = FMath::Min(AmountToDistribute, ItemMoveData.SourceItem->GetQuantity());
	int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack, ItemMoveData.SourceItem->GetItemSingleWeight());
		
	if (bOnlyCheck)
		return RequestedAddAmount;

	UListInventorySlotWidget* ListInventorySlot = NewObject<UListInventorySlotWidget>();	
	FItemMapping Slots(ListInventorySlot->GetSlotData());
			
	AddNewItem(ItemMoveData, Slots, AmountToDistribute);
	return ActualAmountToAdd + TotalAddedAmount;
}

FItemAddResult UListInventoryWidget::HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (InventoryData.ItemCollectionLink->FindItemMappingForItemInContainer(ItemMoveData.SourceItem, GetAsContainerWidget()))
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}

	if (ItemMoveData.SourceInventory == this)
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}

	UListInventorySlotWidget* ListInventorySlot = NewObject<UListInventorySlotWidget>();	
	FItemMapping Slots;
	Slots.OccupiedSlots.Add(ListInventorySlot->GetSlotData());

	if (!bOnlyCheck)
		AddNewItem(ItemMoveData, Slots, ItemMoveData.SourceItem->GetQuantity());

	return FItemAddResult::AddedAll(1, true, FText::Format(
		FText::FromString("Successfully added {0} to inventory as a reference"),
		ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
}

void UListInventoryWidget::AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount)
{
	TObjectPtr<UItemBase> FinalItem;
	if (InventorySettings.bUseReferences)
	{
		FinalItem = ItemMoveData.SourceItem;
	}
	else
	{
		FinalItem = ItemMoveData.SourceItem->DuplicateItem();
		FinalItem->SetQuantity(AddAmount);
	}

	if (InventoryData.ItemCollectionLink)
	{
		OccupiedSlots.InventoryID = GetAsContainerWidget()->GetFName();
		OccupiedSlots.InventoryType = GetAsContainerWidget()->GetInventoryType();
		InventoryData.ItemCollectionLink->AddItem(FinalItem, OccupiedSlots);
		NotifyAddItem(OccupiedSlots, FinalItem,ItemMoveData.SourceItem->GetQuantity());
	}
}

void UListInventoryWidget::InsertToStackItem(UItemBase* Item, int32 AddQuantity)
{
	Item->SetQuantity(Item->GetQuantity() + AddQuantity);
	UpdateWeightInfo();
	UpdateMoneyInfo();
	auto Slots = GetItemMapping(Item);
	ItemsList->RegenerateAllEntries();
	if (OnAddItemDelegate.IsBound())
		OnAddItemDelegate.Broadcast(*Slots, Item);
}

void UListInventoryWidget::AddItemToPanel(UItemBase* Item)
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

void UListInventoryWidget::NotifyAddItem(FItemMapping& FromSlots, UItemBase* NewItem, int32 ChangeQuantity)
{
	Super::NotifyAddItem(FromSlots, NewItem, ChangeQuantity);
	AddItemToPanel(NewItem);
	UpdateWeightInfo();
}

void UListInventoryWidget::NotifyPreRemoveItem(FItemMapping& FromSlots, UItemBase* RemovedItem, int32 RemoveQuantity)
{
	Super::NotifyPreRemoveItem(FromSlots, RemovedItem, RemoveQuantity);
	ReDrawAllItems();
	UpdateWeightInfo();
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
	DragItemDragDropOperation->ItemMoveData.SourceInventory = this;
	//DragItemDragDropOperation->ItemMoveData.SourceItemPivotSlot = this;

	OutOperation = DragItemDragDropOperation;
}

bool UListInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                        UDragDropOperation* InOperation)
{
	if (!InOperation) return false;

	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	auto DragOp = Cast<UItemDragDropOperation>(InOperation);
	DragOp->ItemMoveData.TargetInventory = this;
	DragOp->ItemMoveData.TargetSlot = nullptr;

	if (OnItemDroppedDelegate.IsBound())
			OnItemDroppedDelegate.Broadcast(DragOp->ItemMoveData);
	
	return true;
}
