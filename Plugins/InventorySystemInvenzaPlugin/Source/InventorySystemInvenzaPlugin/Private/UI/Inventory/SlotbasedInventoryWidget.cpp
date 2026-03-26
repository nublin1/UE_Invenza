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
	SlotBasedInventoryRef->OnInventoryRedrawRequested.AddDynamic(this, &USlotbasedInventoryWidget::ReDrawAllItems);
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

			UInventorySlotData* NewInventorySlotData = UInventorySlotData::Create(this);
			NewSlot->SetSlotData(NewInventorySlotData);
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
		InventoryRef = NewInventoryRef;
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

			auto SlotPosit = FIntPoint(UniSlot->GetRow(),  UniSlot->GetColumn());
			
			UInventorySlotData* NewInventorySlotData = UInventorySlotData::CreateWithData(this, NAME_None, SlotPosit, nullptr, NewInvSlots[i]->AllowedSlotCategory);
			NewInvSlots[i]->SetSlotData(NewInventorySlotData);
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

	for (auto ItemSlotData : NewItemSlots.OccupiedSlots)
	{
		if(const auto ItemSlot = GetSlotByPosition(ItemSlotData->CellPosition))
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

	if (!SlotBasedInventoryRef)
		return Reply;
	
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
		ItemTooltipWidget->SetTooltipData(ItemInSlot, SlotBasedInventoryRef);
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
	DraggedWidget->UpdateVisual(Item);
	
	DraggedWidget->SetVisibility(ESlateVisibility::Hidden);
	
	
	UItemDragDropOperation* DragItemDragDropOperation = NewObject<UItemDragDropOperation>();
	DragItemDragDropOperation->DefaultDragVisual = DraggedWidget;
	DragItemDragDropOperation->Pivot = EDragPivot::TopLeft;

	DragItemDragDropOperation->ItemMoveData.SourceItem = ItemCollection->GetItemFromSlot(SlotUnderMouse->GetSlotData(), InvID);
	DragItemDragDropOperation->ItemMoveData.SourceInventory = SlotBasedInventoryRef;
	DragItemDragDropOperation->ItemMoveData.SourceItemPivotSlot = SlotUnderMouse;
	DragItemDragDropOperation->ItemMoveData.SavedOrientation = ItemCollection->FindItemMappingByContainerName(Item, InvID)->ItemOrientation;
	DragItemDragDropOperation->ItemMoveData.TargetOrientation = ItemCollection->FindItemMappingByContainerName(Item, InvID)->ItemOrientation;

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

		auto Result = InventoryRef->HandleAddItem(DragOp->ItemMoveData, true);
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
