// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/BaseInventoryWidget.h"

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

void UBaseInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UBaseInventoryWidget::InitSlots);
	
	UpdateWeightInfo();
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

	int Temp_NumberOfRows = 0;
	int Temp_NumberOfColumns = 0;

	for (int32 i = 0; i < NewInvSlots.Num(); ++i)
	{
		if (const UWidget* ChildWidget = SlotsGridPanel->GetChildAt(i))
		{
			const UUniformGridSlot* UniSlot = Cast<UUniformGridSlot>(ChildWidget->Slot);
			if (UniSlot->GetRow() > Temp_NumberOfRows)
				Temp_NumberOfRows = UniSlot->GetRow();
			if (UniSlot->GetColumn() > Temp_NumberOfColumns)
				Temp_NumberOfColumns = UniSlot->GetColumn();

			NewInvSlots[i]->SetSlotPosition(FIntVector2( UniSlot->GetColumn(), UniSlot->GetRow()));
		}
	}
	
	InventorySlots = NewInvSlots;
}

UUserWidget* UBaseInventoryWidget::GetItemLinkedWidget(UBaseInventorySlot* _ItemSlot)
{
	for (auto& Elem : InventoryItemsMap)
	{
		for (const auto& ItemSlot : Elem.Value.ItemSlots)
		{
			if (ItemSlot == _ItemSlot)
				return Elem.Value.ItemVisualLinked;
		}
	}
	
	return nullptr;
}

FItemSlotMapping UBaseInventoryWidget::GetAllSlotsFromInventoryItemsMap()
{
	FItemSlotMapping AllPositions;

	for (auto& Elem : InventoryItemsMap)
	{
		for (const auto& ItemSlot : Elem.Value.ItemSlots)
		{
			AllPositions.ItemSlots.Add(ItemSlot);
		}
	}

	return AllPositions;
}

FItemSlotMapping* UBaseInventoryWidget::GetOccupiedSlots(UItemBase* Item)
{	
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetItemSlots: Item is null!"));
		return nullptr;
	}

	for (auto& Pair : InventoryItemsMap)
	{
		if (Pair.Key == Item)
			return &Pair.Value;
	}
	
	return nullptr;
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

TArray<UItemBase*> UBaseInventoryWidget::GetAllItems()
{
	TArray<UItemBase*> AllItems;
	InventoryItemsMap.GetKeys(AllItems);
	return AllItems;
}

TArray<UItemBase*> UBaseInventoryWidget::GetAllSameItems(UItemBase* TestItem)
{
	TArray<UItemBase*> AllItems;

	for (auto& Elem : InventoryItemsMap)
	{
		if (Elem.Key->GetItemName() == TestItem->GetItemName())
			AllItems.Add(Elem.Key);
	}

	return AllItems;
}

UItemBase* UBaseInventoryWidget::GetItemFromSlot(UBaseInventorySlot* TargetSlot)
{
	if (!TargetSlot)
		return nullptr;

	for (auto Pair  : InventoryItemsMap)
	{
		for (auto Element : Pair.Value.ItemSlots)
		{
			if (Element == TargetSlot)
				return Pair.Key;
		}
	}

	return nullptr;
		
}

bool UBaseInventoryWidget::bIsSlotEmpty(const FIntVector2 SlotPosition)
{
	FItemSlotMapping BusySlots = GetAllSlotsFromInventoryItemsMap();
	for (const auto InvSlot : BusySlots.ItemSlots)
	{
		if (InvSlot->GetSlotPosition() == SlotPosition)
			return false;
	}
	
	return true;
}

bool UBaseInventoryWidget::bIsSlotEmpty(const UBaseInventorySlot* SlotCheck)
{
	FItemSlotMapping BusySlots = GetAllSlotsFromInventoryItemsMap();
	for (auto InvSlot : BusySlots.ItemSlots)
	{
		if (InvSlot == SlotCheck)
			return false;
	}
	return true;
}

void UBaseInventoryWidget::HandleRemoveItem(UItemBase* Item)
{
	if (!Item) return;

	auto Mapping = GetOccupiedSlots(Item);
	if (!Mapping) return;

	Mapping->ItemVisualLinked->RemoveFromParent();
	InventoryItemsMap.Remove(Item);
}

FItemAddResult UBaseInventoryWidget::HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	if (ItemMoveData.SourceInventory && ItemMoveData.TargetInventory &&  ItemMoveData.SourceInventory == ItemMoveData.TargetInventory)
	{
		HandleSwapItems(ItemMoveData);
		return FItemAddResult::AddedAll(0, false, FText::FromString("Successfully swaped Items"));
	}
	
	// non-stack
	if (!ItemMoveData.SourceItem->IsStackable())
	{
		// Check if input item has valid weight
		if (FMath::IsNearlyZero(ItemMoveData.SourceItem->GetItemSingleWeight()) || ItemMoveData.SourceItem->
			GetItemSingleWeight() < 0)
		{
			return FItemAddResult::AddedNone(FText::Format(FText::FromString("Item {0} has invalid weight"),
														   ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
		}

		// will the item weight overflow the weight capacity?
		if (InventoryWeightCapacity > 0)
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
	// will the item weight overflow the weight capacity?
	if (InventoryTotalWeight + ItemMoveData.SourceItem->GetItemSingleWeight() * ItemMoveData.SourceItem->GetQuantity() > InventoryWeightCapacity)
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Couldn't add {0} to inventory."),
														   ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}
	
	const int32 InitialRequestedAddAmount = ItemMoveData.SourceItem->GetQuantity();
	const int32 StackableAmountAdded = HandleStackableItems(ItemMoveData, InitialRequestedAddAmount,
																bOnlyCheck);

	if (StackableAmountAdded == InitialRequestedAddAmount)
	{
		return FItemAddResult::AddedAll(StackableAmountAdded, false, FText::Format(
												FText::FromString("Successfully added {0} of {1} to inventory"),
												InitialRequestedAddAmount,ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}
	else if (StackableAmountAdded < InitialRequestedAddAmount && StackableAmountAdded > 0)
	{
		return FItemAddResult::AddedPartial(StackableAmountAdded, false, FText::Format(
													FText::FromString(
														"Partial amount of {0} added to inventory. number added {1}"),
													ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name, StackableAmountAdded));
	}
	else if (StackableAmountAdded <= 0)
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Couldn't add {0} to inventory."),
														   ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}

	return FItemAddResult::AddedNone(FText::Format(
		FText::FromString("Could not add {0} to inventory. No remaining slots or weight."),
		ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
}


FItemAddResult UBaseInventoryWidget::HandleNonStackableItems(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (!ItemMoveData.TargetSlot)
	{
		TObjectPtr<UBaseInventorySlot> EmptySlot = nullptr;
		for (const auto InventorySlot : InventorySlots)
		{
			if (bool IsEmptySlot = bIsSlotEmpty(InventorySlot))
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
			FItemSlotMapping Slots;
			Slots.ItemSlots.Add(EmptySlot);
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
			FItemSlotMapping Slots;
			Slots.ItemSlots.Add(ItemMoveData.TargetSlot);
			AddNewItem(ItemMoveData, Slots);
		}

		return FItemAddResult::AddedAll(1, false, FText::Format(
											FText::FromString("Successfully added {0} of {1} to inventory"),
											1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}

	return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
}

int32 UBaseInventoryWidget::HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck)
{
	int32 AmountToDistribute = RequestedAddAmount;
	
	auto CalculateActualAmountToAdd = [&](int32 InAmountToAdd) -> int32
	{
		const int32 WeightLimitAddAmount = InventoryWeightCapacity - InventoryTotalWeight;
		int32 MaxItemsThatFit = WeightLimitAddAmount / ItemMoveData.SourceItem->GetItemSingleWeight();
		return FMath::Min(MaxItemsThatFit, InAmountToAdd);
	};

	if (!ItemMoveData.TargetSlot)
	{
		auto Sameitems = GetAllSameItems(ItemMoveData.SourceItem);
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

				Item->SetQuantity(Item->GetQuantity() + ActualAmountToAdd);
				AmountToDistribute -= ActualAmountToAdd;

				auto Slots = GetOccupiedSlots(Item);
				if (Slots)
					NotifyUpdateItem(Slots, Item);
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Unable to find occupied slots for item %s"), *Item->GetName());
				}
			}
		}
		
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

		FItemSlotMapping Slots;
		Slots.ItemSlots.Add(TargetSlot);
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

		FItemSlotMapping Slots;
		Slots.ItemSlots.Add(ItemMoveData.TargetSlot);
		FItemMoveData NewItemMoveData;
		NewItemMoveData.SourceItem = ItemMoveData.SourceItem;
		NewItemMoveData.SourceItem->SetQuantity(ActualAmountToAdd);
		
		AddNewItem(NewItemMoveData, Slots);
		ItemMoveData.SourceItem->SetQuantity(ItemMoveData.SourceItem->GetQuantity() - ActualAmountToAdd);
		return RequestedAddAmount - AmountToDistribute;
	}
	else
	{
		UItemBase* ItemFromSlot= GetItemFromSlot(ItemMoveData.TargetSlot);
		
		if (ItemFromSlot && ItemFromSlot != ItemMoveData.SourceItem)
			return 0;

		const int32 AmountToAddToStack = FMath::Min(AmountToDistribute,
				ItemMoveData.SourceItem->GetItemRef().ItemNumeraticData.MaxStackSize -
				ItemMoveData.SourceItem->GetQuantity());

		int32 WeightLimitAddAmount = InventoryWeightCapacity - InventoryTotalWeight;
		int32 MaxItemsThatFit = WeightLimitAddAmount / ItemMoveData.SourceItem->GetItemSingleWeight();
		int32 ActualAmountToAdd = FMath::Min(MaxItemsThatFit, AmountToDistribute);

		ItemFromSlot->SetQuantity(ItemFromSlot->GetQuantity() + ActualAmountToAdd);
		AmountToDistribute -= ActualAmountToAdd;

		//NotifyUpdateItem(Slots, ItemInPivot);
		return RequestedAddAmount - AmountToDistribute;
	}
}

void UBaseInventoryWidget::HandleSwapItems(FItemMoveData& ItemMoveData)
{
	if (bIsSlotEmpty(ItemMoveData.TargetSlot))
	{
		ReplaceItem (ItemMoveData.SourceItem, ItemMoveData.TargetSlot);
	}
}

void UBaseInventoryWidget::AddNewItem(FItemMoveData& ItemMoveData, FItemSlotMapping OccupiedSlots)
{
	TObjectPtr<UItemBase> NewItem = NewObject<UItemBase>(this); 
	NewItem->SetQuantity(ItemMoveData.SourceItem->GetQuantity());
	NewItem->SetItemRef(ItemMoveData.SourceItem->GetItemRef());
	NewItem->SetItemName(ItemMoveData.SourceItem->GetItemName());

	InventoryItemsMap.Add(NewItem, OccupiedSlots);
	
	NotifyAddItem(OccupiedSlots, NewItem);
}

void UBaseInventoryWidget::ReplaceItem(UItemBase* Item, UBaseInventorySlot* NewSlot)
{
	auto Mapping = InventoryItemsMap.Find(Item);
	Mapping->ItemSlots[0] = NewSlot;

	//UE_LOG(LogTemp, Warning, TEXT("ReplaceItem done!"))

	ReplaceItemInPanel(*Mapping, Item);
}

FVector2D UBaseInventoryWidget::CalculateItemVisualPosition(FIntVector2 SlotPosition, FIntVector2 ItemSize) const
{
	float X = SlotPosition.X * SlotSize.X;
	float Y = SlotPosition.Y * SlotSize.Y;

	return FVector2D(X, Y);
}

void UBaseInventoryWidget::AddItemToPanel(FItemSlotMapping& FromSlots, UItemBase* Item)
{
	auto Slots = GetOccupiedSlots(Item);

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
		SlotInCanvas->SetSize(FVector2D(SlotSize.X * 1, SlotSize.Y *  1));

	Slots->ItemVisualLinked = ItemVisual;

	ItemVisual->UpdateVisualSize(SlotSize, FIntVector2(1, 1));
	ItemVisual->UpdateItemName(Item->GetItemRef().ItemTextData.Name);
	ItemVisual->UpdateQuantityText(Item->GetQuantity());
	ItemVisual->UpdateVisual(Item);
	
	//ItemVisual->SetPivotSlot(ItemPivotSlot);			
	SlotInCanvas->SetPosition(VisualPosition);
}

void UBaseInventoryWidget::ReplaceItemInPanel(FItemSlotMapping& FromSlots, UItemBase* Item)
{
	if (!FromSlots.ItemVisualLinked) return;

	const UBaseInventorySlot* ItemPivotSlot =FromSlots.ItemSlots[0];
	const FVector2D VisualPosition = CalculateItemVisualPosition(ItemPivotSlot->GetSlotPosition(), Item->GetOccupiedSlots());

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(FromSlots.ItemVisualLinked->Slot);
	if (!CanvasSlot) return;

	CanvasSlot->SetPosition(VisualPosition);
}

void UBaseInventoryWidget::UpdateSlotInPanel(FItemSlotMapping* FromSlots, UItemBase* Item)
{
	if (!FromSlots->ItemVisualLinked || !Item)
		return;

	FromSlots->ItemVisualLinked->UpdateQuantityText(Item->GetQuantity());
}

void UBaseInventoryWidget::UpdateWeightInfo()
{
	if (!WeightInfo)
		return;

	FString Text_WeightInfo;
	InventoryTotalWeight = 0;
	auto AllItems = GetAllItems();
	if (AllItems.IsEmpty())
	{
		Text_WeightInfo = {" " + FString::SanitizeFloat(0) + "/" + FString::SanitizeFloat(InventoryWeightCapacity)};
	}
	else
	{
		for (auto Item : AllItems)
		{
			InventoryTotalWeight+= (Item->GetQuantity() + Item->GetItemSingleWeight());
		}
	}

	if (InventoryTotalWeight > 0)
		Text_WeightInfo = {" " + FString::SanitizeFloat(InventoryTotalWeight) + "/" + FString::SanitizeFloat(InventoryWeightCapacity)};
	
	WeightInfo->SetText(FText::FromString(Text_WeightInfo));
}

void UBaseInventoryWidget::NotifyAddItem(FItemSlotMapping& FromSlots, UItemBase* NewItem)
{
	AddItemToPanel(FromSlots, NewItem);
	UpdateWeightInfo();
	OnAddItemDelegate.Broadcast(FromSlots, NewItem);
}

void UBaseInventoryWidget::NotifyUpdateItem(FItemSlotMapping* FromSlots, UItemBase* NewItem)
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

void UBaseInventoryWidget::NotifyRemoveItem(FItemSlotMapping FromSlots, UItemBase* RemovedItem) const
{
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

	int32 Column = FMath::FloorToInt(LocalCursorPos.X / SlotSize.X);
	int32 Row = FMath::FloorToInt(LocalCursorPos.Y / SlotSize.Y);
	
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

			auto Linked= GetItemLinkedWidget(SelectedSlot);
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

	DragItemDragDropOperation->ItemMoveData.SourceItem = GetItemFromSlot(SelectedSlot);
	DragItemDragDropOperation->ItemMoveData.SourceInventory = this;

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
	
	if (GridPosition.X >= 0 && GridPosition.Y >= 0)
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
	
	if (GridPosition.X >= 0 && GridPosition.Y >= 0)
	{
		auto DragOp = Cast<UItemDragDropOperation>(InOperation);
		auto TargetSlot = GetSlotByPosition(FIntVector2(GridPosition.X, GridPosition.Y));
		DragOp->ItemMoveData.TargetInventory = this;
		DragOp->ItemMoveData.TargetSlot = TargetSlot;

		auto Result = HandleAddItem(DragOp->ItemMoveData, false);

		return true;
	}
	
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}


