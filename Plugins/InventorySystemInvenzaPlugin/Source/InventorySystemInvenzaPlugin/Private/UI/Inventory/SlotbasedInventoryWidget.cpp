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
#include "UI/Core/Buttons/FilterTagButton.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/Image/ImageBaseWidget.h"
#include "UI/Core/ItemFiltersPanel/FiltersPanel.h"
#include "UI/Drag/HighlightSlotWidget.h"
#include "UI/HelpersWidgets/ItemTooltipWidget.h"
#include "UI/Inventory/ListInventoryWidget.h"
#include "UI/Inventory/SlotbasedInventorySlot.h"
#include "UI/Item/InventoryItemWidget.h"
#include "Utility/InventoryUtility.h"

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
	CreateTooltipWidget();
}

void USlotbasedInventoryWidget::InitializeInventoryWidgetWithSettings()
{
	ApplyInventorySettings();
	BuildInventorySlots();
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
	SlotBasedInventoryRef->OnTradeContextUpdated.AddDynamic(this, &USlotbasedInventoryWidget::UpdateTradeContext);
	SlotBasedInventoryRef->OnRequestToResetItemVisual.AddDynamic(this, &USlotbasedInventoryWidget::ResetItemVisual);

	SlotBasedInventoryRef->OnInventorySlotDataUpdated.AddDynamic(this, &USlotbasedInventoryWidget::ReDrawInvSlots);
}

void USlotbasedInventoryWidget::ReDrawInvSlots()
{
	InitializeInventoryWidgetWithSettings();
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

FSlotBasedInventoryWidgetInitData USlotbasedInventoryWidget::CollectInitSlotsDataFromWidget()
{
	FSlotBasedInventoryWidgetInitData Result;
	
	if (!SlotsGridPanel)
		return Result;

	if (!SlotBasedInventoryRef)
		return Result;
	
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
					if (NewInvSlots.Num() == 0)
					{
						Result.InvCellSize = InventorySlot->GetSlotSize();
					}
					
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

			FInventorySlotInfo SlotInfo;
			SlotInfo.SlotName = NAME_None,
			SlotInfo.CellPosition = SlotPosit;
			SlotInfo.UseAction = nullptr;
			SlotInfo.AllowedCategory = NewInvSlots[i]->AllowedSlotCategory;
			
			Result.SlotLayout.Add(SlotInfo);
		}
	}

	Result.SlotSpacing = SlotsGridPanel->GetSlotPadding();
	Result.InventorySize = GetNumberRowsAndColumns();
	
	return Result;
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
		BindDelegated();
	}
}

void USlotbasedInventoryWidget::ApplyInventorySettings()
{
	if (!SlotBasedInventoryRef)
		return;
	
	auto InvSettings = SlotBasedInventoryRef->GetInventorySettings();
	
	if (!SlotsGridPanel || !InvSettings.InventorySlotBasedSettings.SlotbasedInventorySlotClass || !SlotBasedInventoryRef)
	{
		if (!InvSettings.InventorySlotBasedSettings.SlotbasedInventorySlotClass)
		{
			UE_LOG(LogTemp, Error, TEXT("USlotbasedInventoryWidget: SlotbasedInventorySlotClass is NULL"));
		}
		return;
	}

	TargetInventoryTag = InvSettings.InventoryTag;

	auto InvSize = SlotBasedInventoryRef->GetInventorySize();
	NumberRows  = InvSize.X;
	NumColumns  = InvSize.Y;
	SlotSpacing = SlotBasedInventoryRef->GetSlotSpacing();
	InvCellSize = SlotBasedInventoryRef->GetInvCellSize();

	SlotsGridPanel->SetSlotPadding(SlotSpacing);
}

void USlotbasedInventoryWidget::BuildInventorySlots()
{
	if (!SlotsGridPanel)
	{
		return;
	}

	SlotsGridPanel->ClearChildren();
	InventorySlots.Empty();

	TArray<UInventorySlotData*> ExistingSlots;
	ExistingSlots = SlotBasedInventoryRef->GetInventorySlots();
	
	auto InvSettings = SlotBasedInventoryRef->GetInventorySettings();

	for (int32 Row = 0; Row < NumberRows; ++Row)
	{
		for (int32 Col = 0; Col < NumColumns; ++Col)
		{
			USlotbasedInventorySlot* NewSlot = CreateWidget<USlotbasedInventorySlot>(
				GetOwningPlayer(),
				InvSettings.InventorySlotBasedSettings.SlotbasedInventorySlotClass
			);

			if (!NewSlot)
				continue;

			if (!NewSlot->GetDefaultCellImage() && DefaultCellImage)
			{
				NewSlot->UpdateVisualWithTexture(DefaultCellImage);
			}

			FIntPoint SlotPosit(Row, Col);

			UInventorySlotData* SlotData = nullptr;
			for (UInventorySlotData* ExistingSlot : ExistingSlots)
			{
				if (ExistingSlot && ExistingSlot->InventorySlotInfo.CellPosition == SlotPosit)
				{
					SlotData = ExistingSlot;
					break;
				}
			}
			
			if (!SlotData)
				continue;

			NewSlot->SetSlotData(SlotData);
			NewSlot->SetSlotPosition(SlotPosit);

			UUniformGridSlot* GridSlot = SlotsGridPanel->AddChildToUniformGrid(NewSlot, Row, Col);

			InventorySlots.Add(NewSlot);
		}
	}
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
			ItemMapping->ItemVisualLinked->CoreCellWidget->ResetBorderColor();
			ItemMapping->ItemVisualLinked->ChangeOpacity(1.0f);
		}
	}

	ActiveFilters.Empty();
}

void USlotbasedInventoryWidget::OnFilterStatusChanged(UUIButton* ItemCategoryButton)
{
	auto CastedCategoryButton = Cast<UFilterTagButton>(ItemCategoryButton);
	if (!CastedCategoryButton)
		return;

	const FGameplayTag Category = CastedCategoryButton->GetFilterTag();
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
{if (ActiveFilters.Num() == 0)
{
	ClearFilters();
	return;
}

	auto ItemsWithMappings =
		SlotBasedInventoryRef->GetItemCollectionLinked()
		->GetItemsWithMappingsByContainer(SlotBasedInventoryRef->GetInventoryContainerID());

	if (ItemsWithMappings.Num() == 0)
		return;

	for (auto& Item : ItemsWithMappings)
	{
		auto* Mapping = Item.Value;
		auto Visual = Mapping->ItemVisualLinked;

		if (!Visual)
			continue;

		const bool bPassFilter = ActiveFilters.Contains(Item.Key->GetItemRef().ItemCategory);

		if (bPassFilter)
		{
			if (ItemFiltersPanel->bUseFilterColor)
			{
				Visual->ChangeBorderColor(ItemFiltersPanel->ItemFilterBorderColor);
			}

			Visual->ChangeOpacity(1.0f);
		}
		else
		{
			Visual->ChangeOpacity(ItemFiltersPanel->FilterOpacity);
			Visual->CoreCellWidget->ResetBorderColor();
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

			ItemMapping->ItemVisualLinked->CoreCellWidget->ResetBorderColor();
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
					ItemMapping->ItemVisualLinked->CoreCellWidget->ResetBorderColor();
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
			ItemMapping->ItemVisualLinked->CoreCellWidget->ResetBorderColor();
			ItemMapping->ItemVisualLinked->ChangeOpacity(ItemFiltersPanel->FilterOpacity);
		}
	}
}

void USlotbasedInventoryWidget::ResetItemVisual(UItemBase* ItemToReset)
{
	if (!ItemToReset)
		return;

	auto ItemCollection =SlotBasedInventoryRef->GetItemCollectionLinked();
	auto InvID = SlotBasedInventoryRef->GetInventoryContainerID();
	
	auto ItemMapping = ItemCollection->FindItemMappingByContainerName(ItemToReset, InvID);
	if (!ItemMapping || !ItemMapping->ItemVisualLinked)
		return;

	ItemMapping->ItemVisualLinked->CoreCellWidget->ResetBorderColor();
	ItemMapping->ItemVisualLinked->ChangeOpacity(1.0f);
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
	const float StepX = InvCellSize.X + SlotSpacing.Left + SlotSpacing.Right;
	const float StepY = InvCellSize.Y + SlotSpacing.Top + SlotSpacing.Bottom;
	
	float X = SlotPosition.X * StepX + SlotSpacing.Left;
	float Y = SlotPosition.Y * StepY + SlotSpacing.Top;

	return FVector2D(Y, X);
}


void USlotbasedInventoryWidget::AddItemToPanel(FItemMapping& ItemSlots, UItemBase* Item)
{
	if (!Item)
		return;
	
	//auto Slots = ItemSlots;

	if (ItemSlots.OccupiedSlots.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemSlotDatas Is empty!"));
		return;
	}
	
	const FVector2D VisualPosition = CalculateItemVisualPosition(ItemSlots.OccupiedSlots[0]->InventorySlotInfo.CellPosition);

	if (!UISettings.InventoryItemVisualClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("USlotbasedInventoryWidget::InventoryItemVisualClass is not set in UISettings!"));
		return;
	}

	TObjectPtr<UInventoryItemWidget> ItemVisual = CreateWidget<UInventoryItemWidget>(
		GetAsContainerWidget(), UISettings.InventoryItemVisualClass);
	ItemsVisualsPanel->AddChildToCanvas(ItemVisual);
	ItemVisual->CoreCellWidget->Content_Image->SetBaseMaterial(UISettings.SlotBasedInventoryItemMaterial);

	SlotBasedInventoryRef->GetItemCollectionLinked()->UpdateItemVisualLinks(Item, SlotBasedInventoryRef->GetInventoryContainerID(), ItemVisual, nullptr);

	for (auto ItemSlotData : ItemSlots.OccupiedSlots)
	{
		if (auto ItemSlot = GetSlotByPosition(ItemSlotData->InventorySlotInfo.CellPosition))
		{
			if (bHideBackgroundWhenOccupied)
				ItemSlot->ClearVisual();
			else if (OccupiedCellImage)
				ItemSlot->UpdateVisualWithTexture(OccupiedCellImage);
		}
	}

	bool IgnoreSize = SlotBasedInventoryRef->GetInventorySettings().InventorySlotBasedSettings.bIgnoreItemSize;
	auto TotalSize = UInventoryUtility::CalculateItemVisualSize(Item, ItemSlots.ItemOrientation, InvCellSize, SlotSpacing, IgnoreSize);
	
	ItemVisual->UpdateItemVisual(Item, ItemSlots.ItemOrientation, TotalSize, VisualPosition, IgnoreSize);
	ItemVisual->UpdateItemName(Item->GetItemRef().ItemTextData.DisplayName);
	ItemVisual->UpdateQuantityText(Item->GetQuantity());

	RefreshFilteredItemsList();
	if (ItemFiltersPanel)
	{
		auto SearchText = ItemFiltersPanel->GetSearchText()->GetText();
		if (!SearchText.IsEmpty())
			SearchTextChanged(SearchText);
	}
}

void USlotbasedInventoryWidget::ReplaceItemInPanel(TArray<UInventorySlotData*> OldItemSlots, FItemMapping& NewItemSlots, UItemBase* Item)
{
	if (!Item) return;

	if (OldItemSlots.IsEmpty() && NewItemSlots.ItemVisualLinked)
	{
		for (auto& SlotToReset : InventorySlots) SlotToReset->ResetVisual();
	}
	else
	{
		for (auto ItemSlotData : OldItemSlots)
		{
			if (auto ItemSlot = GetSlotByPosition(ItemSlotData->InventorySlotInfo.CellPosition))
			{
				ItemSlot->ResetVisual();
			}
		}
	}

	for (auto ItemSlotData : NewItemSlots.OccupiedSlots)
	{
		if(const auto ItemSlot = GetSlotByPosition(ItemSlotData->InventorySlotInfo.CellPosition))
		{
			if (bHideBackgroundWhenOccupied)
				ItemSlot->ClearVisual();
			else if (OccupiedCellImage)
				ItemSlot->UpdateVisualWithTexture(OccupiedCellImage);
		}
	}

	if (!NewItemSlots.ItemVisualLinked)
	{
		UE_LOG(LogTemp, Warning, TEXT("USlotbasedInventoryWidget::ReplaceItemInPanel ItemVisualLinked is null!"));
		return;
	}

	FVector2D NewVisualPosition = CalculateItemVisualPosition(NewItemSlots.OccupiedSlots[0]->InventorySlotInfo.CellPosition);
	//UE_LOG(LogTemp, Log, TEXT("Row: %f, Column: %f"),  NewVisualPosition.X,  NewVisualPosition.Y);
	bool IgnoreSize = SlotBasedInventoryRef->GetInventorySettings().InventorySlotBasedSettings.bIgnoreItemSize;
	auto TotalSize = UInventoryUtility::CalculateItemVisualSize(Item, NewItemSlots.ItemOrientation, InvCellSize, SlotSpacing, IgnoreSize);
	NewItemSlots.ItemVisualLinked->UpdateItemVisual(Item, NewItemSlots.ItemOrientation, TotalSize, NewVisualPosition, IgnoreSize);
}

void USlotbasedInventoryWidget::UpdateItem(UItemBase* Item)
{
	if (!Item) return;

	auto Mapping = SlotBasedInventoryRef->GetItemCollectionLinked()->FindItemMappingByContainerName(Item, SlotBasedInventoryRef->GetInventoryContainerID());
	if (!Mapping || !Mapping->ItemVisualLinked)
	{
		return;
	}
	FVector2D NewVisualPosition = CalculateItemVisualPosition(Mapping->OccupiedSlots[0]->InventorySlotInfo.CellPosition);
	bool IgnoreSize = SlotBasedInventoryRef->GetInventorySettings().InventorySlotBasedSettings.bIgnoreItemSize;
	auto TotalSize = UInventoryUtility::CalculateItemVisualSize(Item, Mapping->ItemOrientation, InvCellSize, SlotSpacing, IgnoreSize);
	Mapping->ItemVisualLinked->UpdateItemVisual(Item, Mapping->ItemOrientation, TotalSize, NewVisualPosition, IgnoreSize);
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
	if (!Item || !FromSlots.ItemVisualLinked)
	return;

	FromSlots.ItemVisualLinked->RemoveFromParent();

	for (auto ItemSlotData : FromSlots.OccupiedSlots)
	{
		if (auto ItemSlot = GetSlotByPosition(ItemSlotData->InventorySlotInfo.CellPosition))
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

void USlotbasedInventoryWidget::UpdateTradeContext()
{
	
}

void USlotbasedInventoryWidget::CreateHighlightWidget()
{
	if (!UISettings.HighlightSlotWidgetClass)
	{
		UE_LOG(LogTemp, Log, TEXT("USlotBasedInventoryWidget::HighlightSlotWidgetClass is null"));
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
	HighlightWidgetPreview->SetHighlightColors(UISettings.AllowedColor, UISettings.NotAllowedColor);
	HighlightWidgetPreview->CoreCellWidget->Content_Image->SetBaseMaterial(UISettings.HighlightItemMaterial);
	HighlightWidgetPreview->SetVisibility(ESlateVisibility::Collapsed);
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
				if (InvSlot && InvSlot->GetSlotData() && SlotGeometry.IsUnderLocation(ScreenCursorPos))
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
		int32 Column = FMath::FloorToInt(LocalCursorPos.X / (InvCellSize.X + SlotSpacing.Left + SlotSpacing.Right));
		int32 Row    = FMath::FloorToInt(LocalCursorPos.Y / (InvCellSize.Y + SlotSpacing.Top + SlotSpacing.Bottom));

		return FIntPoint(Row, Column);
	}
}

FReply USlotbasedInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (!SlotBasedInventoryRef)
		return FReply::Unhandled();
	
	FVector2D ScreenCursorPos = InMouseEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);
	
	auto ItemCollection = SlotBasedInventoryRef->GetItemCollectionLinked();
	auto InvID = SlotBasedInventoryRef->GetInventoryContainerID();
	
	bool bIsPosValid = bIsGridPositionValid(GridPosition);
	if (bIsPosValid)
	{
		SlotUnderMouse = GetSlotByPosition(GridPosition);
	}
	
	if (!SlotUnderMouse || !ItemCollection) return FReply::Unhandled();
	
	auto ItemInSlot = ItemCollection->GetItemFromSlot(SlotUnderMouse->GetSlotData(), InvID);
	if (!ItemInSlot) return FReply::Unhandled();
	
	auto Handler = UInventoryUtility::FindInventoryHandler(GetOwningPlayerPawn());
	if (!Handler) return FReply::Unhandled();
	
	if (InMouseEvent.GetEffectingButton() == UISettings.ItemSelectKey)
	{
		FItemMoveData ItemMoveData;
		ItemMoveData.SourceInventory = SlotBasedInventoryRef;
		ItemMoveData.SourceItemPivotSlotCoordinate = SlotUnderMouse->GetSlotData()->InventorySlotInfo.CellPosition;
		ItemMoveData.SourceItem = ItemInSlot;
		if (!ItemMoveData.SourceItem)
			return FReply::Unhandled();

		FInventoryModifierState Modifiers =
			IInventoryInteractionHandler::Execute_GetInventoryModifierStates(Handler->_getUObject());

		if (Modifiers.bIsQuickGrabModifierActive)
		{
			Handler->Execute_OnQuickTransferItem(Handler.GetObject(), ItemMoveData);

			return FReply::Handled();
		}
		if (Modifiers.bIsGrabAllSameModifierActive)
		{
			Handler->Execute_OnQuickTransferAllSameItems(Handler.GetObject(), ItemMoveData);

			return FReply::Handled();
		}
		
		return Reply.Handled().DetectDrag(TakeWidget(), UISettings.ItemSelectKey);
	}

	if (InMouseEvent.GetEffectingButton() == UISettings.ItemUseKey)
	{
		if (SlotBasedInventoryRef->GetInventorySettings().bAllowItemUsage)
		{
			ItemInSlot->UseItem();
			FReply::Handled();
		}
	}
	
	if (InMouseEvent.GetEffectingButton() == UISettings.ItemMenuKey)
	{
		Handler->Execute_ItemContextMenuRequest(Handler.GetObject(),InvID, ItemInSlot);

		return FReply::Handled();
	}
	
	return FReply::Unhandled();
}

FReply USlotbasedInventoryWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);

	if (!SlotBasedInventoryRef)
		return FReply::Unhandled();

	FVector2D ScreenCursorPos = InMouseEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);
	
	auto ItemCollection = SlotBasedInventoryRef->GetItemCollectionLinked();
	auto InvID = SlotBasedInventoryRef->GetInventoryContainerID();
	
	if (InMouseEvent.GetEffectingButton() == UISettings.ItemSelectKey)
	{
		if (bIsGridPositionValid(GridPosition))
		{
			SlotUnderMouse = GetSlotByPosition(GridPosition);
		}
		if (!SlotUnderMouse) return FReply::Unhandled();
		
		auto ItemInSlot = ItemCollection->GetItemFromSlot(SlotUnderMouse->GetSlotData(), InvID);
		if (!ItemInSlot) return FReply::Unhandled();

		SlotBasedInventoryRef->RequestSplitStack(ItemInSlot, ItemInSlot->GetQuantity() / 2);
	}

	return Reply;
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
    auto Item = ItemCollection->GetItemFromSlot(SlotUnderMouse->GetSlotData(), InvID);
    auto ItemMap = ItemCollection->FindItemMappingByContainerName(Item, InvID);
    auto DraggedWidget = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(), UISettings.DraggedWidgetClass);

    if (!DraggedWidget || !Item || !ItemMap) return;

    DraggedWidget->AddToPlayerScreen(1);
    DraggedWidget->SetPositionInViewport(FVector2D(-10000, -10000));
	
	bool IgnoreSize = SlotBasedInventoryRef->GetInventorySettings().InventorySlotBasedSettings.bIgnoreItemSize;
	auto TotalSize = UInventoryUtility::CalculateItemVisualSize(Item,  ItemMap->ItemOrientation, UISettings.DragWidgetSlotSize, SlotSpacing, IgnoreSize);
	
	DraggedWidget->UpdateItemVisual(Item,ItemMap->ItemOrientation, TotalSize, FVector2D(0.0f), IgnoreSize);
	
    UItemDragDropOperation* DragOp = NewObject<UItemDragDropOperation>();
	DragOp->SetUISettings(UISettings);
    DragOp->DefaultDragVisual = DraggedWidget;
    DragOp->Pivot = EDragPivot::TopLeft;
    DragOp->ItemMoveData.SourceItem = Item;
    DragOp->ItemMoveData.SourceInventory = SlotBasedInventoryRef;
    DragOp->ItemMoveData.SourceItemPivotSlotCoordinate = SlotUnderMouse->GetSlotData()->InventorySlotInfo.CellPosition;
    DragOp->ItemMoveData.SavedOrientation = ItemMap->ItemOrientation;
    DragOp->ItemMoveData.TargetOrientation = ItemMap->ItemOrientation;

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda(
        [DraggedWidget]() { DraggedWidget->SetVisibility(ESlateVisibility::Visible); }
    ), 0.125f, false);

    DraggedWidget->SetVisibility(ESlateVisibility::Hidden);
    OutOperation = DragOp;

	if (!ItemMap->ItemVisualLinked)
	{
		return;
	}
	ItemMap->ItemVisualLinked->ChangeOpacity(0.33f);

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
	auto DragOp = Cast<UItemDragDropOperation>(InOperation);
	if (!DragOp) 
		return false;
	
	if (!SlotsGridPanel) return false;
	
	FVector2D ScreenCursorPos = InDragDropEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);
	
	if (bIsGridPositionValid(GridPosition))
	{
		//UE_LOG(LogTemp, Log, TEXT("Column: %d, Row: %d"), GridPosition.X, GridPosition.Y);

		if (!HighlightWidgetPreview)
			return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
		
		auto TargetSlot = GetSlotByPosition(GridPosition);
		if (!TargetSlot)
			return false;
		DragOp->ItemMoveData.TargetInventory = SlotBasedInventoryRef;
		DragOp->ItemMoveData.TargetSlotCoordinate = TargetSlot->GetSlotPosition();

		const FVector2D VisualPosition = CalculateItemVisualPosition(GridPosition);

		bool IgnoreSize = SlotBasedInventoryRef->GetInventorySettings().InventorySlotBasedSettings.bIgnoreItemSize;
		auto TotalSize = UInventoryUtility::CalculateItemVisualSize(DragOp->ItemMoveData.SourceItem, DragOp->ItemMoveData.TargetOrientation, InvCellSize, SlotSpacing, IgnoreSize);
		
		HighlightWidgetPreview->UpdateItemVisual(DragOp->ItemMoveData.SourceItem, DragOp->ItemMoveData.TargetOrientation, TotalSize, VisualPosition, IgnoreSize);
		HighlightWidgetPreview->ChangeOpacity(UISettings.HighlightItemOpacity);

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
	
	if (!InOperation || !SlotsGridPanel ||!SlotBasedInventoryRef) return false;
	
	FVector2D ScreenCursorPos = InDragDropEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);

	auto ItemCollection = SlotBasedInventoryRef->GetItemCollectionLinked();
	auto InvID = SlotBasedInventoryRef->GetInventoryContainerID();
	
	//UE_LOG(LogTemp, Log, TEXT("Row: %d, Column: %d"),  GridPosition.X,  GridPosition.Y);
	if (bIsGridPositionValid(GridPosition))
	{
		auto DragOp = Cast<UItemDragDropOperation>(InOperation);
		auto TargetSlot = GetSlotByPosition(GridPosition);

		if (auto ItemMap = ItemCollection->FindItemMappingByContainerName(DragOp->ItemMoveData.SourceItem, InvID))
		{
			if (ItemMap->ItemVisualLinked)
				ItemMap->ItemVisualLinked->ChangeOpacity(1.0f);
		}
		
		DragOp->ItemMoveData.TargetInventory = SlotBasedInventoryRef;
		DragOp->ItemMoveData.TargetSlotCoordinate = TargetSlot->GetSlotPosition();

		if (OnItemDroppedDelegate.IsBound())
			OnItemDroppedDelegate.Broadcast(DragOp->ItemMoveData);

		return true;
	}
	
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void USlotbasedInventoryWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
	
	auto ItemCollection = SlotBasedInventoryRef->GetItemCollectionLinked();
	auto InvID = SlotBasedInventoryRef->GetInventoryContainerID();
	
	auto DragOp = Cast<UItemDragDropOperation>(InOperation);
	if (DragOp && DragOp->ItemMoveData.SourceItem)
	{
		if (auto ItemMap = ItemCollection->FindItemMappingByContainerName(DragOp->ItemMoveData.SourceItem, InvID))
			ItemMap->ItemVisualLinked->ChangeOpacity(1.0f);
	}
	
}
