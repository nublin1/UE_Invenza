//  Nublin Studio 2025 All Rights Reserved.

#include "UI/Inventory/BaseInventoryWidget.h"

#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/UIManagerComponent.h"
#include "ActorComponents/Items/itemBase.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "Slate/SObjectWidget.h"
#include "UI/Item/InventoryItemWidget.h"

UBaseInventoryWidget::UBaseInventoryWidget(): SlotsGridPanel(nullptr), InventoryWeightCapacity(0)
{
}

void UBaseInventoryWidget::InitializeInventory()
{
	UpdateWeightInfo();
}

void UBaseInventoryWidget::ReDrawAllItems()
{
	if (!ItemCollectionLink || !ItemsVisualsPanel)
		return;

	ItemsVisualsPanel->ClearChildren();

	auto AllItems =ItemCollectionLink->GetAllItemsByContainer(this);
	for (auto Item : AllItems)
	{
		AddItemToPanel(Item);
	}
}

void UBaseInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	OnVisibilityChanged.AddDynamic(this, &UBaseInventoryWidget::HandleVisibilityChanged);
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UBaseInventoryWidget::InitSlots);
}

void UBaseInventoryWidget::InitSlots()
{
	if (!SlotsGridPanel)
		return;
	
	TArray<TObjectPtr<UBaseInventorySlot>> NewInvSlots;
	const int32 NumChildren = SlotsGridPanel->GetChildrenCount();

	for (int32 i = 0; i < NumChildren; ++i)
	{
		if (UWidget* ChildWidget = SlotsGridPanel->GetChildAt(i))
		{
			auto WClass = ChildWidget->GetClass();
			if (WClass->IsChildOf(UBaseInventorySlot::StaticClass()))
			{
				if (auto HotbarSlot = Cast<UBaseInventorySlot>(ChildWidget))
				{
					NewInvSlots.Add(HotbarSlot);
				}
			}
		}
	}

	for (int32 i = 0; i < NewInvSlots.Num(); ++i)
	{
		if (const UWidget* ChildWidget = SlotsGridPanel->GetChildAt(i))
		{
			const UUniformGridSlot* UniSlot = Cast<UUniformGridSlot>(ChildWidget->Slot);
			if (UniSlot->GetRow() >= NumberOfRows)
				NumberOfRows = UniSlot->GetRow() + 1;
			if (UniSlot->GetColumn() >= NumberOfColumns)
				NumberOfColumns = UniSlot->GetColumn() + 1;

			NewInvSlots[i]->SetSlotPosition(FIntVector2( UniSlot->GetColumn(), UniSlot->GetRow()));
		}
	}
	
	InventorySlots = NewInvSlots;
}

void UBaseInventoryWidget::HandleVisibilityChanged(ESlateVisibility NewVisibility)
{
	if (NewVisibility == ESlateVisibility::Visible)
	{
		
	}
}

FItemMapping* UBaseInventoryWidget::GetItemMapping(UItemBase* Item)
{	
	if (!Item)
	{
		return nullptr;
	}
	auto Mapping = ItemCollectionLink->FindItemMappingForItemInContainer(Item, this);
	return Mapping;
}

UBaseInventorySlot* UBaseInventoryWidget::GetSlotByPosition(FIntVector2 SlotPosition)
{
	for (auto& Elem : InventorySlots)
	{
		if (Elem->GetSlotPosition() == SlotPosition)
			return Elem;
		
	}

	return nullptr;
}

bool UBaseInventoryWidget::bIsSlotEmpty(const FIntVector2 SlotPosition)
{
	auto BusySlots = ItemCollectionLink->CollectOccupiedSlotsByContainer(this);
	for (const auto InvSlot : BusySlots)
	{
		if (InvSlot->GetSlotPosition() == SlotPosition)
			return false;
	}
	
	return true;
}

bool UBaseInventoryWidget::bIsSlotEmpty(const UBaseInventorySlot* SlotCheck)
{
	auto BusySlots = ItemCollectionLink->CollectOccupiedSlotsByContainer(this);
	for (auto InvSlot : BusySlots)
	{
		if (InvSlot == SlotCheck)
			return false;
	}
	return true;
}

void UBaseInventoryWidget::HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity)
{
	if (!Item) return;

	Item->SetQuantity(Item->GetQuantity() - RemoveQuantity);
	
	auto Slots = GetItemMapping(Item);
	if (Slots)
		NotifyUpdateItem(Slots, Item);
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to find occupied slots for item %s"), *Item->GetName());
	}
}

void UBaseInventoryWidget::HandleRemoveItemFromContainer(UItemBase* Item)
{
	if (!Item) return;

	auto Mapping = GetItemMapping(Item);
	if (!Mapping) return;

	NotifyRemoveItem(*Mapping, Item);

	Mapping->ItemVisualLinked->RemoveFromParent();
	if (ItemCollectionLink)
	{
		ItemCollectionLink->RemoveItem(Item, this);
	}

	UpdateWeightInfo();
}

FItemAddResult UBaseInventoryWidget::HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	if(ItemMoveData.SourceInventory && !ItemMoveData.SourceInventory->GetCanReferenceItems() && bUseReferences)
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   0, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}
	
	if (bUseReferences)
		return HandleAddReferenceItem(ItemMoveData);

	if (ItemMoveData.SourceInventory
		&& ItemMoveData.SourceInventory->GetIsUseReference()
		&& ItemCollectionLink->HasItemInContainer(ItemMoveData.SourceItem, this))
	{
		return HandleSwapOrAddItems(ItemMoveData, bOnlyCheck);
	}
	
	if (ItemMoveData.SourceInventory 
		&& ItemMoveData.TargetInventory
		&& ItemMoveData.SourceItemPivotSlot)
	{
		auto TargetItem = ItemCollectionLink->GetItemFromSlot(ItemMoveData.TargetSlot, this);
		if (ItemMoveData.SourceInventory == ItemMoveData.TargetInventory)
		{
			if (TargetItem && TargetItem->IsStackable() && UItemBase::bIsSameitems(TargetItem, ItemMoveData.SourceItem))
			{
				return TryAddStackableItem(ItemMoveData, false);
			}
            
			if (bIsSlotEmpty(ItemMoveData.TargetSlot))
			{
				ReplaceItem(ItemMoveData.SourceItem, ItemMoveData.TargetSlot);
				return FItemAddResult::Swapped(0, false, FText::FromString("Item successfully moved to an empty slot."));
			}
            
			ReplaceItem(ItemMoveData.SourceItem, ItemMoveData.TargetSlot);
			ReplaceItem(TargetItem, ItemMoveData.SourceItemPivotSlot);
            
			return FItemAddResult::Swapped(0, false, FText::FromString("Items successfully swapped between slots."));
		}
		
		if (TargetItem && TargetItem->IsStackable() && UItemBase::bIsSameitems(TargetItem, ItemMoveData.SourceItem))
		{
			return TryAddStackableItem(ItemMoveData, false);
		}
        
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
		if (InventoryWeightCapacity >= 0)
		{
			if (InventoryTotalWeight + ItemMoveData.SourceItem->GetItemSingleWeight() > InventoryWeightCapacity)
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

FItemAddResult UBaseInventoryWidget::HandleNonStackableItems(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (!ItemMoveData.TargetSlot)
	{
		TObjectPtr<UBaseInventorySlot> EmptySlot = nullptr;
		for (const auto InventorySlot : InventorySlots)
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
			FItemMapping Slots (EmptySlot);
			AddNewItem(ItemMoveData, Slots);
		}
		
		return FItemAddResult::AddedAll(1, false, FText::Format(
												FText::FromString("Successfully added {0} of {1} to inventory"),
												1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}

	if (bIsSlotEmpty(ItemMoveData.TargetSlot->GetSlotPosition()))
	{
		if (!bOnlyCheck)
		{
			FItemMapping Slots (ItemMoveData.TargetSlot);
			AddNewItem(ItemMoveData, Slots);
		}

		return FItemAddResult::AddedAll(1, false, FText::Format(
											FText::FromString("Successfully added {0} of {1} to inventory"),
											1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}

	return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
}

FItemAddResult UBaseInventoryWidget::TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	// will the item weight overflow the weight capacity?
	if (InventoryWeightCapacity >= 0)
	{
		if (InventoryTotalWeight + ItemMoveData.SourceItem->GetItemSingleWeight() * ItemMoveData.SourceItem->GetQuantity() > InventoryWeightCapacity)
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

int32 UBaseInventoryWidget::HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck)
{
	int32 AmountToDistribute = RequestedAddAmount;
	
	auto CalculateActualAmountToAdd = [&](int32 InAmountToAdd) -> int32
	{
		if (InventoryWeightCapacity >= 0)
		{
			const int32 WeightLimitAddAmount = InventoryWeightCapacity - InventoryTotalWeight;
			int32 MaxItemsThatFit = WeightLimitAddAmount / ItemMoveData.SourceItem->GetItemSingleWeight();
			return FMath::Min(MaxItemsThatFit, InAmountToAdd);
		}
		return InAmountToAdd;
	};

	if (!ItemMoveData.TargetSlot)
	{
		auto Sameitems = ItemCollectionLink->GetAllSameItemsInContainer(this, ItemMoveData.SourceItem);
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
				int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack);

				InsertToStackItem(Item, ActualAmountToAdd);
				AmountToDistribute -= ActualAmountToAdd;
			}
		}

		if (AmountToDistribute<=0) return RequestedAddAmount;
		
		TObjectPtr<UBaseInventorySlot> TargetSlot = nullptr;
		for (const auto InventorySlot : InventorySlots)
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
		int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack);
		
		if (bOnlyCheck)
			return AmountToDistribute;

		FItemMapping Slots(TargetSlot);
		FItemMoveData NewItemMoveData;
		NewItemMoveData.SourceItem = ItemMoveData.SourceItem;
		NewItemMoveData.SourceItem->SetQuantity(ActualAmountToAdd);
		
		AddNewItem(NewItemMoveData, Slots);
		ItemMoveData.SourceItem->SetQuantity(ItemMoveData.SourceItem->GetQuantity() - ActualAmountToAdd);
		return RequestedAddAmount - AmountToDistribute;
	}

	if (bIsSlotEmpty(ItemMoveData.TargetSlot))
	{
		const int32 AmountToAddToStack = ItemMoveData.SourceItem->GetQuantity();

		int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack);

		FItemMapping Slots(ItemMoveData.TargetSlot);
		FItemMoveData NewItemMoveData;
		NewItemMoveData.SourceItem = ItemMoveData.SourceItem;
		NewItemMoveData.SourceItem->SetQuantity(ActualAmountToAdd);
		
		AddNewItem(NewItemMoveData, Slots);
		ItemMoveData.SourceItem->SetQuantity(ItemMoveData.SourceItem->GetQuantity() - ActualAmountToAdd);
		return ActualAmountToAdd;
	}
	else
	{
		auto ItemFromSlot = ItemCollectionLink->GetItemFromSlot(ItemMoveData.TargetSlot, this);
		
		if (ItemFromSlot && ItemFromSlot == ItemMoveData.SourceItem)
			return 0;

		int32 AmountToAddToStack = FMath::Min(AmountToDistribute,
					ItemFromSlot->GetItemRef().ItemNumeraticData.MaxStackSize - ItemFromSlot->GetQuantity());
		int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack);

		if (bOnlyCheck && ActualAmountToAdd > 0)
			return ActualAmountToAdd;
		
		InsertToStackItem(ItemFromSlot, ActualAmountToAdd);
		AmountToDistribute -= ActualAmountToAdd;
		ItemMoveData.SourceInventory->HandleRemoveItem(ItemMoveData.SourceItem, ActualAmountToAdd);
		return RequestedAddAmount - AmountToDistribute;
	}
}

FItemAddResult UBaseInventoryWidget::HandleAddReferenceItem(FItemMoveData& ItemMoveData)
{
	if (ItemMoveData.TargetSlot == nullptr)
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));

	if (bIsSlotEmpty(ItemMoveData.TargetSlot))
	{
		if (ItemCollectionLink->FindItemMappingForItemInContainer(ItemMoveData.SourceItem, this))
		{
			ReplaceItem(ItemMoveData.SourceItem, ItemMoveData.TargetSlot);
			return FItemAddResult::Swapped(0, false, FText::FromString("Item successfully moved to an empty slot."));
		}
		
		if (ItemMoveData.SourceInventory == this)
		{
			return FItemAddResult::AddedAll(1, true, FText::Format(
			FText::FromString("Successfully added {0} to inventory as a reference"),
			ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
		}
		
		FItemMapping Slots;
		Slots.ItemSlots.Add(ItemMoveData.TargetSlot);
		AddNewItem(ItemMoveData, Slots);

		return FItemAddResult::AddedAll(1, true, FText::Format(
			FText::FromString("Successfully added {0} to inventory as a reference"),
			ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}

	
	return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
}

FItemAddResult UBaseInventoryWidget::HandleSwapOrAddItems(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (ItemMoveData.SourceInventory->bUseReferences)
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Cannot add '{0}' to the inventory as references are enabled."),
											   ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));

	auto TarItem =ItemCollectionLink->GetItemFromSlot(ItemMoveData.TargetSlot, this);
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


	return TryAddStackableItem(ItemMoveData, false);
}

void UBaseInventoryWidget::AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots)
{
	TObjectPtr<UItemBase> FinalItem;
	if (bUseReferences)
	{
		FinalItem = ItemMoveData.SourceItem;
	}
	else
	{
		FinalItem = NewObject<UItemBase>(this);
		FinalItem->SetQuantity(ItemMoveData.SourceItem->GetQuantity());
		FinalItem->SetItemRef(ItemMoveData.SourceItem->GetItemRef());
	}
	
	if (ItemCollectionLink)
	{
		OccupiedSlots.BaseInventoryWidgetLink = this;
		ItemCollectionLink->AddItem(FinalItem, OccupiedSlots);
	}

	NotifyAddItem(OccupiedSlots, FinalItem);
}

void UBaseInventoryWidget::ReplaceItem(UItemBase* Item, UBaseInventorySlot* NewSlot)
{
	if (!ItemCollectionLink)
	{
		return;
	}
	
	auto Mapping = ItemCollectionLink->FindItemMappingForItemInContainer(Item, this);
	if (!Mapping)
		return;
	
	Mapping->ItemSlots[0] = NewSlot;

	//UE_LOG(LogTemp, Warning, TEXT("ReplaceItem done!"))

	ReplaceItemInPanel(*Mapping, Item);
}

void UBaseInventoryWidget::InsertToStackItem(UItemBase* Item, int32 AddQuantity)
{
	Item->SetQuantity(Item->GetQuantity() + AddQuantity);
	auto Slots = GetItemMapping(Item);
	if (Slots)
		NotifyUpdateItem(Slots, Item);
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to find occupied slots for item %s"), *Item->GetName());
	}
}

FVector2D UBaseInventoryWidget::CalculateItemVisualPosition(FIntVector2 SlotPosition, FIntVector2 ItemSize) const
{
	float X = SlotPosition.X * UISettings.SlotSize.X;
	float Y = SlotPosition.Y * UISettings.SlotSize.Y;

	return FVector2D(X, Y);
}

void UBaseInventoryWidget::AddItemToPanel( UItemBase* Item)
{
	auto Slots = GetItemMapping(Item);

	const UBaseInventorySlot* ItemPivotSlot =Slots->ItemSlots[0];
	const FVector2D VisualPosition = CalculateItemVisualPosition(ItemPivotSlot->GetSlotPosition(), Item->GetOccupiedSlots());

	if (!UISettings.InventoryItemVisualClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryItemVisualClass is not set in UISettings!"));
		return;
	}
	
	TObjectPtr<UInventoryItemWidget> ItemVisual = CreateWidget<UInventoryItemWidget>(this, UISettings.InventoryItemVisualClass);
	auto SlotInCanvas = ItemsVisualsPanel->AddChildToCanvas(ItemVisual);
	if (SlotInCanvas)
		SlotInCanvas->SetSize(FVector2D(UISettings.SlotSize.X * 1, UISettings.SlotSize.Y *  1));

	Slots->ItemVisualLinked = ItemVisual;

	ItemVisual->UpdateVisualSize(UISettings.SlotSize, FIntVector2(1, 1));
	ItemVisual->UpdateItemName(Item->GetItemRef().ItemTextData.Name);
	ItemVisual->UpdateQuantityText(Item->GetQuantity());
	ItemVisual->UpdateVisual(Item);
	
	//ItemVisual->SetPivotSlot(ItemPivotSlot);			
	SlotInCanvas->SetPosition(VisualPosition);
}

void UBaseInventoryWidget::ReplaceItemInPanel(FItemMapping& FromSlots, UItemBase* Item)
{
	if (!FromSlots.ItemVisualLinked) return;

	const UBaseInventorySlot* ItemPivotSlot =FromSlots.ItemSlots[0];
	const FVector2D VisualPosition = CalculateItemVisualPosition(ItemPivotSlot->GetSlotPosition(), Item->GetOccupiedSlots());

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(FromSlots.ItemVisualLinked->Slot);
	if (!CanvasSlot) return;

	CanvasSlot->SetPosition(VisualPosition);
}

void UBaseInventoryWidget::UpdateSlotInPanel(FItemMapping* FromSlots, UItemBase* Item)
{
	if (!FromSlots->ItemVisualLinked || !Item)
		return;

	FromSlots->ItemVisualLinked->UpdateQuantityText(Item->GetQuantity());
	FromSlots->ItemVisualLinked->UpdateItemName(Item->GetItemRef().ItemTextData.Name);
	FromSlots->ItemVisualLinked->UpdateVisual(Item);
}

void UBaseInventoryWidget::RemoveItemFromPanel(FItemMapping* FromSlots, UItemBase* Item)
{
	if (!FromSlots->ItemVisualLinked || !Item)
		return;

	FromSlots->ItemVisualLinked->RemoveFromParent();
}

void UBaseInventoryWidget::UpdateWeightInfo()
{
	if (OnWightUpdatedDelegate.IsBound() && ItemCollectionLink)
	{
		InventoryTotalWeight = 0;
		auto AllItems = ItemCollectionLink->GetAllItemsByContainer(this);
		if (AllItems.IsEmpty())
		{
			OnWightUpdatedDelegate.Broadcast(0, InventoryWeightCapacity);
		}
		else
		{
			for (auto Item : AllItems)
			{
				InventoryTotalWeight += Item->GetQuantity() * Item->GetItemSingleWeight();
			}

			OnWightUpdatedDelegate.Broadcast(InventoryTotalWeight, InventoryWeightCapacity);
		}
	}
}

void UBaseInventoryWidget::NotifyAddItem(FItemMapping& FromSlots, UItemBase* NewItem)
{
	AddItemToPanel(NewItem);
	UpdateWeightInfo();
	OnAddItemDelegate.Broadcast(FromSlots, NewItem);
}

void UBaseInventoryWidget::NotifyUpdateItem(FItemMapping* FromSlots, UItemBase* NewItem)
{
	UpdateSlotInPanel(FromSlots, NewItem);
	UpdateWeightInfo();
	//OnItemUpdateDelegate.Broadcast(FromSlots, NewItem);
}

/*void UBaseInventoryWidget::NotifyUseSlot(UBaseInventorySlot* FromSlot)
{
	//auto Item = FindItemBySlot(FromSlot);

	//OnUseItemDelegate.Broadcast(FromSlot, Item);
}*/

void UBaseInventoryWidget::NotifyRemoveItem(FItemMapping& FromSlots, UItemBase* RemovedItem) 
{
	RemoveItemFromPanel(&FromSlots, RemovedItem);
	OnRemoveItemDelegate.Broadcast(FromSlots, RemovedItem);
}

FIntPoint UBaseInventoryWidget::CalculateGridPosition(const FGeometry& Geometry, const FVector2D& ScreenCursorPos) const
{
	if (!SlotsGridPanel) return FIntPoint(-1, -1);
	FVector2D LocalCursorPos = SlotsGridPanel->GetCachedGeometry().AbsoluteToLocal(ScreenCursorPos);
	
	if (ScrollBox)
	{
		LocalCursorPos.X += ScrollBox->GetScrollOffset();
		LocalCursorPos.Y += ScrollBox->GetScrollOffsetOfEnd();
	}

	int32 Column = FMath::FloorToInt(LocalCursorPos.X / UISettings.SlotSize.X);
	int32 Row = FMath::FloorToInt(LocalCursorPos.Y / UISettings.SlotSize.Y);
	
	return FIntPoint(Column, Row);
}

FReply UBaseInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		//TODO: Rewrite with Hit Testing
		
		FVector2D ScreenCursorPos = InMouseEvent.GetScreenSpacePosition();
		FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);

		if (GridPosition.X >= 0 && GridPosition.Y >= 0)
		{
			SelectedSlot = GetSlotByPosition(FIntVector2(GridPosition.X, GridPosition.Y));
			if (!SelectedSlot) return FReply::Unhandled();

			auto Linked= ItemCollectionLink-> GetItemLinkedWidgetForSlot(SelectedSlot);
			if (!Linked) return FReply::Unhandled();
			
			return Reply.Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
		}
	}
	
	return FReply::Unhandled();
}

void UBaseInventoryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	auto Manager = GetOwningPlayerPawn()->FindComponentByClass<UUIManagerComponent>();
	if (!Manager)
		return;

	UInventoryItemWidget* DraggedWidget = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(),Manager->GetUISettings().DraggedWidgetClass);
	if (!DraggedWidget) return;
	DraggedWidget->SetVisibility(ESlateVisibility::Hidden);
	
	UItemDragDropOperation* DragItemDragDropOperation = NewObject<UItemDragDropOperation>();
	DragItemDragDropOperation->DefaultDragVisual = DraggedWidget;
	DragItemDragDropOperation->Pivot = EDragPivot::CenterCenter;

	DragItemDragDropOperation->ItemMoveData.SourceItem = ItemCollectionLink->GetItemFromSlot(SelectedSlot, this);
	DragItemDragDropOperation->ItemMoveData.SourceInventory = this;
	DragItemDragDropOperation->ItemMoveData.SourceItemPivotSlot = SelectedSlot;

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

bool UBaseInventoryWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                            UDragDropOperation* InOperation)
{
	if (!SlotsGridPanel) return false;
	
	FVector2D ScreenCursorPos = InDragDropEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);
	
	if (GridPosition.X >= 0 && GridPosition.Y >= 0 && GridPosition.X<=NumberOfRows && GridPosition.Y<=NumberOfColumns)
	{
		//UE_LOG(LogTemp, Log, TEXT("Column: %d, Row: %d"), GridPosition.X, GridPosition.Y);
	}
	
	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

bool UBaseInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (!InOperation || !SlotsGridPanel) return false;
	
	FVector2D ScreenCursorPos = InDragDropEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);
	
	if (GridPosition.X >= 0 && GridPosition.Y >= 0 && GridPosition.X<=NumberOfRows && GridPosition.Y<=NumberOfColumns)
	{
		auto DragOp = Cast<UItemDragDropOperation>(InOperation);
		auto TargetSlot = GetSlotByPosition(FIntVector2(GridPosition.X, GridPosition.Y));
		DragOp->ItemMoveData.TargetInventory = this;
		DragOp->ItemMoveData.TargetSlot = TargetSlot;

		auto Result = HandleAddItem(DragOp->ItemMoveData, false);
		switch (Result.OperationResult)
		{
		case EItemAddResult::IAR_AllItemAdded:
			if (Result.bIsUsedReferences)
			{
				break;
			}
			if (DragOp->ItemMoveData.SourceInventory->GetItemCollection() == DragOp->ItemMoveData.TargetInventory->GetItemCollection())
			{
				DragOp->ItemMoveData.SourceInventory->HandleRemoveItemFromContainer(DragOp->ItemMoveData.SourceItem);
				break;
			}
			if (DragOp->ItemMoveData.SourceItem->GetQuantity()<= 0)
			{
				DragOp->ItemMoveData.SourceInventory->HandleRemoveItemFromContainer(DragOp->ItemMoveData.SourceItem);
				break;
			}
			
			break;
		case EItemAddResult::IAR_NoItemAdded:
			break;
		case EItemAddResult::IAR_PartialAmountItemAdded:
			break;
		case EItemAddResult::IAR_ItemSwapped:
			if (DragOp->ItemMoveData.SourceInventory->bUseReferences && DragOp->ItemMoveData.SourceInventory != this)
			{
				DragOp->ItemMoveData.SourceInventory->HandleRemoveItemFromContainer(DragOp->ItemMoveData.SourceItem);
			}
			break;
		}

		return true;
	}
	
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}


