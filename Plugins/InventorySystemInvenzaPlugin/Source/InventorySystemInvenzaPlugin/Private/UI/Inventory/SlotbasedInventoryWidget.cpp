//  Nublin Studio 2025 All Rights Reserved.

#include "UI/Inventory/SlotbasedInventoryWidget.h"

#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/Items/itemBase.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "Slate/SObjectWidget.h"
#include "UI/Core/CoreCellWidget.h"
#include "UI/Core/Buttons/ItemCategoryButton.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/ItemFiltersPanel/ItemFiltersPanel.h"
#include "UI/Drag/HighlightSlotWidget.h"
#include "UI/HelpersWidgets/ItemTooltipWidget.h"
#include "UI/Inventory/ListInventoryWidget.h"
#include "UI/Inventory/SlotbasedInventorySlot.h"
#include "UI/Item/InventoryItemWidget.h"

USlotbasedInventoryWidget::USlotbasedInventoryWidget(): SlotsGridPanel(nullptr)
{
}

void USlotbasedInventoryWidget::InitializeInventory()
{
	InitSlots();
	UpdateWeightInfo();
	UpdateMoneyInfo();
	CreateTooltipWidget();
}

void USlotbasedInventoryWidget::ReDrawAllItems()
{
	if (!InventoryData.ItemCollectionLink || !ItemsVisualsPanel)
		return;

	ItemsVisualsPanel->ClearChildren();
	for (auto& InventorySlot : InventoryData.InventorySlots)
	{
		InventorySlot->ResetVisual();
	}

	auto AllItems =InventoryData.ItemCollectionLink->GetAllItemsByContainer(GetAsContainerWidget());
	for (auto Item : AllItems)
	{
		AddItemToPanel(Item);
	}
}

void USlotbasedInventoryWidget::RebuildSlots(int32 InRows, int32 InColumns)
{
	if (!SlotsGridPanel || !UISettings.DefaultSlotbasedInventorySlotClass)
	{
		return;
	}
	
	SlotsGridPanel->ClearChildren();
	InventoryData.InventorySlots.Empty();

	for (int32 Row = 0; Row < InRows; ++Row)
	{
		for (int32 Col = 0; Col < InColumns; ++Col)
		{
			USlotbasedInventorySlot* NewSlot = CreateWidget<USlotbasedInventorySlot>(GetOwningPlayer(),
				UISettings.DefaultSlotbasedInventorySlotClass);
			if (!NewSlot)
				continue;
			
			NewSlot->SetSlotPosition({ Row, Col });
			
			UUniformGridSlot* GridSlot = SlotsGridPanel->AddChildToUniformGrid(NewSlot, Row, Col);
			if (GridSlot)
			{
			}
			
			InventoryData.InventorySlots.Add(NewSlot);
		}
	}
	
	NumberRows  = InRows;
	NumColumns  = InColumns;
	InventoryData.InventoryTotalWeight = 0; 
	InventoryData.InventoryTotalMoney  = 0;

	//UE_LOG(LogTemp, Log, TEXT("RebuildSlots: создано %d x %d = %d ячеек"),
		//InRows, InColumns, InRows * InColumns);
}

void USlotbasedInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ItemFiltersPanel)
	{
		if (ItemFiltersPanel->GetSearchText())
		{
			ItemFiltersPanel->GetSearchText()->OnTextChanged.AddDynamic(this, &USlotbasedInventoryWidget::SearchTextChanged);
		}
		
		for (auto FilterButton : ItemFiltersPanel->GetFilteredCategores())
		{
			FilterButton->OnButtonClicked.AddDynamic(this, &USlotbasedInventoryWidget::OnFilterStatusChanged);
		}

		if (ItemFiltersPanel->GetClearFiltersButton())
		{			
			ItemFiltersPanel->GetClearFiltersButton()->MainButton->OnClicked.AddDynamic(this, &USlotbasedInventoryWidget::ClearFilters);
		}
	}
}

void USlotbasedInventoryWidget::InitSlots()
{
	if (!SlotsGridPanel)
		return;
	
	TArray<TObjectPtr<USlotbasedInventorySlot>> NewInvSlots;
	const int32 NumChildren = SlotsGridPanel->GetChildrenCount();

	for (int32 i = 0; i < NumChildren; ++i)
	{
		if (UWidget* ChildWidget = SlotsGridPanel->GetChildAt(i))
		{
			auto WClass = ChildWidget->GetClass();
			if (WClass->IsChildOf(USlotbasedInventorySlot::StaticClass()))
			{
				if (auto InventorySlot = Cast<USlotbasedInventorySlot>(ChildWidget))
				{
					NewInvSlots.Add(InventorySlot);
				}
			}
		}
	}

	for (int32 i = 0; i < NewInvSlots.Num(); ++i)
	{
		if (const UWidget* ChildWidget = SlotsGridPanel->GetChildAt(i))
		{
			const UUniformGridSlot* UniSlot = Cast<UUniformGridSlot>(ChildWidget->Slot);
			if (UniSlot->GetRow() >= NumberRows)
				NumberRows = UniSlot->GetRow() + 1;
			if (UniSlot->GetColumn() >= NumColumns)
				NumColumns = UniSlot->GetColumn() + 1;

			NewInvSlots[i]->SetSlotPosition(FIntVector2(UniSlot->GetRow(),  UniSlot->GetColumn()));
		}
	}

	SlotSpacing = SlotsGridPanel->GetSlotPadding();
	
	TArray<UInventorySlot*> ConvertedSlots;
	for (auto InvSlot : NewInvSlots)
	{
		if (InvSlot)
		{
			ConvertedSlots.Add(InvSlot);
		}
	}
	InventoryData.InventorySlots = ConvertedSlots;
}

void USlotbasedInventoryWidget::ClearFilters()
{
	for (auto Item : InventoryData.ItemCollectionLink->GetAllItemsByContainer(GetAsContainerWidget()))
	{
		auto ItemMapping = InventoryData.ItemCollectionLink->FindItemMappingForItemInContainer(Item, GetAsContainerWidget());
		if (!ItemMapping)
			continue;

		ItemMapping->ItemVisualLinked->GetCoreCellWidget()->ResetBorderColor();
		ItemMapping->ItemVisualLinked->ChangeOpacity(1.0f);
	}

	ActiveFilters.Empty();
}

void USlotbasedInventoryWidget::OnFilterStatusChanged(UUIButton* ItemCategoryButton)
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

	// Перерисовываем все слоты под текущие ActiveFilters
	RefreshFilteredItemsList();
}

void USlotbasedInventoryWidget::RefreshFilteredItemsList()
{
	if (ActiveFilters.Num() == 0)
	{
		ClearFilters();
		return;
	}

	// Сначала затемняем все и сбрасываем бордер у тех, кто был ранее подсвечен
	for (auto& Item : InventoryData.ItemCollectionLink->GetAllItemsByContainer(GetAsContainerWidget()))
	{
		if (auto Mapping = InventoryData.ItemCollectionLink->FindItemMappingForItemInContainer(Item, GetAsContainerWidget()))
		{
			Mapping->ItemVisualLinked->ChangeOpacity(ItemFiltersPanel->FilterOpacity);
			Mapping->ItemVisualLinked->GetCoreCellWidget()->ResetBorderColor();
		}
	}

	// Потом для каждой активной категории — подсвечиваем и делаем полную opacity
	for (auto ActiveCategory : ActiveFilters)
	{
		for (auto& Item : InventoryData.ItemCollectionLink->GetAllItemsByCategory(ActiveCategory))
		{
			if (auto Mapping = InventoryData.ItemCollectionLink->FindItemMappingForItemInContainer(Item, GetAsContainerWidget()))
			{
				if (ItemFiltersPanel->bUseFilterColor)
				{
					Mapping->ItemVisualLinked->ChangeBorderColor(ItemFiltersPanel->ItemFilterBorderColor);
				}
				Mapping->ItemVisualLinked->ChangeOpacity(1.0f);
			}
		}
	}
}

void USlotbasedInventoryWidget::SearchTextChanged(const FText& NewText)
{
	if (ItemFiltersPanel->IsSearchInFilteredSlots())
	{
		for (auto ActiveFilter : ActiveFilters)
		{
			for (auto Item : InventoryData.ItemCollectionLink->GetAllItemsByCategory(ActiveFilter))
			{
				auto ItemMapping = InventoryData.ItemCollectionLink->FindItemMappingForItemInContainer(Item, GetAsContainerWidget());
				if (!ItemMapping)
					continue;

				FString StringName = Item->GetItemRef().ItemTextData.Name.ToString();
				if (StringName.Contains(NewText.ToString(), ESearchCase::IgnoreCase))
				{
					ItemMapping->ItemVisualLinked->ChangeBorderColor(ItemFiltersPanel->ItemFilterBorderColor);
					ItemMapping->ItemVisualLinked->ChangeOpacity(1.0f);
				}
				else
				{
					ItemMapping->ItemVisualLinked->GetCoreCellWidget()->ResetBorderColor();
					ItemMapping->ItemVisualLinked->ChangeOpacity(ItemFiltersPanel->FilterOpacity);
				}
			}
		}
				
		return;
	}

	for (auto Item : InventoryData.ItemCollectionLink->GetAllItemsByContainer(GetAsContainerWidget()))
	{
		auto ItemMapping = InventoryData.ItemCollectionLink->FindItemMappingForItemInContainer(Item, GetAsContainerWidget());
		if (!ItemMapping)
			continue;

		FString StringName = Item->GetItemRef().ItemTextData.Name.ToString();
		if (StringName.Contains(NewText.ToString(), ESearchCase::IgnoreCase))
		{
			ItemMapping->ItemVisualLinked->ChangeBorderColor(ItemFiltersPanel->ItemFilterBorderColor);
			ItemMapping->ItemVisualLinked->ChangeOpacity(1.0f);
		}
		else
		{
			ItemMapping->ItemVisualLinked->GetCoreCellWidget()->ResetBorderColor();
			ItemMapping->ItemVisualLinked->ChangeOpacity(ItemFiltersPanel->FilterOpacity);
		}
	}
}

UInventorySlot* USlotbasedInventoryWidget::GetSlotByPosition(FIntVector2 SlotPosition)
{
	for (auto& Elem : InventoryData.InventorySlots )
	{
		if (Elem->GetSlotPosition() == SlotPosition)
			return Elem;
	}

	return nullptr;
}

bool USlotbasedInventoryWidget::bIsSlotEmpty(const FIntVector2 SlotPosition)
{
	auto BusySlots = InventoryData.ItemCollectionLink->CollectOccupiedSlotsByContainer(GetAsContainerWidget());
	for (const auto InvSlotData : BusySlots)
	{
		if (InvSlotData.SlotPosition == SlotPosition)
			return false;
	}
	
	return true;
}

bool USlotbasedInventoryWidget::bIsSlotEmpty(const UInventorySlot* SlotCheck)
{
	auto BusySlots = InventoryData.ItemCollectionLink->CollectOccupiedSlotsByContainer(GetAsContainerWidget());
	for (auto InvSlotData : BusySlots)
	{
		if (InvSlotData == SlotCheck->GetSlotData())
			return false;
	}
	return true;
}

bool USlotbasedInventoryWidget::bIsGridPositionValid(FIntPoint& GridPosition)
{
	return GridPosition.X >= 0 && GridPosition.Y >= 0 && GridPosition.X<=NumberRows && GridPosition.Y<=NumColumns;
}

void USlotbasedInventoryWidget::HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity)
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

void USlotbasedInventoryWidget::HandleRemoveItemFromContainer(UItemBase* Item)
{
	if (!Item) return;

	auto Mapping = GetItemMapping(Item);
	if (!Mapping) return;

	NotifyPreRemoveItem(*Mapping, Item, Item->GetQuantity());

	Mapping->ItemVisualLinked->RemoveFromParent();
	if (InventoryData.ItemCollectionLink)
	{
		InventoryData.ItemCollectionLink->RemoveItem(Item, GetAsContainerWidget());
	}

	NotifyPostRemoveItem();
}

FItemAddResult USlotbasedInventoryWidget::HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	//UE_LOG(LogTemp, Warning, TEXT("item Name is %s"), *ItemMoveData.SourceItem->GetName());
	//UE_LOG(LogTemp, Warning, TEXT("ItemCollectionLink number is %i"), ItemCollectionLink->GetItemLocations().Num());

	if (!ItemMoveData.SourceItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item is null. Nothing to add"));
		return FItemAddResult::AddedNone(FText::FromString("Item is null. Nothing to add"));
	}
	
	if (ItemMoveData.SourceInventory && ItemMoveData.SourceItem->GetQuantity() <= 0)
		UE_LOG(LogTemp, Warning, TEXT("item Quantity is %i"), ItemMoveData.SourceItem->GetQuantity());
	
	if(ItemMoveData.SourceInventory
		&& ItemMoveData.SourceInventory != ItemMoveData.TargetInventory
		&& !ItemMoveData.SourceInventory->GetInventorySettings().bCanReferenceItems
		&& InventorySettings.bUseReferences)
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   0, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}
	
	if (InventorySettings.bUseReferences)
		return HandleAddReferenceItem(ItemMoveData, bOnlyCheck);

	if (ItemMoveData.SourceInventory
		&& ItemMoveData.SourceInventory->GetInventorySettings().bUseReferences
		&& ItemMoveData.SourceInventory != ItemMoveData.TargetInventory
		&& ItemMoveData.TargetSlot)
	{
		if (bIsSlotEmpty(ItemMoveData.TargetSlot))
		{
			auto Mapping = InventoryData.ItemCollectionLink->FindItemMappingForItemInContainer(ItemMoveData.SourceItem, GetAsContainerWidget());
			if (Mapping)
			{
				if (!bOnlyCheck)
					ReplaceItem(ItemMoveData.SourceItem, ItemMoveData.TargetSlot);
				return FItemAddResult::Swapped(0, false, FText::FromString("Items successfully swapped."));
			}
		}
		
		return FItemAddResult::AddedNone(FText::FromString("Cant add Item by References"));
	}

	if (ItemMoveData.SourceInventory
		&& ItemMoveData.SourceInventory->GetInventorySettings().bUseReferences
		&& InventoryData.ItemCollectionLink->HasItemInContainer(ItemMoveData.SourceItem, GetAsContainerWidget()))
	{
		return HandleSwapOrAddItems(ItemMoveData, bOnlyCheck);
	}
	
	if (ItemMoveData.SourceInventory 
		&& ItemMoveData.TargetInventory
		&& ItemMoveData.SourceItemPivotSlot
		&& ItemMoveData.TargetSlot)
	{
		if (ItemMoveData.SourceInventory->GetInventorySettings().bUseReferences)
			return FItemAddResult::AddedNone(FText::FromString("Cant add Item by References"));
		
		auto TargetItem = InventoryData.ItemCollectionLink->GetItemFromSlot(ItemMoveData.TargetSlot->GetSlotData(), GetAsContainerWidget());
		if (ItemMoveData.SourceInventory == ItemMoveData.TargetInventory)
		{
			if (TargetItem && TargetItem->IsStackable() && UItemBase::bIsSameItems(TargetItem, ItemMoveData.SourceItem))
			{
				return TryAddStackableItem(ItemMoveData, false);
			}
			
			if (bIsSlotEmpty(ItemMoveData.TargetSlot))
			{
				
				if (!bOnlyCheck)
					ReplaceItem(ItemMoveData.SourceItem, ItemMoveData.TargetSlot);
				return FItemAddResult::Swapped(0, false, FText::FromString("Item successfully moved to an empty slot."));
			}

			if (!bOnlyCheck)
			{
				ReplaceItem(ItemMoveData.SourceItem, ItemMoveData.TargetSlot);
				ReplaceItem(TargetItem, ItemMoveData.SourceItemPivotSlot);
			}            
			return FItemAddResult::Swapped(0, false, FText::FromString("Items successfully swapped between slots."));
		}
		
		if (TargetItem && TargetItem->IsStackable() && UItemBase::bIsSameItems(TargetItem, ItemMoveData.SourceItem))
		{
			return TryAddStackableItem(ItemMoveData, bOnlyCheck);
		}

		if (!TargetItem && ItemMoveData.SourceItem->IsStackable())
		{
			return TryAddStackableItem(ItemMoveData, bOnlyCheck);
		}

		if (ItemMoveData.SourceInventory == ItemMoveData.TargetInventory)
			return HandleSwapOrAddItems(ItemMoveData, bOnlyCheck);
	}
	
	// non-stack
	if (!ItemMoveData.SourceItem->IsStackable())
	{
		// Check if input item has valid weight
		/*if (FMath::IsNearlyZero(ItemMoveData.SourceItem->GetItemSingleWeight()) || ItemMoveData.SourceItem->
			GetItemSingleWeight() < 0)
		{
			return FItemAddResult::AddedNone(FText::Format(FText::FromString("Item {0} has invalid weight"),
														   ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
		}*/

		// will the item weight overflow the weight capacity?
		if (InventorySettings.InventoryWeightCapacity >= 0)
		{
			if (InventoryData.InventoryTotalWeight + ItemMoveData.SourceItem->GetItemSingleWeight() > InventorySettings.InventoryWeightCapacity)
			{
				return FItemAddResult::AddedNone(FText::Format(
					FText::FromString("Item {0} would overflow weight limit"),
					ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
			}
		}

		return HandleNonStackableItems(ItemMoveData, bOnlyCheck);
	}

	// stack
	return TryAddStackableItem(ItemMoveData, bOnlyCheck);
}

FItemAddResult USlotbasedInventoryWidget::HandleNonStackableItems(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (!ItemMoveData.TargetSlot)
	{
		TObjectPtr<UInventorySlot> EmptySlot = nullptr;
		for (const auto InventorySlot : InventoryData.InventorySlots )
		{
			if (bIsSlotEmpty(InventorySlot))
			{
				EmptySlot = InventorySlot;
				break;
			}
		}

		if (EmptySlot == nullptr)
			return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory. No Empty slots"),
												   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));

		if (!bOnlyCheck)
		{
			FItemMapping Slots (EmptySlot->GetSlotData());
			AddNewItem(ItemMoveData, Slots, 1);
		}
		
		return FItemAddResult::AddedAll(1, false, FText::Format(
												FText::FromString("Successfully added {0} of {1} to inventory"),
												1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}

	if (bIsSlotEmpty(ItemMoveData.TargetSlot->GetSlotPosition()))
	{
		if (!bOnlyCheck)
		{
			FItemMapping Slots (ItemMoveData.TargetSlot->GetSlotData());
			AddNewItem(ItemMoveData, Slots, 1);
		}

		return FItemAddResult::AddedAll(1, false, FText::Format(
											FText::FromString("Successfully added {0} of {1} to inventory"),
											1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}

	return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
}

FItemAddResult USlotbasedInventoryWidget::TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	// will the item weight overflow the weight capacity?
	if (InventorySettings.InventoryWeightCapacity >= 0)
	{
		if (InventoryData.InventoryTotalWeight + ItemMoveData.SourceItem->GetItemSingleWeight() * ItemMoveData.
			SourceItem->GetQuantity() > InventorySettings.InventoryWeightCapacity)
		{
			return FItemAddResult::AddedNone(FText::Format(FText::FromString("Couldn't add {0} to inventory."),
			                                               ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
		}
	}
	
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

int32 USlotbasedInventoryWidget::HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck)
{
	int32 AmountToDistribute = RequestedAddAmount;
	int32 TotalAddedAmount = 0;

	if (!ItemMoveData.TargetSlot)
	{
		auto Sameitems = InventoryData.ItemCollectionLink->GetAllSameItemsInContainer(GetAsContainerWidget(), ItemMoveData.SourceItem);
		if (Sameitems.Num()> 0)
		{
			for (auto& Item : Sameitems)
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
		
		TObjectPtr<UInventorySlot> TargetSlot = nullptr;
		for (const auto InventorySlot : InventoryData.InventorySlots )
		{
			if (bIsSlotEmpty(InventorySlot))
			{
				TargetSlot = InventorySlot;
				break;
			}
		}

		if (TargetSlot == nullptr)
			return RequestedAddAmount - AmountToDistribute;

		const int32 AmountToAddToStack = FMath::Min(AmountToDistribute, ItemMoveData.SourceItem->GetQuantity());
		int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack, ItemMoveData.SourceItem->GetItemSingleWeight());
		
		if (bOnlyCheck)
			return AmountToDistribute;

		FItemMapping Slots(TargetSlot->GetSlotData());
		FItemMoveData NewItemMoveData;
		NewItemMoveData.SourceItem = ItemMoveData.SourceItem;
		NewItemMoveData.SourceItem->SetQuantity(ActualAmountToAdd);
		
		AddNewItem(NewItemMoveData, Slots, ActualAmountToAdd);
		return ActualAmountToAdd + TotalAddedAmount;
	}

	if (bIsSlotEmpty(ItemMoveData.TargetSlot))
	{
		const int32 AmountToAddToStack = ItemMoveData.SourceItem->GetQuantity();

		int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack, ItemMoveData.SourceItem->GetItemSingleWeight());

		FItemMapping Slots(ItemMoveData.TargetSlot->GetSlotData());
		FItemMoveData NewItemMoveData;
		NewItemMoveData.SourceItem = ItemMoveData.SourceItem;

		if (bOnlyCheck)
			return ActualAmountToAdd;
		
		AddNewItem(NewItemMoveData, Slots, ActualAmountToAdd);
		return ActualAmountToAdd;
	}
	else
	{
		auto ItemFromSlot = InventoryData.ItemCollectionLink->GetItemFromSlot(ItemMoveData.TargetSlot->GetSlotData(), GetAsContainerWidget());
		
		if (ItemFromSlot && ItemFromSlot == ItemMoveData.SourceItem)
			return 0;

		int32 AmountToAddToStack = FMath::Min(AmountToDistribute,
					ItemFromSlot->GetItemRef().ItemNumeraticData.MaxStackSize - ItemFromSlot->GetQuantity());
		int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack, ItemMoveData.SourceItem->GetItemSingleWeight());

		if (bOnlyCheck && ActualAmountToAdd > 0)
			return ActualAmountToAdd;
		
		InsertToStackItem(ItemFromSlot, ActualAmountToAdd);
		AmountToDistribute -= ActualAmountToAdd;
		return ActualAmountToAdd;
	}
}

FItemAddResult USlotbasedInventoryWidget::HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (ItemMoveData.TargetSlot == nullptr)
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));

	if (bIsSlotEmpty(ItemMoveData.TargetSlot))
	{
		if (InventoryData.ItemCollectionLink->FindItemMappingForItemInContainer(ItemMoveData.SourceItem, GetAsContainerWidget()))
		{
			if (!bOnlyCheck)
				ReplaceItem(ItemMoveData.SourceItem, ItemMoveData.TargetSlot);
			return FItemAddResult::Swapped(0, true, FText::FromString("Item successfully moved to an empty slot."));
		}
		
		if (ItemMoveData.SourceInventory == this)
		{
			return FItemAddResult::AddedAll(1, true, FText::Format(
			FText::FromString("Successfully added {0} to inventory as a reference"),
			ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
		}
		
		FItemMapping Slots;
		Slots.ItemSlotDatas.Add(ItemMoveData.TargetSlot->GetSlotData());
		if (!bOnlyCheck)
			AddNewItem(ItemMoveData, Slots, ItemMoveData.SourceItem->GetQuantity());

		return FItemAddResult::AddedAll(1, true, FText::Format(
			FText::FromString("Successfully added {0} to inventory as a reference"),
			ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}

	if (ItemMoveData.SourceInventory == this)
	{
		auto TarItem = InventoryData.ItemCollectionLink->GetItemFromSlot(ItemMoveData.TargetSlot->GetSlotData(), GetAsContainerWidget());
		if (bOnlyCheck)
			return FItemAddResult::Swapped(0, true, FText::FromString("Items successfully swapped."));
		
		ReplaceItem(ItemMoveData.SourceItem, ItemMoveData.TargetSlot);
		ReplaceItem(TarItem, ItemMoveData.SourceItemPivotSlot);
		return FItemAddResult::Swapped(0, true, FText::FromString("Items successfully swapped."));
	}

	if (!bOnlyCheck)
	{
		HandleRemoveItemFromContainer(InventoryData.ItemCollectionLink->GetItemFromSlot(ItemMoveData.TargetSlot->GetSlotData(), GetAsContainerWidget()));

		FItemMapping Slots;
		Slots.ItemSlotDatas.Add(ItemMoveData.TargetSlot->GetSlotData());
		AddNewItem(ItemMoveData, Slots, ItemMoveData.SourceItem->GetQuantity());

		return FItemAddResult::AddedAll(1, true, FText::Format(
			FText::FromString("Successfully added {0} to inventory as a reference"),
			ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}
	
	
	return FItemAddResult::AddedAll(1, true, FText::Format(
			FText::FromString("Successfully added {0} to inventory as a reference"),
			ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
}

FItemAddResult USlotbasedInventoryWidget::HandleSwapOrAddItems(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (ItemMoveData.SourceInventory->GetInventorySettings().bCanReferenceItems)
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Cannot add '{0}' to the inventory as references are enabled."),
											   ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));

	auto TarItem =InventoryData.ItemCollectionLink->GetItemFromSlot(ItemMoveData.TargetSlot->GetSlotData(), GetAsContainerWidget());
	if (ItemMoveData.SourceInventory != this && TarItem)
	{
		if (bOnlyCheck)
			return FItemAddResult::AddedAll(0, false, 
			FText::FromString("Successfully added to inventory"));
		
		FItemMoveData ItemMoveData2;
		ItemMoveData2.SourceInventory = this;
		ItemMoveData2.TargetInventory = ItemMoveData.SourceInventory;
		ItemMoveData2.SourceItem = TarItem;
		ItemMoveData2.SourceItemPivotSlot = ItemMoveData.TargetSlot;
		ItemMoveData2.TargetSlot = ItemMoveData.SourceItemPivotSlot;
		
		auto Result1 = HandleAddItem(ItemMoveData, true);
		auto Result2 = ItemMoveData2.TargetInventory->HandleAddItem(ItemMoveData2, true);
		if (Result1.OperationResult == EItemAddResult::IAR_AllItemAdded && Result2.OperationResult == EItemAddResult::IAR_AllItemAdded)
		{
			ItemMoveData.SourceInventory->HandleRemoveItemFromContainer(ItemMoveData.SourceItem);
			HandleRemoveItemFromContainer(TarItem);

			HandleAddItem(ItemMoveData);
			ItemMoveData2.TargetInventory->HandleAddItem(ItemMoveData2);
			return FItemAddResult::Swapped(0, false, FText::FromString("Items successfully swapped between containers."));
		}
		return FItemAddResult::AddedNone(FText::FromString("Cannot be swapped between containers."));
	}

	if (ItemMoveData.SourceInventory == this)
	{		
		if (!TarItem && bOnlyCheck)
			return FItemAddResult::AddedAll(0, false, 
			FText::FromString("Successfully added to inventory"));
	}

	return TryAddStackableItem(ItemMoveData, bOnlyCheck);
}

void USlotbasedInventoryWidget::AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount)
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
		OccupiedSlots.InventoryContainerName = GetAsContainerWidget()->GetFName();
		OccupiedSlots.InventoryType = GetAsContainerWidget()->GetInventoryType();
		InventoryData.ItemCollectionLink->AddItem(FinalItem, OccupiedSlots);
	}

	NotifyAddItem(OccupiedSlots, FinalItem, ItemMoveData.SourceItem->GetQuantity());
}

void USlotbasedInventoryWidget::ReplaceItem(UItemBase* Item, UInventorySlot* NewSlot)
{
	if (!InventoryData.ItemCollectionLink)
	{
		return;
	}
	
	auto Mapping = InventoryData.ItemCollectionLink->FindItemMappingForItemInContainer(Item, GetAsContainerWidget());
	if (!Mapping)
		return;

	//Clear old slots
	auto OldSlots = Mapping->ItemSlotDatas;
	
	Mapping->ItemSlotDatas[0] = NewSlot->GetSlotData();

	for (auto ItemSlotData : Mapping->ItemSlotDatas)
	{
		UTexture2D* tempText= nullptr;
		auto ItemSlot = GetSlotByPosition(ItemSlotData.SlotPosition);
		if (ItemSlot && !bIsSlotEmpty(ItemSlotData.SlotPosition))
			ItemSlot->UpdateVisualWithTexture(tempText);
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("ReplaceItem done!"))
	ReplaceItemInPanel(*Mapping, Item);

	for (auto OldItemSlotData : OldSlots)
	{
		auto ItemSlot = GetSlotByPosition(OldItemSlotData.SlotPosition);
		if (ItemSlot && bIsSlotEmpty(OldItemSlotData.SlotPosition))
		{
			ItemSlot->ResetVisual();
		}
	}
}

FVector2D USlotbasedInventoryWidget::CalculateItemVisualPosition(FIntVector2 SlotPosition) const
{
	const float StepX = UISettings.SlotSize.X + SlotSpacing.Left + SlotSpacing.Right;
	const float StepY = UISettings.SlotSize.Y + SlotSpacing.Top + SlotSpacing.Bottom;
	
	float X = SlotPosition.X * StepX + SlotSpacing.Left;
	float Y = SlotPosition.Y * StepY + SlotSpacing.Top;

	return FVector2D(Y, X);
}

void USlotbasedInventoryWidget::AddItemToPanel( UItemBase* Item)
{
	auto Slots = GetItemMapping(Item);
	
	const FVector2D VisualPosition = CalculateItemVisualPosition(Slots->ItemSlotDatas[0].SlotPosition);

	if (!UISettings.InventoryItemVisualClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryItemVisualClass is not set in UISettings!"));
		return;
	}
	
	TObjectPtr<UInventoryItemWidget> ItemVisual = CreateWidget<UInventoryItemWidget>(GetAsContainerWidget(), UISettings.InventoryItemVisualClass);
	auto SlotInCanvas = ItemsVisualsPanel->AddChildToCanvas(ItemVisual);
	if (SlotInCanvas)
		SlotInCanvas->SetSize(FVector2D(UISettings.SlotSize.X * 1, UISettings.SlotSize.Y *  1));
	Slots->ItemVisualLinked = ItemVisual;

	for (auto ItemSlotData : Slots->ItemSlotDatas)
	{
		UTexture2D* tempText= nullptr;
		auto ItemSlot = GetSlotByPosition(ItemSlotData.SlotPosition);
		if(ItemSlot && !bIsSlotEmpty(ItemSlotData.SlotPosition))
		{
			ItemSlot->UpdateVisualWithTexture(tempText);
		}
	}

	FIntVector2 ItemSize =	FIntVector2(Item->GetItemRef().ItemNumeraticData.NumHorizontalSlots,
		Item->GetItemRef().ItemNumeraticData.NumVerticalSlots);
	ItemVisual->UpdateVisualSize(UISettings.SlotSize, ItemSize);
	ItemVisual->UpdateItemName(Item->GetItemRef().ItemTextData.Name);
	ItemVisual->UpdateQuantityText(Item->GetQuantity());
	ItemVisual->UpdateVisual(Item);
	
	//ItemVisual->SetPivotSlot(ItemPivotSlot);			
	SlotInCanvas->SetPosition(VisualPosition);

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

void USlotbasedInventoryWidget::ReplaceItemInPanel(FItemMapping& FromSlots, UItemBase* Item)
{
	if (!FromSlots.ItemVisualLinked) return;

	auto ItemPivotSlot =FromSlots.ItemSlotDatas[0];
	const FVector2D VisualPosition = CalculateItemVisualPosition(ItemPivotSlot.SlotPosition);

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(FromSlots.ItemVisualLinked->Slot);
	if (!CanvasSlot) return;

	CanvasSlot->SetPosition(VisualPosition);
}

void USlotbasedInventoryWidget::UpdateSlotInPanel(FItemMapping FromSlots, UItemBase* Item)
{
	if (!FromSlots.ItemVisualLinked || !Item)
		return;

	FromSlots.ItemVisualLinked->UpdateQuantityText(Item->GetQuantity());
	FromSlots.ItemVisualLinked->UpdateItemName(Item->GetItemRef().ItemTextData.Name);
	FromSlots.ItemVisualLinked->UpdateVisual(Item);
}

void USlotbasedInventoryWidget::RemoveItemFromPanel(FItemMapping* FromSlots, UItemBase* Item)
{
	if (!FromSlots->ItemVisualLinked || !Item)
		return;

	FromSlots->ItemVisualLinked->RemoveFromParent();

	for (auto ItemSlotData : FromSlots->ItemSlotDatas)
	{
		auto ItemSlot = GetSlotByPosition(ItemSlotData.SlotPosition);
		if (ItemSlot && !bIsSlotEmpty(ItemSlotData.SlotPosition))
		{
			ItemSlot->ResetVisual();
		}
	}
}

void USlotbasedInventoryWidget::NotifyAddItem(FItemMapping& FromSlots, UItemBase* NewItem, int32 ChangeQuantity)
{
	Super::NotifyAddItem(FromSlots, NewItem, ChangeQuantity);
	if (!FromSlots.ItemVisualLinked)
		AddItemToPanel(NewItem);
	else
		UpdateSlotInPanel(FromSlots, NewItem);
}

void USlotbasedInventoryWidget::NotifyPreRemoveItem(FItemMapping& FromSlots, UItemBase* RemovedItem, int32 RemoveQuantity) 
{
	Super::NotifyPreRemoveItem(FromSlots, RemovedItem, RemoveQuantity);
	if (RemoveQuantity >= RemovedItem->GetQuantity())
		RemoveItemFromPanel(&FromSlots, RemovedItem);
	else
		UpdateSlotInPanel(FromSlots, RemovedItem);
	
}

void USlotbasedInventoryWidget::CreateHighlightWidget()
{
	if (!UISettings.HighlightSlotWidgetClass)
	{
		UE_LOG(LogTemp, Log, TEXT("HighlightSlotWidgetClass is null"));
		return;
	}

	if (!HighlightVisualsPanel)
		return;

	HighlightWidgetPreview = CreateWidget<UHighlightSlotWidget>(GetAsContainerWidget(), UISettings.HighlightSlotWidgetClass);
	auto CanvasSlot = HighlightVisualsPanel->AddChild(HighlightWidgetPreview);
	if (!CanvasSlot)
	{
		return;
	}

	HighlightWidgetPreview->SetVisibility(ESlateVisibility::Collapsed);
}

void USlotbasedInventoryWidget::CreateTooltipWidget()
{
	if (!InventorySettings.bShowTooltips || !UISettings.ItemTooltipWidgetClass)
		return;
	
	InventoryData.ItemTooltipWidget = CreateWidget<UItemTooltipWidget>(this, UISettings.ItemTooltipWidgetClass);
	SetToolTip(InventoryData.ItemTooltipWidget);
	InventoryData.ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
	
}

FIntPoint USlotbasedInventoryWidget::CalculateGridPosition(const FGeometry& Geometry, const FVector2D& ScreenCursorPos) const
{
	if (!SlotsGridPanel) return FIntPoint(-1, -1);
	FVector2D LocalCursorPos = SlotsGridPanel->GetCachedGeometry().AbsoluteToLocal(ScreenCursorPos);
	
	if (ScrollBox)
	{
		LocalCursorPos.X += ScrollBox->GetScrollOffset();
		LocalCursorPos.Y += ScrollBox->GetScrollOffsetOfEnd();
	}

	if (bHasSlotSpacing)
	{
		const TArray<UWidget*>& Children = SlotsGridPanel->GetAllChildren();
		for (int32 i = 0; i < Children.Num(); ++i)
		{
			if (UWidget* Child = Children[i])
			{
				auto InvSlot = Cast<UInventorySlot>(Child);
				const FGeometry& SlotGeometry = Child->GetCachedGeometry();
				if (SlotGeometry.IsUnderLocation(ScreenCursorPos) && InvSlot)
				{
					int32 Row = InvSlot->GetSlotPosition().X;
					int32 Column = InvSlot->GetSlotPosition().Y;
					
					//UE_LOG(LogTemp, Log, TEXT("Row: %d, Column: %d"),Row, Column );
					return FIntPoint(Row, Column);
				}
			}
		}
		return FIntPoint(-1, -1);
	}
	else
	{
		int32 Column = FMath::FloorToInt(LocalCursorPos.X / (UISettings.SlotSize.X + SlotSpacing.Left + SlotSpacing.Right));
		int32 Row    = FMath::FloorToInt(LocalCursorPos.Y / (UISettings.SlotSize.Y + SlotSpacing.Top + SlotSpacing.Bottom));

		return FIntPoint(Row, Column);
	}
}

FReply USlotbasedInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	
	FVector2D ScreenCursorPos = InMouseEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);
	
	if (InMouseEvent.GetEffectingButton() == UISettings.ItemSelectKey)
	{
		if (GridPosition.X >= 0 && GridPosition.Y >= 0)
		{
			SlotUnderMouse = GetSlotByPosition(FIntVector2(GridPosition.X, GridPosition.Y));
		}
		if (!SlotUnderMouse || !InventoryData.ItemCollectionLink) return FReply::Unhandled();
		
		auto ItemInSlot = InventoryData.ItemCollectionLink->GetItemFromSlot(SlotUnderMouse->GetSlotData(), GetAsContainerWidget());
		if (!ItemInSlot) return FReply::Unhandled();

		if (UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>())
		{
			FItemMoveData ItemMoveData;
			ItemMoveData.SourceInventory = this;
			ItemMoveData.SourceItemPivotSlot = SlotUnderMouse;
			ItemMoveData.SourceItem = ItemInSlot;
			if (!ItemMoveData.SourceItem)
				return FReply::Unhandled();

			if (InventoryManager->GetInventoryModifierStates().bIsQuickGrabModifierActive)
			{
				InventoryManager->OnQuickTransferItem(ItemMoveData);

				return FReply::Unhandled();
			}
			if (InventoryManager->GetInventoryModifierStates().bIsGrabAllSameModifierActive)
			{
				auto SameItems = InventoryData.ItemCollectionLink->GetAllSameItemsInContainer(GetAsContainerWidget(), ItemMoveData.SourceItem);
				for (auto Item : SameItems)
				{
					ItemMoveData.SourceItem = Item;
					InventoryManager->OnQuickTransferItem(ItemMoveData);
				}
				
				return FReply::Unhandled();
			}
		}
		
		//TODO: Rewrite with Hit Testing
		
		auto Linked= InventoryData.ItemCollectionLink-> GetItemLinkedWidgetForSlot(SlotUnderMouse->GetSlotData());
		if (!Linked) return FReply::Unhandled();

		return Reply.Handled().DetectDrag(TakeWidget(), UISettings.ItemSelectKey);
	}

	if (InMouseEvent.GetEffectingButton() == UISettings.ItemUseKey && InventorySettings.bCanUseItems)
	{
		if (GridPosition.X >= 0 && GridPosition.Y >= 0)
		{
			SlotUnderMouse = GetSlotByPosition(FIntVector2(GridPosition.X, GridPosition.Y));
		}
		if (!SlotUnderMouse) return FReply::Unhandled();
		
		auto ItemInSlot = InventoryData.ItemCollectionLink->GetItemFromSlot(SlotUnderMouse->GetSlotData(), GetAsContainerWidget());
		if (!ItemInSlot) return FReply::Unhandled();

		if (HandleTradeModalOpening(ItemInSlot))
			return FReply::Handled();
		
		ItemInSlot->UseItem();
		
		FReply::Handled();
	}
	
	return FReply::Unhandled();
}

FReply USlotbasedInventoryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	FVector2D ScreenCursorPos = InMouseEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);

	if (bIsGridPositionValid(GridPosition))
	{
		SlotUnderMouse = GetSlotByPosition(FIntVector2(GridPosition.X, GridPosition.Y));
	}
	
	if (!SlotUnderMouse || !InventoryData.ItemCollectionLink) return FReply::Unhandled();
	auto ItemInSlot = InventoryData.ItemCollectionLink->GetItemFromSlot(SlotUnderMouse->GetSlotData(), GetAsContainerWidget());
	
	if (ItemInSlot && InventoryData.ItemTooltipWidget)
	{
		InventoryData.ItemTooltipWidget->SetTooltipData(ItemInSlot, this);
		InventoryData.ItemTooltipWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else if (InventoryData.ItemTooltipWidget)
		InventoryData.ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);

	return Reply;
}

void USlotbasedInventoryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                                     UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UInventoryItemWidget* DraggedWidget = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(), UISettings.DraggedWidgetClass);
	if (!DraggedWidget) return;
	DraggedWidget->SetVisibility(ESlateVisibility::Hidden);
	
	UItemDragDropOperation* DragItemDragDropOperation = NewObject<UItemDragDropOperation>();
	DragItemDragDropOperation->DefaultDragVisual = DraggedWidget;
	DragItemDragDropOperation->Pivot = EDragPivot::CenterCenter;

	DragItemDragDropOperation->ItemMoveData.SourceItem = InventoryData.ItemCollectionLink->GetItemFromSlot(SlotUnderMouse->GetSlotData(), GetAsContainerWidget());
	DragItemDragDropOperation->ItemMoveData.SourceInventory = this;
	DragItemDragDropOperation->ItemMoveData.SourceItemPivotSlot = SlotUnderMouse;

	auto ShowDragVisual = [DraggedWidget]()
	{
		DraggedWidget->SetVisibility(ESlateVisibility::Visible);
	};
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	const FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda(ShowDragVisual);
	FTimerHandle TimerHandle;
	TimerManager.SetTimer(TimerHandle, TimerDelegate, 0.125f, false);

	OutOperation = DragItemDragDropOperation;

	if (!HighlightWidgetPreview)
		CreateHighlightWidget();
	if (HighlightWidgetPreview)
		HighlightWidgetPreview->SetVisibility(ESlateVisibility::Visible);
}

void USlotbasedInventoryWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

	if (!HighlightWidgetPreview)
		CreateHighlightWidget();
	if (HighlightWidgetPreview)
		HighlightWidgetPreview->SetVisibility(ESlateVisibility::Visible);
}

void USlotbasedInventoryWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	if (HighlightWidgetPreview)
		HighlightWidgetPreview->SetVisibility(ESlateVisibility::Collapsed);
}


bool USlotbasedInventoryWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                            UDragDropOperation* InOperation)
{
	if (!SlotsGridPanel) return false;
	
	FVector2D ScreenCursorPos = InDragDropEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);
	
	if (bIsGridPositionValid(GridPosition))
	{
		//UE_LOG(LogTemp, Log, TEXT("Column: %d, Row: %d"), GridPosition.X, GridPosition.Y);

		if (!HighlightWidgetPreview)
			return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
		
		const FVector2D VisualPosition = CalculateItemVisualPosition(FIntVector2(GridPosition.X, GridPosition.Y));
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(HighlightWidgetPreview->Slot);
		CanvasSlot->SetSize(FVector2D(UISettings.SlotSize.X * 1, UISettings.SlotSize.Y *  1));
		CanvasSlot->SetPosition(VisualPosition);

		auto DragOp = Cast<UItemDragDropOperation>(InOperation);
		auto TargetSlot = GetSlotByPosition(FIntVector2(GridPosition.X, GridPosition.Y));
		DragOp->ItemMoveData.TargetInventory = this;
		DragOp->ItemMoveData.TargetSlot = TargetSlot;

		auto Result = HandleAddItem(DragOp->ItemMoveData, true);
		switch (Result.OperationResult)
		{
		case EItemAddResult::IAR_AllItemAdded:
			HighlightWidgetPreview->SetHighlightState(EHighlightState::Allowed);
			break;
		case EItemAddResult::IAR_NoItemAdded:
			HighlightWidgetPreview->SetHighlightState(EHighlightState::NotAllowed);
			break;
		case EItemAddResult::IAR_PartialAmountItemAdded:
			HighlightWidgetPreview->SetHighlightState(EHighlightState::Partial);
			break;
		case EItemAddResult::IAR_ItemSwapped:
			HighlightWidgetPreview->SetHighlightState(EHighlightState::Allowed);
			break;
		}
	}
	
	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

bool USlotbasedInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (HighlightWidgetPreview)
		HighlightWidgetPreview->SetVisibility(ESlateVisibility::Collapsed);
	
	if (!InOperation || !SlotsGridPanel) return false;
	
	FVector2D ScreenCursorPos = InDragDropEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);
	
	//UE_LOG(LogTemp, Log, TEXT("Row: %d, Column: %d"),  GridPosition.X,  GridPosition.Y);
	if (bIsGridPositionValid(GridPosition))
	{
		auto DragOp = Cast<UItemDragDropOperation>(InOperation);
		auto TargetSlot = GetSlotByPosition(FIntVector2(GridPosition.X, GridPosition.Y));
		DragOp->ItemMoveData.TargetInventory = this;
		DragOp->ItemMoveData.TargetSlot = TargetSlot;

		if (OnItemDroppedDelegate.IsBound())
			OnItemDroppedDelegate.Broadcast(DragOp->ItemMoveData);

		return true;
	}
	
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
