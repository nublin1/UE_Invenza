//  Nublin Studio 2026 All Rights Reserved.

#include "UI/Inventory/SlotbasedInventoryWidget.h"

#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Data/Items/itemBase.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "DragDrop/ItemDragDropOperation.h"
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

void USlotbasedInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ItemFiltersPanel)
	{
		if (ItemFiltersPanel->GetSearchText())
			ItemFiltersPanel->GetSearchText()->OnTextChanged.AddDynamic(this, &USlotbasedInventoryWidget::SearchTextChanged);
		
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

void USlotbasedInventoryWidget::InitializeInventoryWidget()
{
	InitSlots();
	CreateTooltipWidget();
}

void USlotbasedInventoryWidget::BindDelegated()
{
	if (!SlotBasedInventoryRef)
	{
		return;
	}

	SlotBasedInventoryRef->OnAddItemDelegate.AddDynamic(this, &USlotbasedInventoryWidget::AddItemToPanel);
	SlotBasedInventoryRef->OnItemRemovedDelegate.AddDynamic(this, &USlotbasedInventoryWidget::RemoveItemFromPanel);
	SlotBasedInventoryRef->OnItemReplaceDelegate.AddDynamic(this, &USlotbasedInventoryWidget::ReplaceItemInPanel);
	SlotBasedInventoryRef->OnStackedItemDelegate.AddDynamic(this, &USlotbasedInventoryWidget::UpdateItem);
	SlotBasedInventoryRef->OnUnstackedItemDelegate.AddDynamic(this, &USlotbasedInventoryWidget::UpdateItem);
	SlotBasedInventoryRef->OnUseSlotDelegate.AddDynamic(this, &USlotbasedInventoryWidget::UsedItemInPanel);

	SlotBasedInventoryRef->OnWeightUpdatedDelegate.AddDynamic(this, &USlotbasedInventoryWidget::UpdateWeightInfo);
	SlotBasedInventoryRef->OnMoneyUpdatedDelegate.AddDynamic(this, &USlotbasedInventoryWidget::UpdateMoneyInfo);
}

void USlotbasedInventoryWidget::ReDrawAllItems()
{
	if (!SlotBasedInventoryRef || !ItemsVisualsPanel)
		return;

	ItemsVisualsPanel->ClearChildren();
	for (auto& InventorySlot : InventorySlots)
	{
		InventorySlot->ResetVisual();
	}

	auto InvId = SlotBasedInventoryRef->GetInventoryContainerID();

	auto AllItems = SlotBasedInventoryRef->GetItemCollectionLinked()->GetAllItemsByContainer(InvId);
	if (AllItems.IsEmpty())
		return;
	
	for (auto Item : AllItems)
	{
		auto Mapping = SlotBasedInventoryRef->GetItemCollectionLinked()->FindItemMappingByContainerName(Item, InvId);
		AddItemToPanel(*Mapping, Item);
	}
}

void USlotbasedInventoryWidget::RebuildSlots(int32 InRows, int32 InColumns)
{
	if (!SlotsGridPanel || !UISettings.DefaultSlotbasedInventorySlotClass)
	{
		return;
	}
	
	SlotsGridPanel->ClearChildren();
	InventorySlots.Empty();

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
			
			InventorySlots.Add(NewSlot);
		}
	}
	
	NumberRows  = InRows;
	NumColumns  = InColumns;
}

TArray<UInventorySlotData*> USlotbasedInventoryWidget::GetSlotData()
{
	TArray<UInventorySlotData*> Array;
		
	if (InventorySlots.IsEmpty())
		return Array;

	for (auto& InventorySlot : InventorySlots)
	{
		Array.Add(InventorySlot->GetSlotData());
	}

	return Array;
}

void USlotbasedInventoryWidget::SetInventoryBaseRef(UInventoryBase* NewInventoryRef)
{
	if (USlotbasedInventory* SlotInventory = Cast<USlotbasedInventory>(NewInventoryRef))
	{
		Super::SetInventoryBaseRef(NewInventoryRef);
		SlotBasedInventoryRef = SlotInventory;
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
					if (!InventorySlot->GetDefaultCellImage() && DefaultCellImage)
						InventorySlot->UpdateVisualWithTexture(DefaultCellImage);
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

			NewInvSlots[i]->SetSlotPosition(FIntPoint(UniSlot->GetRow(),  UniSlot->GetColumn()));
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
	InventorySlots = ConvertedSlots;
}

void USlotbasedInventoryWidget::ClearFilters()
{
	if (!SlotBasedInventoryRef)
		return;
	
	for (auto Item : SlotBasedInventoryRef->GetItemCollectionLinked()->GetAllItemsByContainer(SlotBasedInventoryRef->GetInventoryContainerID()))
	{
		auto ItemMapping = SlotBasedInventoryRef->GetItemCollectionLinked()->FindItemMappingByContainerName(Item, SlotBasedInventoryRef->GetInventoryContainerID());
		if (!ItemMapping)
			continue;

		if (ItemMapping->ItemVisualLinked)
		{
			ItemMapping->ItemVisualLinked->GetCoreCellWidget()->ResetBorderColor();
			ItemMapping->ItemVisualLinked->ChangeOpacity(1.0f);
		}
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
	
	RefreshFilteredItemsList();
}

void USlotbasedInventoryWidget::RefreshFilteredItemsList()
{
	if (ActiveFilters.Num() == 0)
	{
		ClearFilters();
		return;
	}

	auto Mappings = SlotBasedInventoryRef->GetItemCollectionLinked()->GetAllMappingsByContainer(SlotBasedInventoryRef->GetInventoryContainerID());
	if (Mappings.Num() == 0)
		return;
	
	for (auto& Mapping : Mappings)
	{
		Mapping.ItemVisualLinked->ChangeOpacity(ItemFiltersPanel->FilterOpacity);
		Mapping.ItemVisualLinked->GetCoreCellWidget()->ResetBorderColor();
	}
	
	for (auto ActiveCategory : ActiveFilters)
	{
		for (auto& Mapping : Mappings)
		{
			if (ItemFiltersPanel->bUseFilterColor)
			{
				Mapping.ItemVisualLinked->ChangeBorderColor(ItemFiltersPanel->ItemFilterBorderColor);
			}
			Mapping.ItemVisualLinked->ChangeOpacity(1.0f);
		}
	}
}

void USlotbasedInventoryWidget::SearchTextChanged(const FText& NewText)
{
	auto ItemCollection =SlotBasedInventoryRef->GetItemCollectionLinked();
	auto InvID = SlotBasedInventoryRef->GetInventoryContainerID();
	
	if (NewText.IsEmpty())
	{
		for (auto Item : ItemCollection->GetAllItemsByContainer(InvID))
		{
			auto ItemMapping = ItemCollection->FindItemMappingByContainerName(Item, InvID);
			if (!ItemMapping)
				continue;

			ItemMapping->ItemVisualLinked->GetCoreCellWidget()->ResetBorderColor();
			ItemMapping->ItemVisualLinked->ChangeOpacity(1.0f);

			RefreshFilteredItemsList();
		}
		return;
	}
	
	if (ItemFiltersPanel->IsSearchInFilteredSlots())
	{
		for (auto ActiveFilter : ActiveFilters)
		{
			for (auto Item : ItemCollection->GetAllItemsByCategory(ActiveFilter))
			{
				auto ItemMapping = ItemCollection->FindItemMappingByContainerName(Item, InvID);
				if (!ItemMapping)
					continue;

				FString StringName = Item->GetItemRef().ItemTextData.DisplayName.ToString();
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

	for (auto Item : ItemCollection->GetAllItemsByContainer(InvID))
	{
		auto ItemMapping = ItemCollection->FindItemMappingByContainerName(Item, InvID);
		if (!ItemMapping)
			continue;

		FString StringName = Item->GetItemRef().ItemTextData.DisplayName.ToString();
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

UInventorySlot* USlotbasedInventoryWidget::GetSlotByPosition(FIntPoint SlotPosition)
{
	for (auto& Elem : InventorySlots )
	{
		if (Elem->GetSlotPosition() == SlotPosition)
			return Elem;
	}

	return nullptr;
}

bool USlotbasedInventoryWidget::bIsGridPositionValid(FIntPoint& GridPosition)
{
	return GridPosition.X >= 0 && GridPosition.Y >= 0 && GridPosition.X<NumberRows && GridPosition.Y<NumColumns;
}

/*FItemAddResult USlotbasedInventoryWidget::HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
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
				return TryAddStackableItem(ItemMoveData, bOnlyCheck);
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
		}#1#

		// will the item weight overflow the weight capacity?
		if (InventorySettings.InventoryMaxWeightCapacity >= 0)
		{
			if (InventoryData.InventoryTotalWeight + ItemMoveData.SourceItem->GetItemSingleWeight() > InventorySettings.InventoryMaxWeightCapacity)
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

TObjectPtr<UInventorySlot> USlotbasedInventoryWidget::GetAvailableSlotForItem(UItemBase* Item)
{
	TObjectPtr<UInventorySlot> FreeSlot;

	for (int32 i = 0; i <= NumColumns; i++)
	{
		for (int32 j = 0; j <= NumberRows; j++)
		{
			auto CheckPos = FIntPoint(i, j);
			//UE_LOG(LogTemp, Log, TEXT("CheckPos %i and %i"),CheckPos.X, CheckPos.Y);
			if (bIsGridPositionValid(CheckPos) && bIsSlotEmpty(FIntVector2(CheckPos.X, CheckPos.Y)))
			{
				FreeSlot = GetSlotByPosition(FIntVector2(CheckPos.X, CheckPos.Y));
				if (!FreeSlot)
				{
					continue;
				}
				
				return FreeSlot;
			}
		}
	}

	return FreeSlot;
}

FItemAddResult USlotbasedInventoryWidget::HandleNonStackableItems(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (!ItemMoveData.TargetSlot)
	{
		TObjectPtr<UInventorySlot> EmptySlot = GetAvailableSlotForItem(ItemMoveData.SourceItem);

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
	if (InventorySettings.InventoryMaxWeightCapacity >= 0)
	{
		if (InventoryData.InventoryTotalWeight + ItemMoveData.SourceItem->GetItemSingleWeight() * ItemMoveData.
			SourceItem->GetQuantity() > InventorySettings.InventoryMaxWeightCapacity)
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
			return ActualAmountToAdd + TotalAddedAmount;

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

		if (!bOnlyCheck)
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
		Slots.OccupiedSlots.Add(ItemMoveData.TargetSlot->GetSlotData());
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
		Slots.OccupiedSlots.Add(ItemMoveData.TargetSlot->GetSlotData());
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
}*/

FVector2D USlotbasedInventoryWidget::CalculateItemVisualPosition(FIntPoint SlotPosition) const
{
	const float StepX = UISettings.SlotSize.X + SlotSpacing.Left + SlotSpacing.Right;
	const float StepY = UISettings.SlotSize.Y + SlotSpacing.Top + SlotSpacing.Bottom;
	
	float X = SlotPosition.X * StepX + SlotSpacing.Left;
	float Y = SlotPosition.Y * StepY + SlotSpacing.Top;

	return FVector2D(Y, X);
}

void USlotbasedInventoryWidget::AddItemToPanel(FItemMapping ItemSlots, UItemBase* Item)
{
	if (!Item)
		return;
	
	auto Slots = ItemSlots;

	if (Slots.OccupiedSlots.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemSlotDatas Is empty!"));
		return;
	}
	
	const FVector2D VisualPosition = CalculateItemVisualPosition(Slots.OccupiedSlots[0]->CellPosition);

	if (!UISettings.InventoryItemVisualClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("USlotbasedInventoryWidget::InventoryItemVisualClass is not set in UISettings!"));
		return;
	}
	
	TObjectPtr<UInventoryItemWidget> ItemVisual = CreateWidget<UInventoryItemWidget>(GetAsContainerWidget(), UISettings.InventoryItemVisualClass);
	auto SlotInCanvas = ItemsVisualsPanel->AddChildToCanvas(ItemVisual);
	if (SlotInCanvas)
		SlotInCanvas->SetSize(FVector2D(UISettings.SlotSize.X * 1, UISettings.SlotSize.Y *  1));
	Slots.ItemVisualLinked = ItemVisual;

	for (auto ItemSlotData : Slots.OccupiedSlots)
	{
		auto ItemSlot = GetSlotByPosition(ItemSlotData->CellPosition);
		if(ItemSlot)
		{
			if (bHideBackgroundWhenOccupied)
				ItemSlot->ClearVisual();
			else if (OccupiedCellImage)
				ItemSlot->UpdateVisualWithTexture(OccupiedCellImage);
		}
	}

	FIntPoint ItemSize = FIntPoint(Item->GetItemRef().ItemNumeraticData.InventoryHorizontalSlots,
		Item->GetItemRef().ItemNumeraticData.InventoryVerticalSlots);
	ItemVisual->UpdateVisualSize(UISettings.SlotSize, ItemSize);
	ItemVisual->UpdateItemName(Item->GetItemRef().ItemTextData.DisplayName);
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

void USlotbasedInventoryWidget::ReplaceItemInPanel(TArray<UInventorySlotData*> OldItemSlots, FItemMapping NewItemSlots, UItemBase* Item)
{
	if (!Item) return;

	for (auto ItemSlotData : OldItemSlots)
	{
		if (auto ItemSlot = GetSlotByPosition(ItemSlotData->CellPosition))
		{
			ItemSlot->ResetVisual();
		}
	}

	for (auto ItemSlotData : NewItemSlots)
	{
		auto ItemSlot = GetSlotByPosition(ItemSlotData->CellPosition);
		if(ItemSlot)
		{
			if (bHideBackgroundWhenOccupied)
				ItemSlot->ClearVisual();
			else if (OccupiedCellImage)
				ItemSlot->UpdateVisualWithTexture(OccupiedCellImage);
		}
	}

	const FVector2D NewVisualPosition = CalculateItemVisualPosition(NewItemSlots.OccupiedSlots[0]->CellPosition);

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(NewItemSlots.ItemVisualLinked->Slot))
	{
		CanvasSlot->SetPosition(NewVisualPosition);
	}
}

void USlotbasedInventoryWidget::UpdateItem(UItemBase* Item, int32 ChangedAmount)
{
	if (!Item) return;

	auto Mapping = SlotBasedInventoryRef->GetItemCollectionLinked()->FindItemMappingByContainerName(Item, SlotBasedInventoryRef->GetInventoryContainerID());
	if (!Mapping)
	{
		return;
	}

	Mapping->ItemVisualLinked->UpdateQuantityText(Item->GetQuantity());
}

void USlotbasedInventoryWidget::UpdateSlotInPanel(FItemMapping FromSlots, UItemBase* Item)
{
	if (!FromSlots.ItemVisualLinked || !Item)
		return;

	FromSlots.ItemVisualLinked->UpdateQuantityText(Item->GetQuantity());
	FromSlots.ItemVisualLinked->UpdateItemName(Item->GetItemRef().ItemTextData.DisplayName);
	FromSlots.ItemVisualLinked->UpdateVisual(Item);
}

void USlotbasedInventoryWidget::RemoveItemFromPanel(FItemMapping FromSlots, UItemBase* Item)
{
	if (!FromSlots.ItemVisualLinked || !Item)
		return;

	FromSlots.ItemVisualLinked->RemoveFromParent();

	for (auto ItemSlotData : FromSlots.OccupiedSlots)
	{
		if (auto ItemSlot = GetSlotByPosition(ItemSlotData->CellPosition))
		{
			ItemSlot->ResetVisual();
		}
	}
}

void USlotbasedInventoryWidget::UsedItemInPanel(UInventorySlotData* UsedSlot)
{
	if (!UsedSlot) return;

	for (auto Element : InventorySlots)
	{
		if (Element->GetSlotData() == UsedSlot)
		{
			// TODO
		}
	}
}

void USlotbasedInventoryWidget::UpdateWeightInfo(float InventoryTotalWeight)
{
	
}

void USlotbasedInventoryWidget::UpdateMoneyInfo(int32 InventoryTotalMoney)
{
	
}

void USlotbasedInventoryWidget::CreateHighlightWidget()
{
	if (!UISettings.HighlightSlotWidgetClass)
	{
		UE_LOG(LogTemp, Log, TEXT("USlotbasedInventoryWidget::HighlightSlotWidgetClass is null"));
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
	if (!SlotBasedInventoryRef)
		return;

	auto InvSettings = SlotBasedInventoryRef->GetInventorySettings();
	
	if (!InvSettings.bShowItemTooltips || !UISettings.ItemTooltipWidgetClass)
		return;
	
	ItemTooltipWidget = CreateWidget<UItemTooltipWidget>(this, UISettings.ItemTooltipWidgetClass);
	SetToolTip(ItemTooltipWidget);
	ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
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
	
	auto ItemCollection = SlotBasedInventoryRef->GetItemCollectionLinked();
	auto InvID = SlotBasedInventoryRef->GetInventoryContainerID();
	
	if (InMouseEvent.GetEffectingButton() == UISettings.ItemSelectKey)
	{
		if (GridPosition.X >= 0 && GridPosition.Y >= 0)
		{
			SlotUnderMouse = GetSlotByPosition(GridPosition);
		}
		if (!SlotUnderMouse || !ItemCollection) return FReply::Unhandled();
		
		auto ItemInSlot = ItemCollection->GetItemFromSlot(SlotUnderMouse->GetSlotData(), InvID);
		if (!ItemInSlot) return FReply::Unhandled();

		if (UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>())
		{
			FItemMoveData ItemMoveData;
			ItemMoveData.SourceInventory = SlotBasedInventoryRef;
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
				auto SameItems = ItemCollection->GetAllSameItemsInContainer(InvID, ItemMoveData.SourceItem);
				for (auto Item : SameItems)
				{
					ItemMoveData.SourceItem = Item;
					InventoryManager->OnQuickTransferItem(ItemMoveData);
				}
				
				return FReply::Unhandled();
			}
		}
		
		//TODO: Rewrite with Hit Testing
		
		//auto Linked = ItemCollection->GetItemLinkedWidgetForSlot(SlotUnderMouse->GetSlotData());
		//if (!Linked) return FReply::Unhandled();

		return Reply.Handled().DetectDrag(TakeWidget(), UISettings.ItemSelectKey);
	}

	if (InMouseEvent.GetEffectingButton() == UISettings.ItemUseKey)
	{
		if (GridPosition.X >= 0 && GridPosition.Y >= 0)
		{
			SlotUnderMouse = GetSlotByPosition(GridPosition);
		}
		if (!SlotUnderMouse) return FReply::Unhandled();
		
		auto ItemInSlot = ItemCollection->GetItemFromSlot(SlotUnderMouse->GetSlotData(), InvID);
		if (!ItemInSlot) return FReply::Unhandled();

		if (HandleTradeModalOpening(ItemInSlot))
			return FReply::Handled();

		if (SlotBasedInventoryRef->GetInventorySettings().bAllowItemUsage)
		{
			ItemInSlot->UseItem();
			FReply::Handled();
		}
	}
	
	return FReply::Unhandled();
}

FReply USlotbasedInventoryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	FVector2D ScreenCursorPos = InMouseEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);

	auto ItemCollection = SlotBasedInventoryRef->GetItemCollectionLinked();
	auto InvID = SlotBasedInventoryRef->GetInventoryContainerID();

	if (bIsGridPositionValid(GridPosition))
	{
		SlotUnderMouse = GetSlotByPosition(GridPosition);
	}
	
	if (!SlotUnderMouse || !ItemCollection) return FReply::Unhandled();
	auto ItemInSlot = ItemCollection->GetItemFromSlot(SlotUnderMouse->GetSlotData(), InvID);
	
	if (ItemInSlot && ItemTooltipWidget)
	{
		ItemTooltipWidget->SetTooltipData(ItemInSlot, this);
		ItemTooltipWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else if (ItemTooltipWidget)
		ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);

	return Reply;
}

void USlotbasedInventoryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                                     UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	auto ItemCollection = SlotBasedInventoryRef->GetItemCollectionLinked();
	auto InvID = SlotBasedInventoryRef->GetInventoryContainerID();

	auto DraggedWidget = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(), UISettings.DraggedWidgetClass);
	auto Item = ItemCollection->GetItemFromSlot(SlotUnderMouse->GetSlotData(), InvID);
	if (!DraggedWidget || !Item) return;

	DraggedWidget->AddToPlayerScreen(1);
	DraggedWidget->SetPositionInViewport(FVector2D(-10000, -10000));

	FIntVector2 ItemSize =	FIntVector2(Item->GetItemRef().ItemNumeraticData.InventoryHorizontalSlots,
		Item->GetItemRef().ItemNumeraticData.InventoryVerticalSlots);
	DraggedWidget->UpdateVisual(Item);
	
	DraggedWidget->SetVisibility(ESlateVisibility::Hidden);
	
	
	UItemDragDropOperation* DragItemDragDropOperation = NewObject<UItemDragDropOperation>();
	DragItemDragDropOperation->DefaultDragVisual = DraggedWidget;
	DragItemDragDropOperation->Pivot = EDragPivot::TopLeft;

	DragItemDragDropOperation->ItemMoveData.SourceItem = ItemCollection->GetItemFromSlot(SlotUnderMouse->GetSlotData(), InvID);
	DragItemDragDropOperation->ItemMoveData.SourceInventory = SlotBasedInventoryRef;
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
		
		const FVector2D VisualPosition = CalculateItemVisualPosition(GridPosition);
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(HighlightWidgetPreview->Slot);
		CanvasSlot->SetSize(FVector2D(UISettings.SlotSize.X * 1, UISettings.SlotSize.Y *  1));
		CanvasSlot->SetPosition(VisualPosition);

		auto DragOp = Cast<UItemDragDropOperation>(InOperation);
		auto TargetSlot = GetSlotByPosition(GridPosition);
		DragOp->ItemMoveData.TargetInventory = SlotBasedInventoryRef;
		DragOp->ItemMoveData.TargetSlot = TargetSlot;

		HighlightWidgetPreview->UpdateVisualWithTexture(DragOp->ItemMoveData.SourceItem->GetItemRef().ItemAssetData.Icon);

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
	else
	{
		if (HighlightWidgetPreview)
			HighlightWidgetPreview->SetVisibility(ESlateVisibility::Collapsed);
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
		auto TargetSlot = GetSlotByPosition(GridPosition);
		DragOp->ItemMoveData.TargetInventory = SlotBasedInventoryRef;
		DragOp->ItemMoveData.TargetSlot = TargetSlot;

		if (OnItemDroppedDelegate.IsBound())
			OnItemDroppedDelegate.Broadcast(DragOp->ItemMoveData);

		return true;
	}
	
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
