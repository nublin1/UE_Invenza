//  Nublin Studio 2026 All Rights Reserved.

#include "UI/Inventory/SlotbasedInventoryWidget.h"
#include "GameFramework/Pawn.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Engine/World.h"
#include "Data/Items/itemBase.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "UI/Core/CoreCellWidget.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/Image/ImageBaseWidget.h"
#include "UI/Core/ItemFiltersPanel/FiltersPanel.h"
#include "UI/Drag/HighlightSlotWidget.h"
#include "UI/HelpersWidgets/ItemTooltipWidget.h"
#include "UI/Inventory/ListInventoryWidget.h"
#include "UI/Inventory/SlotbasedInventorySlot.h"
#include "UI/Item/InventoryItemWidget.h"
#include "Utility/InterfaceUtils.h"
#include "Utility/InvenzayUtility.h"

static UWidget* GetSlotbasedInventoryCellVisual(UWidget* SlotWidget)
{
	const auto* Cell = Cast<USlotbasedInventorySlot>(SlotWidget);
	return Cell && Cell->CoreCellWidget && Cell->CoreCellWidget->SizeBox
			   ? static_cast<UWidget*>(Cell->CoreCellWidget->SizeBox.Get())
			   : SlotWidget;
}

USlotbasedInventoryWidget::USlotbasedInventoryWidget() : SlotsGridPanel(nullptr)
{
}

void USlotbasedInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemFiltersPanel)
	{
		if (ItemFiltersPanel->GetSearchText())
		{
			ItemFiltersPanel->GetSearchText()->OnTextChanged.AddUniqueDynamic(
				this, &USlotbasedInventoryWidget::SearchTextChanged);
		}

		for (auto FilterButton : ItemFiltersPanel->GetFilteredCategores())
		{
			FilterButton->OnButtonClicked.AddUniqueDynamic(this, &USlotbasedInventoryWidget::OnFilterStatusChanged);
		}

		if (ItemFiltersPanel->GetClearFiltersButton())
		{
			ItemFiltersPanel->GetClearFiltersButton()->MainButton->OnClicked.AddUniqueDynamic(
				this, &USlotbasedInventoryWidget::ClearFilters);
		}
	}
}

void USlotbasedInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bUsesAuthoredSlotLayout || !SlotBasedInventoryRef || !SlotsGridPanel)
	{
		return;
	}
	FVector2D ActualCellSize;
	if (!TryCollectCellSizeFromSlate(ActualCellSize))
	{
		return;
	}
	const FVector2D GridSize = SlotsGridPanel->GetCachedGeometry().GetLocalSize();
	if (bCellSizeCollected && CollectedLayoutSize.Equals(MyGeometry.GetLocalSize()) &&
		CollectedGridSize.Equals(GridSize) && InvCellSize.Equals(ActualCellSize))
	{
		return;
	}

	if (CollectAuthoredCellGeometry())
	{
		bCellSizeCollected = true;
		CollectedLayoutSize = MyGeometry.GetLocalSize();
		CollectedGridSize = GridSize;
		ReDrawAllItems();
	}
}

void USlotbasedInventoryWidget::InitializeInventoryWidget()
{
	CreateTooltipWidget();
}

void USlotbasedInventoryWidget::InitializeInventoryWidgetWithSettings()
{
	if (!SlotBasedInventoryRef)
	{
		return;
	}
	const auto PendingSlots = SlotBasedInventoryRef->GetInventorySlots();
	// Replicated slot references can arrive in a later bunch. Keep the authored layout intact.
	if (PendingSlots.Contains(nullptr) ||
		(SlotBasedInventoryRef->GetInventorySettings().bCollectInvDataFromWidget && PendingSlots.IsEmpty()))
	{
		CreateTooltipWidget();
		return;
	}
	ApplyInventorySettings();
	BuildInventorySlots();
	CreateTooltipWidget();
	if (ScrollBox)
	{
		ScrollBox->SetClipping(EWidgetClipping::ClipToBoundsAlways);
	}
	if (ItemsVisualsPanel)
	{
		ItemsVisualsPanel->SetClipping(EWidgetClipping::ClipToBoundsAlways);
	}
	if (HighlightVisualsPanel)
	{
		HighlightVisualsPanel->SetClipping(EWidgetClipping::ClipToBoundsAlways);
	}
}

bool USlotbasedInventoryWidget::DoesItemFootprintFitGrid(UObject* Item, EItemOrientationType Orientation,
														 FIntPoint Pivot) const
{
	if (!IsValid(SlotBasedInventoryRef) || !IsValid(Item) || !Item->Implements<UObjectDataProvider>())
	{
		return false;
	}
	const auto Settings = SlotBasedInventoryRef->GetInventorySettings();
	const FIntPoint Footprint = Settings.InventorySlotBasedSettings.bIgnoreItemSize
									? FIntPoint(1, 1)
									: IObjectDataProvider::Execute_GetItemSize(Item, Orientation);
	const FIntPoint GridSize = SlotBasedInventoryRef->GetInventorySize();
	if (Pivot.X < 0 || Pivot.Y < 0 || Footprint.X <= 0 || Footprint.Y <= 0 || Pivot.X >= GridSize.X ||
		Pivot.Y >= GridSize.Y || Footprint.Y > GridSize.X - Pivot.X || Footprint.X > GridSize.Y - Pivot.Y)
	{
		return false;
	}
	// Sparse equipment layouts can contain holes inside their rectangular bounds.
	const auto Slots = SlotBasedInventoryRef->GetSlotsForItemAt(Pivot, Item, Orientation);
	return Slots.Num() == Footprint.X * Footprint.Y && !Slots.Contains(nullptr);
}

void USlotbasedInventoryWidget::BindDelegated()
{
	if (!SlotBasedInventoryRef)
	{
		return;
	}

	SlotBasedInventoryRef->OnAddItemDelegate.AddUniqueDynamic(this, &USlotbasedInventoryWidget::AddItemToPanel);
	SlotBasedInventoryRef->OnItemRemovedDelegate.AddUniqueDynamic(this,
																  &USlotbasedInventoryWidget::RemoveItemFromPanel);
	SlotBasedInventoryRef->OnItemReplaceDelegate.AddUniqueDynamic(this, &USlotbasedInventoryWidget::ReplaceItemInPanel);
	SlotBasedInventoryRef->OnStackedItemDelegate.AddUniqueDynamic(this, &USlotbasedInventoryWidget::UpdateItem);
	SlotBasedInventoryRef->OnUnstackedItemDelegate.AddUniqueDynamic(this, &USlotbasedInventoryWidget::UpdateItem);
	SlotBasedInventoryRef->OnUseSlotDelegate.AddUniqueDynamic(this, &USlotbasedInventoryWidget::UsedItemInPanel);

	SlotBasedInventoryRef->OnWeightUpdatedDelegate.AddUniqueDynamic(this, &USlotbasedInventoryWidget::UpdateWeightInfo);
	SlotBasedInventoryRef->OnMoneyUpdatedDelegate.AddUniqueDynamic(this, &USlotbasedInventoryWidget::UpdateMoneyInfo);
	SlotBasedInventoryRef->OnInventoryRedrawRequested.AddUniqueDynamic(this,
																	   &USlotbasedInventoryWidget::ReDrawAllItems);
	SlotBasedInventoryRef->OnTradeContextUpdated.AddUniqueDynamic(this, &USlotbasedInventoryWidget::UpdateTradeContext);
	SlotBasedInventoryRef->OnRequestToResetItemVisual.AddUniqueDynamic(this,
																	   &USlotbasedInventoryWidget::ResetItemVisual);

	SlotBasedInventoryRef->OnInventorySlotDataUpdated.AddUniqueDynamic(this,
																	   &USlotbasedInventoryWidget::ReDrawInvSlots);
}

void USlotbasedInventoryWidget::UpdateViewportSize()
{
	if (!GridSizeBox || !SlotBasedInventoryRef)
	{
		return;
	}

	const auto InvSettings = SlotBasedInventoryRef->GetInventorySettings();
	const FVector2D& CellSize = InvSettings.InventorySlotBasedSettings.InvCellSize;

	const float MaxWidth = VisibleRowsColumns.Y * CellSize.X;
	const float MaxHeight = VisibleRowsColumns.X * CellSize.Y;

	GridSizeBox->SetMaxDesiredWidth(MaxWidth);
	GridSizeBox->SetMaxDesiredHeight(MaxHeight);

	//GridSizeBox->SetMaxDesiredWidth(ViewportMaxSize.X);
	//GridSizeBox->SetMaxDesiredHeight(ViewportMaxSize.Y);
}

void USlotbasedInventoryWidget::ReDrawInvSlots()
{
	InitializeInventoryWidgetWithSettings();
}

void USlotbasedInventoryWidget::ReDrawAllItems()
{
	if (!GetWorld() || GetWorld()->bIsTearingDown)
	{
		return;
	}
	ConfigureTradeFilter();
	if (!IsValid(SlotBasedInventoryRef) || !ItemsVisualsPanel ||
		!IsValid(SlotBasedInventoryRef->GetItemCollectionLinked()))
	{
		return;
	}

	ItemsVisualsPanel->ClearChildren();
	for (auto& InventorySlot : InventorySlots)
	{
		if (IsValid(InventorySlot))
		{
			InventorySlot->ResetVisual();
		}
	}

	auto InvId = SlotBasedInventoryRef->GetInventoryContainerID();

	auto AllItems = SlotBasedInventoryRef->GetItemCollectionLinked()->GetAllItemsByContainer(InvId);
	if (AllItems.IsEmpty())
	{
		return;
	}

	for (auto Item : AllItems)
	{
		auto Mapping = SlotBasedInventoryRef->GetItemCollectionLinked()->FindItemMappingByContainerName(Item, InvId);
		if (Mapping && IsValid(Item))
		{
			AddItemToPanel(*Mapping, Item);
		}
	}
}

FSlotBasedInventoryWidgetInitData USlotbasedInventoryWidget::CollectInitSlotsDataFromWidget()
{
	FSlotBasedInventoryWidgetInitData Result;

	if (!SlotsGridPanel || !SlotBasedInventoryRef)
	{
		return Result;
	}

	TArray<TObjectPtr<USlotbasedInventorySlot>> NewInvSlots;
	const int32 NumChildren = SlotsGridPanel->GetChildrenCount();

	// 1. Collect only valid slot widgets.
	for (int32 i = 0; i < NumChildren; ++i)
	{
		if (UWidget* ChildWidget = SlotsGridPanel->GetChildAt(i))
		{
			if (auto InventorySlot = Cast<USlotbasedInventorySlot>(ChildWidget))
			{
				if (!InventorySlot->GetDefaultCellImage() && DefaultCellImage)
				{
					InventorySlot->UpdateVisualWithTexture(DefaultCellImage);
				}
				NewInvSlots.Add(InventorySlot);
			}
		}
	}

	// Reset grid-wide counters before calculating the layout again.
	NumberRows = 0;
	NumColumns = 0;

	// 2. Build the layout using only the collected slots.
	for (USlotbasedInventorySlot* SlotWidget : NewInvSlots)
	{
		if (!SlotWidget)
		{
			continue;
		}

		// Read the panel slot directly from the slot widget.
		const UUniformGridSlot* UniSlot = Cast<UUniformGridSlot>(SlotWidget->Slot);
		if (!UniSlot)
		{
			UE_LOG(LogTemp, Error, TEXT("Slot %s is NOT placed inside a UniformGridPanel!"), *SlotWidget->GetName());
			continue;
		}

		if (UniSlot->GetRow() >= NumberRows)
		{
			NumberRows = UniSlot->GetRow() + 1;
		}
		if (UniSlot->GetColumn() >= NumColumns)
		{
			NumColumns = UniSlot->GetColumn() + 1;
		}

		FInventorySlotInfo SlotInfo;
		SlotInfo.SlotName = NAME_None;
		SlotInfo.CellPosition = FIntPoint(UniSlot->GetRow(), UniSlot->GetColumn());
		SlotInfo.UseAction = nullptr;
		SlotInfo.AllowedCategory = SlotWidget->AllowedSlotCategory;
		SlotInfo.LinkedEquipmentSlot = SlotWidget->LinkedEquipmentSlotTag;

		Result.SlotLayout.Add(SlotInfo);
	}

	CachedInvSlots = NewInvSlots;

	Result.InvCellSize = SlotBasedInventoryRef->GetInventorySettings().InventorySlotBasedSettings.InvCellSize;
	if (!NewInvSlots.IsEmpty())
	{
		UWidget* FirstCell = GetSlotbasedInventoryCellVisual(NewInvSlots[0]);
		FirstCell->TakeWidget();
		FirstCell->ForceLayoutPrepass();
		const FVector2D AuthoredCellSize = FirstCell->GetDesiredSize();
		if (AuthoredCellSize.X > 0.f && AuthoredCellSize.Y > 0.f &&
			FMath::IsFinite(AuthoredCellSize.X) && FMath::IsFinite(AuthoredCellSize.Y))
		{
			Result.InvCellSize = AuthoredCellSize;
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
	{
		return Array;
	}

	for (auto& InventorySlot : InventorySlots)
	{
		Array.Add(InventorySlot->GetSlotData());
	}

	return Array;
}

void USlotbasedInventoryWidget::SetInventoryBaseRef(UInventoryBase* NewInventoryRef)
{
	if (SlotBasedInventoryRef == NewInventoryRef)
	{
		return;
	}
	if (IsValid(SlotBasedInventoryRef))
	{
		SlotBasedInventoryRef->OnAddItemDelegate.RemoveAll(this);
		SlotBasedInventoryRef->OnItemRemovedDelegate.RemoveAll(this);
		SlotBasedInventoryRef->OnItemReplaceDelegate.RemoveAll(this);
		SlotBasedInventoryRef->OnStackedItemDelegate.RemoveAll(this);
		SlotBasedInventoryRef->OnUnstackedItemDelegate.RemoveAll(this);
		SlotBasedInventoryRef->OnUseSlotDelegate.RemoveAll(this);
		SlotBasedInventoryRef->OnWeightUpdatedDelegate.RemoveAll(this);
		SlotBasedInventoryRef->OnMoneyUpdatedDelegate.RemoveAll(this);
		SlotBasedInventoryRef->OnInventoryRedrawRequested.RemoveAll(this);
		SlotBasedInventoryRef->OnTradeContextUpdated.RemoveAll(this);
		SlotBasedInventoryRef->OnRequestToResetItemVisual.RemoveAll(this);
		SlotBasedInventoryRef->OnInventorySlotDataUpdated.RemoveAll(this);

		if (auto* Collection = SlotBasedInventoryRef->GetItemCollectionLinked();
			IsValid(Collection) && ItemsVisualsPanel)
		{
			for (const auto& Pair :
				 Collection->GetItemsWithMappingsByContainer(SlotBasedInventoryRef->GetInventoryContainerID()))
			{
				if (Pair.Value && IsValid(Pair.Value->ItemVisualLinked) &&
					Pair.Value->ItemVisualLinked->GetParent() == ItemsVisualsPanel)
				{
					Pair.Value->ItemVisualLinked = nullptr;
				}
			}
		}
	}
	if (ItemsVisualsPanel)
	{
		ItemsVisualsPanel->ClearChildren();
	}

	SlotBasedInventoryRef = Cast<USlotbasedInventory>(NewInventoryRef);
	InventoryRef = SlotBasedInventoryRef;
	bUsesAuthoredSlotLayout = false;
	bCellSizeCollected = false;
	AuthoredCellRects.Reset();
	BindDelegated();
}

void USlotbasedInventoryWidget::ApplyInventorySettings()
{
	if (!SlotBasedInventoryRef)
	{
		return;
	}

	auto InvSettings = SlotBasedInventoryRef->GetInventorySettings();

	if (!InvSettings.bShowFilterPanel && ItemFiltersPanel)
	{
		ItemFiltersPanel->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (!SlotsGridPanel || !InvSettings.InventorySlotBasedSettings.SlotbasedInventorySlotClass ||
		!SlotBasedInventoryRef)
	{
		if (!InvSettings.InventorySlotBasedSettings.SlotbasedInventorySlotClass)
		{
			UE_LOG(LogTemp, Error, TEXT("USlotbasedInventoryWidget: SlotbasedInventorySlotClass is NULL"));
		}
		return;
	}

	TargetInventoryTag = InvSettings.InventoryTag;

	auto InvSize = SlotBasedInventoryRef->GetInventorySize();
	NumberRows = InvSize.X;
	NumColumns = InvSize.Y;
	InvCellSize = SlotBasedInventoryRef->GetInvCellSize();

	if (InvSettings.bCollectInvDataFromWidget)
	{
		SlotSpacing = SlotsGridPanel->GetSlotPadding();
	}
	else
	{
		SlotSpacing = SlotBasedInventoryRef->GetSlotSpacing();
		SlotsGridPanel->SetSlotPadding(SlotSpacing);
	}
}

void USlotbasedInventoryWidget::BuildInventorySlots()
{
	if (!SlotsGridPanel || !SlotBasedInventoryRef)
	{
		return;
	}
	bUsesAuthoredSlotLayout = false;
	bCellSizeCollected = false;
	AuthoredCellRects.Reset();
	const auto AuthoredData = SlotBasedInventoryRef->GetInventorySlots();
	const bool bCollectFromWidget = SlotBasedInventoryRef->GetInventorySettings().bCollectInvDataFromWidget;
	if ((bCollectFromWidget || !SlotBasedInventoryRef->GetSlotsWithLinkedEquipment().IsEmpty()) &&
		!AuthoredData.IsEmpty() &&
		SlotsGridPanel->GetChildrenCount() == AuthoredData.Num())
	{
		TArray<USlotbasedInventorySlot*> AuthoredWidgets;
		for (UWidget* Child : SlotsGridPanel->GetAllChildren())
		{
			auto* SlotWidget = Cast<USlotbasedInventorySlot>(Child);
			auto* GridSlot = SlotWidget ? Cast<UUniformGridSlot>(SlotWidget->Slot) : nullptr;
			const auto* Data = GridSlot ? AuthoredData.FindByPredicate(
											  [&](UInventorySlotData* Entry)
											  {
												  return Entry &&
														 Entry->InventorySlotInfo.CellPosition ==
															 FIntPoint(GridSlot->GetRow(), GridSlot->GetColumn()) &&
														 Entry->InventorySlotInfo.LinkedEquipmentSlot ==
															 SlotWidget->LinkedEquipmentSlotTag;
											  })
										: nullptr;
			if (!Data)
			{
				break;
			}
			AuthoredWidgets.Add(SlotWidget);
		}
		if (AuthoredWidgets.Num() == AuthoredData.Num())
		{
			// Bind data to the existing layout without recreating or resizing its cells.
			InventorySlots.Reset();
			for (auto* SlotWidget : AuthoredWidgets)
			{
				auto* GridSlot = CastChecked<UUniformGridSlot>(SlotWidget->Slot);
				SlotWidget->SetSlotData(
					SlotBasedInventoryRef->GetSlotByPosition(FIntPoint(GridSlot->GetRow(), GridSlot->GetColumn())));
				InventorySlots.Add(SlotWidget);
			}
			bUsesAuthoredSlotLayout = true;
			bCellSizeCollected = false;
			// Resolve template sizes before the first item redraw, then measure once after layout.
			UWidget* FirstCell = GetSlotbasedInventoryCellVisual(AuthoredWidgets[0]);
			FirstCell->TakeWidget();
			FirstCell->ForceLayoutPrepass();
			const FVector2D AuthoredSize = FirstCell->GetDesiredSize();
			if (AuthoredSize.X > 0 && AuthoredSize.Y > 0)
			{
				InvCellSize = AuthoredSize;
			}
			UpdateViewportSize();
			return;
		}
	}
	if (bCollectFromWidget)
	{
		// Keep the template intact while replicated or restored slot data is incomplete.
		InventorySlots.Reset();
		return;
	}

	SlotsGridPanel->ClearChildren();
	InventorySlots.Empty();

	const auto InvSettings = SlotBasedInventoryRef->GetInventorySettings();
	const auto ExistingSlots = SlotBasedInventoryRef->GetInventorySlots();
	const FVector2D& CellSize = InvSettings.InventorySlotBasedSettings.InvCellSize;

	if (SlotsGridPanel)
	{

		SlotsGridPanel->SetMinDesiredSlotWidth(CellSize.X);
		SlotsGridPanel->SetMinDesiredSlotHeight(CellSize.Y);
	}

	if (!InvSettings.InventorySlotBasedSettings.SlotbasedInventorySlotClass)
	{
		return;
	}

	/*UE_LOG(
		LogTemp,
		Warning,
		TEXT("\n\n========== BUILD INVENTORY SLOTS ==========\n")
		TEXT("Inventory: %s\n")
		TEXT("Inventory Slot Count: %d\n")
		TEXT("Grid Size: %d x %d"),
		*InvSettings.InventoryTag.ToString(),
		ExistingSlots.Num(),
		NumberRows,
		NumColumns
	);*/

	for (int32 Index = 0; Index < ExistingSlots.Num(); ++Index)
	{
		UInventorySlotData* SlotData = ExistingSlots[Index];

		if (!SlotData)
		{
			continue;
		}
	}

	TMap<FIntPoint, UInventorySlotData*> SlotsByPosition;

	for (UInventorySlotData* SlotData : ExistingSlots)
	{
		if (!SlotData)
		{
			continue;
		}

		const FIntPoint Position = SlotData->InventorySlotInfo.CellPosition;
		SlotsByPosition.Add(Position, SlotData);
	}

	for (int32 Row = 0; Row < NumberRows; ++Row)
	{
		for (int32 Col = 0; Col < NumColumns; ++Col)
		{
			const FIntPoint SlotPosition(Row, Col);

			UInventorySlotData** FoundSlot = SlotsByPosition.Find(SlotPosition);

			if (!FoundSlot || !*FoundSlot)
			{
				continue;
			}

			UInventorySlotData* SlotData = *FoundSlot;
			USlotbasedInventorySlot* NewSlot = CreateWidget<USlotbasedInventorySlot>(
				GetOwningPlayer(), InvSettings.InventorySlotBasedSettings.SlotbasedInventorySlotClass);

			NewSlot->SetSlotSize(CellSize);

			if (!NewSlot)
			{
				continue;
			}

			if (!NewSlot->GetDefaultCellImage() && DefaultCellImage)
			{
				NewSlot->UpdateVisualWithTexture(DefaultCellImage);
			}

			NewSlot->SetSlotData(SlotData);
			NewSlot->SetSlotPosition(SlotPosition);
			SlotsGridPanel->AddChildToUniformGrid(NewSlot, Row, Col);

			InventorySlots.Add(NewSlot);
		}
	}

	for (int32 Index = 0; Index < InventorySlots.Num(); ++Index)
	{
		auto WidgetSlot = InventorySlots[Index];
		UInventorySlotData* SlotData = WidgetSlot->GetSlotData();
	}

	UpdateViewportSize();
}

void USlotbasedInventoryWidget::ClearFilters()
{
	ActiveFilters.Empty();
	if (ItemFiltersPanel)
	{
		ItemFiltersPanel->DisableAllFilters();
	}

	RefreshFilteredItemsList();
}

void USlotbasedInventoryWidget::OnFilterStatusChanged(UUIButton* ItemCategoryButton)
{
	if (!ItemCategoryButton)
	{
		return;
	}

	const FGameplayTag Category = ItemCategoryButton->GetBtnTag();
	if (ItemCategoryButton->GetToggleStatus())
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
	ConfigureTradeFilter();
	if (!SlotBasedInventoryRef || !SlotBasedInventoryRef->GetItemCollectionLinked())
	{
		return;
	}

	const auto ItemsWithMappings = SlotBasedInventoryRef->GetItemCollectionLinked()->GetItemsWithMappingsByContainer(
		SlotBasedInventoryRef->GetInventoryContainerID());
	for (const auto& Pair : ItemsWithMappings)
	{
		if (!IsValid(Pair.Key) || !Pair.Value || !Pair.Value->ItemVisualLinked)
		{
			continue;
		}

		auto Visual = Pair.Value->ItemVisualLinked;
		const bool bPass = PassesCategoryAndSearch(Pair.Key);
		Visual->ChangeOpacity(bPass ? 1.0f : (GlobalSettings ? GlobalSettings->FilterOpacity : 0.3f));
		if (bPass && HasActiveItemFilters() && GlobalSettings && GlobalSettings->bUseFilterColor)
		{
			Visual->ChangeBorderColor(GlobalSettings->ItemFilterBorderColor);
		}
		else if (Visual->CoreCellWidget)
		{
			Visual->CoreCellWidget->ResetBorderColor();
		}
	}
}

void USlotbasedInventoryWidget::SearchTextChanged(const FText& NewText)
{
	RefreshFilteredItemsList();
}

void USlotbasedInventoryWidget::ResetItemVisual(UObject* ItemToReset)
{
	if (!ItemToReset)
	{
		return;
	}

	auto ItemCollection = SlotBasedInventoryRef->GetItemCollectionLinked();
	auto InvID = SlotBasedInventoryRef->GetInventoryContainerID();

	auto ItemMapping = ItemCollection->FindItemMappingByContainerName(ItemToReset, InvID);
	if (!ItemMapping || !ItemMapping->ItemVisualLinked)
	{
		return;
	}

	ItemMapping->ItemVisualLinked->CoreCellWidget->ResetBorderColor();
	ItemMapping->ItemVisualLinked->ChangeOpacity(1.0f);
}

UInventorySlot* USlotbasedInventoryWidget::GetSlotByPosition(FIntPoint SlotPosition)
{
	for (auto& Elem : InventorySlots)
	{
		if (Elem->GetSlotPosition() == SlotPosition)
		{
			return Elem;
		}
	}

	return nullptr;
}

bool USlotbasedInventoryWidget::bIsGridPositionValid(FIntPoint& GridPosition)
{
	return GridPosition.X >= 0 && GridPosition.Y >= 0 && GridPosition.X < NumberRows && GridPosition.Y < NumColumns;
}

FVector2D USlotbasedInventoryWidget::CalculateItemVisualPosition(FIntPoint SlotPosition) const
{
	const float StepX = InvCellSize.X + SlotSpacing.Left + SlotSpacing.Right;
	const float StepY = InvCellSize.Y + SlotSpacing.Top + SlotSpacing.Bottom;

	float X = SlotPosition.X * StepX + SlotSpacing.Left;
	float Y = SlotPosition.Y * StepY + SlotSpacing.Top;

	return FVector2D(Y, X);
}

void USlotbasedInventoryWidget::CalculateItemVisualRect(UObject* Item, EItemOrientationType Orientation,
	FIntPoint SlotPosition, UCanvasPanel* VisualPanel, FVector2D& OutPosition, FVector2D& OutSize) const
{
	const bool bIgnoreSize = SlotBasedInventoryRef->GetInventorySettings().InventorySlotBasedSettings.bIgnoreItemSize;
	OutPosition = CalculateItemVisualPosition(SlotPosition);
	OutSize = UInvenzayUtility::CalculateItemVisualSize(Item, Orientation, InvCellSize, SlotSpacing, bIgnoreSize);
	if (!bUsesAuthoredSlotLayout || !SlotsGridPanel || !VisualPanel || AuthoredCellRects.IsEmpty())
	{
		return;
	}
	const FGeometry& GridGeometry = SlotsGridPanel->GetCachedGeometry();
	const FGeometry& PanelGeometry = VisualPanel->GetCachedGeometry();
	if (PanelGeometry.GetLocalSize().X <= 0.f || PanelGeometry.GetLocalSize().Y <= 0.f)
	{
		return;
	}
	const FIntPoint Footprint = bIgnoreSize ? FIntPoint(1, 1)
		: IObjectDataProvider::Execute_GetItemSize(Item, Orientation);
	FVector2D Minimum(TNumericLimits<double>::Max(), TNumericLimits<double>::Max());
	FVector2D Maximum(TNumericLimits<double>::Lowest(), TNumericLimits<double>::Lowest());
	for (int32 Row = 0; Row < Footprint.Y; ++Row)
	{
		for (int32 Column = 0; Column < Footprint.X; ++Column)
		{
			const FSlateRect* Cell = AuthoredCellRects.Find(SlotPosition + FIntPoint(Row, Column));
			if (!Cell)
			{
				return;
			}
			Minimum.X = FMath::Min(Minimum.X, static_cast<double>(Cell->Left));
			Minimum.Y = FMath::Min(Minimum.Y, static_cast<double>(Cell->Top));
			Maximum.X = FMath::Max(Maximum.X, static_cast<double>(Cell->Right));
			Maximum.Y = FMath::Max(Maximum.Y, static_cast<double>(Cell->Bottom));
		}
	}
	if (Footprint.X > 0 && Footprint.Y > 0)
	{
		OutPosition = PanelGeometry.AbsoluteToLocal(GridGeometry.LocalToAbsolute(Minimum));
		OutSize = PanelGeometry.AbsoluteToLocal(GridGeometry.LocalToAbsolute(Maximum)) - OutPosition;
	}
}

bool USlotbasedInventoryWidget::CollectAuthoredCellGeometry()
{
	if (!SlotsGridPanel || !ItemsVisualsPanel || InventorySlots.IsEmpty())
	{
		return false;
	}
	const FGeometry& GridGeometry = SlotsGridPanel->GetCachedGeometry();
	const FVector2D GridSize = GridGeometry.GetLocalSize();
	const FVector2D PanelSize = ItemsVisualsPanel->GetCachedGeometry().GetLocalSize();
	if (GridSize.X <= 0.f || GridSize.Y <= 0.f || PanelSize.X <= 0.f || PanelSize.Y <= 0.f)
	{
		return false;
	}
	TMap<FIntPoint, FSlateRect> CellRects;
	for (UInventorySlot* SlotWidget : InventorySlots)
	{
		if (!IsValid(SlotWidget) || !SlotWidget->GetSlotData())
		{
			return false;
		}
		const FGeometry& CellGeometry = GetSlotbasedInventoryCellVisual(SlotWidget)->GetCachedGeometry();
		const FVector2D Size = CellGeometry.GetLocalSize();
		if (Size.X <= 0.f || Size.Y <= 0.f || !FMath::IsFinite(Size.X) || !FMath::IsFinite(Size.Y))
		{
			return false;
		}
		const FVector2D Minimum = GridGeometry.AbsoluteToLocal(CellGeometry.LocalToAbsolute(FVector2D::ZeroVector));
		const FVector2D Maximum = GridGeometry.AbsoluteToLocal(CellGeometry.LocalToAbsolute(Size));
		CellRects.Add(SlotWidget->GetSlotPosition(), FSlateRect(Minimum.X, Minimum.Y, Maximum.X, Maximum.Y));
	}
	AuthoredCellRects = MoveTemp(CellRects);
	return TryCollectCellSizeFromSlate(InvCellSize);
}

void USlotbasedInventoryWidget::AddItemToPanel(FItemMapping& ItemSlots, UObject* Item)
{
	if (!Item || !UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("AddItemToPanel")))
	{
		return;
	}

	if (!IsValid(SlotBasedInventoryRef) || !IsValid(SlotBasedInventoryRef->GetItemCollectionLinked()) ||
		!ItemsVisualsPanel || ItemSlots.OccupiedSlots.IsEmpty() || !IsValid(ItemSlots.OccupiedSlots[0]))
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemSlotDatas Is empty!"));
		return;
	}

	if (!UISettings.InventoryItemVisualClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryItemVisualClass is not set in UISettings!"));
		return;
	}

	TObjectPtr<UInventoryItemWidget> ItemVisual =
		CreateWidget<UInventoryItemWidget>(this, UISettings.InventoryItemVisualClass);

	if (!ItemVisual)
	{
		return;
	}

	ItemsVisualsPanel->AddChildToCanvas(ItemVisual);
	ItemVisual->CoreCellWidget->Content_Image->SetBaseMaterial(UISettings.SlotBasedInventoryItemMaterial);

	SlotBasedInventoryRef->GetItemCollectionLinked()->UpdateItemVisualLinks(
		Item, SlotBasedInventoryRef->GetInventoryContainerID(), ItemVisual, nullptr);

	for (auto ItemSlotData : ItemSlots.OccupiedSlots)
	{
		if (auto ItemSlot =
				IsValid(ItemSlotData) ? GetSlotByPosition(ItemSlotData->InventorySlotInfo.CellPosition) : nullptr)
		{
			if (bHideBackgroundWhenOccupied)
			{
				ItemSlot->ClearVisual();
			}
			else if (OccupiedCellImage)
			{
				ItemSlot->UpdateVisualWithTexture(OccupiedCellImage);
			}
		}
	}

	bool IgnoreSize = SlotBasedInventoryRef->GetInventorySettings().InventorySlotBasedSettings.bIgnoreItemSize;

	FVector2D VisualPosition;
	FVector2D TotalSize;
	CalculateItemVisualRect(Item, ItemSlots.ItemOrientation, ItemSlots.OccupiedSlots[0]->InventorySlotInfo.CellPosition,
		ItemsVisualsPanel, VisualPosition, TotalSize);

	ItemVisual->UpdateItemVisual(Item, ItemSlots.ItemOrientation, TotalSize, VisualPosition, IgnoreSize);

	ItemVisual->UpdateItemName(IObjectDataProvider::Execute_GetItemRef(Item).ItemTextData.DisplayName);

	ItemVisual->UpdateQuantityText(IObjectDataProvider::Execute_GetQuantity(Item));

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

void USlotbasedInventoryWidget::ReplaceItemInPanel(TArray<UInventorySlotData*> OldItemSlots, FItemMapping& NewItemSlots,
												   UObject* Item)
{
	if (!Item)
	{
		return;
	}

	if (OldItemSlots.IsEmpty() && NewItemSlots.ItemVisualLinked)
	{
		for (auto& SlotToReset : InventorySlots)
		{
			SlotToReset->ResetVisual();
		}
	}
	else
	{
		for (auto ItemSlotData : OldItemSlots)
		{
			if (auto ItemSlot =
					IsValid(ItemSlotData) ? GetSlotByPosition(ItemSlotData->InventorySlotInfo.CellPosition) : nullptr)
			{
				ItemSlot->ResetVisual();
			}
		}
	}

	for (auto ItemSlotData : NewItemSlots.OccupiedSlots)
	{
		if (const auto ItemSlot =
				IsValid(ItemSlotData) ? GetSlotByPosition(ItemSlotData->InventorySlotInfo.CellPosition) : nullptr)
		{
			if (bHideBackgroundWhenOccupied)
			{
				ItemSlot->ClearVisual();
			}
			else if (OccupiedCellImage)
			{
				ItemSlot->UpdateVisualWithTexture(OccupiedCellImage);
			}
		}
	}

	if (!IsValid(SlotBasedInventoryRef) || !IsValid(NewItemSlots.ItemVisualLinked) ||
		NewItemSlots.OccupiedSlots.IsEmpty() || !IsValid(NewItemSlots.OccupiedSlots[0]))
	{
		UE_LOG(LogTemp, Warning, TEXT("USlotbasedInventoryWidget::ReplaceItemInPanel ItemVisualLinked is null!"));
		return;
	}

	bool IgnoreSize = SlotBasedInventoryRef->GetInventorySettings().InventorySlotBasedSettings.bIgnoreItemSize;
	FVector2D NewVisualPosition;
	FVector2D TotalSize;
	CalculateItemVisualRect(Item, NewItemSlots.ItemOrientation,
		NewItemSlots.OccupiedSlots[0]->InventorySlotInfo.CellPosition, ItemsVisualsPanel, NewVisualPosition, TotalSize);
	NewItemSlots.ItemVisualLinked->UpdateItemVisual(Item, NewItemSlots.ItemOrientation, TotalSize, NewVisualPosition,
													IgnoreSize);
}

void USlotbasedInventoryWidget::UpdateItem(UObject* Item)
{
	if (!IsValid(SlotBasedInventoryRef) || !IsValid(SlotBasedInventoryRef->GetItemCollectionLinked()))
	{
		return;
	}

	if (!Item || !UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("UpdateItem")))
	{
		return;
	}

	auto Mapping = SlotBasedInventoryRef->GetItemCollectionLinked()->FindItemMappingByContainerName(
		Item, SlotBasedInventoryRef->GetInventoryContainerID());

	if (!Mapping || !IsValid(Mapping->ItemVisualLinked) || Mapping->OccupiedSlots.IsEmpty() ||
		!IsValid(Mapping->OccupiedSlots[0]))
	{
		return;
	}

	bool IgnoreSize = SlotBasedInventoryRef->GetInventorySettings().InventorySlotBasedSettings.bIgnoreItemSize;

	FVector2D NewVisualPosition;
	FVector2D TotalSize;
	CalculateItemVisualRect(Item, Mapping->ItemOrientation, Mapping->OccupiedSlots[0]->InventorySlotInfo.CellPosition,
		ItemsVisualsPanel, NewVisualPosition, TotalSize);

	Mapping->ItemVisualLinked->UpdateItemVisual(Item, Mapping->ItemOrientation, TotalSize, NewVisualPosition,
												IgnoreSize);

	Mapping->ItemVisualLinked->UpdateQuantityText(IObjectDataProvider::Execute_GetQuantity(Item));
}

void USlotbasedInventoryWidget::UpdateSlotInPanel(FItemMapping FromSlots, UObject* Item)
{
	if (!FromSlots.ItemVisualLinked || !Item ||
		!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("UpdateSlotInPanel")))
	{
		return;
	}

	FromSlots.ItemVisualLinked->UpdateQuantityText(IObjectDataProvider::Execute_GetQuantity(Item));

	FromSlots.ItemVisualLinked->UpdateItemName(IObjectDataProvider::Execute_GetItemRef(Item).ItemTextData.DisplayName);

	FromSlots.ItemVisualLinked->UpdateVisual(Item);
}

void USlotbasedInventoryWidget::RemoveItemFromPanel(FItemMapping FromSlots, UObject* Item)
{
	if (!Item || !FromSlots.ItemVisualLinked)
	{
		return;
	}

	FromSlots.ItemVisualLinked->RemoveFromParent();

	for (auto ItemSlotData : FromSlots.OccupiedSlots)
	{
		if (auto ItemSlot =
				IsValid(ItemSlotData) ? GetSlotByPosition(ItemSlotData->InventorySlotInfo.CellPosition) : nullptr)
		{
			ItemSlot->ResetVisual();
		}
	}
}

bool USlotbasedInventoryWidget::TryCollectCellSizeFromSlate(FVector2D& OutCellSize) const
{
	OutCellSize = FVector2D::ZeroVector;
	if (!SlotsGridPanel || SlotsGridPanel->GetChildrenCount() == 0)
	{
		return false;
	}

	UWidget* FirstSlotWidget = GetSlotbasedInventoryCellVisual(SlotsGridPanel->GetChildAt(0));
	if (!FirstSlotWidget)
	{
		return false;
	}

	TSharedPtr<SWidget> SlateWidget = FirstSlotWidget->GetCachedWidget();
	if (!SlateWidget.IsValid())
	{
		SlateWidget = FirstSlotWidget->TakeWidget();
	}

	if (!SlateWidget.IsValid())
	{
		return false;
	}

	const FGeometry& CellGeometry = SlateWidget->GetTickSpaceGeometry();
	const FVector2D DetectedSize = CellGeometry.GetLocalSize();
	const FGeometry& GridGeometry = SlotsGridPanel->GetCachedGeometry();
	if (DetectedSize.X > 0.f && DetectedSize.Y > 0.f &&
		FMath::IsFinite(DetectedSize.X) && FMath::IsFinite(DetectedSize.Y) &&
		GridGeometry.GetLocalSize().X > 0.f && GridGeometry.GetLocalSize().Y > 0.f)
	{
		OutCellSize = GridGeometry.AbsoluteToLocal(CellGeometry.LocalToAbsolute(DetectedSize)) -
			GridGeometry.AbsoluteToLocal(CellGeometry.LocalToAbsolute(FVector2D::ZeroVector));
		return true;
	}

	return false;
}

void USlotbasedInventoryWidget::UsedItemInPanel(UInventorySlotData* UsedSlot)
{
	if (!UsedSlot)
	{
		return;
	}

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
	{
		return;
	}

	HighlightWidgetPreview =
		CreateWidget<UHighlightSlotWidget>(GetAsContainerWidget(), UISettings.HighlightSlotWidgetClass);
	auto CanvasSlot = HighlightVisualsPanel->AddChild(HighlightWidgetPreview);
	if (!CanvasSlot)
	{
		return;
	}

	HighlightWidgetPreview->SetHighlightColors(UISettings.AllowedColor, UISettings.NotAllowedColor);
	HighlightWidgetPreview->CoreCellWidget->Content_Image->SetBaseMaterial(UISettings.HighlightItemMaterial);
	HighlightWidgetPreview->SetVisibility(ESlateVisibility::Collapsed);
}

FIntPoint USlotbasedInventoryWidget::CalculateGridPosition(const FGeometry& Geometry,
														   const FVector2D& ScreenCursorPos) const
{
	if (!SlotsGridPanel)
	{
		return FIntPoint(-1, -1);
	}
	FVector2D LocalCursorPos = SlotsGridPanel->GetCachedGeometry().AbsoluteToLocal(ScreenCursorPos);

	// AbsoluteToLocal already accounts for scrolling and the containing ScaleBox.
	if (!SlotsGridPanel->GetCachedGeometry().IsUnderLocation(ScreenCursorPos) ||
		(ScrollBox && !ScrollBox->GetCachedGeometry().IsUnderLocation(ScreenCursorPos)))
	{
		return FIntPoint(-1, -1);
	}

	if (bHasSlotSpacing || bUsesAuthoredSlotLayout)
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
		int32 Row = FMath::FloorToInt(LocalCursorPos.Y / (InvCellSize.Y + SlotSpacing.Top + SlotSpacing.Bottom));

		return FIntPoint(Row, Column);
	}
}

FReply USlotbasedInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry,
														  const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (!SlotBasedInventoryRef)
	{
		return FReply::Unhandled();
	}

	FVector2D ScreenCursorPos = InMouseEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);
	SlotUnderMouse = nullptr;

	auto ItemCollection = SlotBasedInventoryRef->GetItemCollectionLinked();
	auto InvID = SlotBasedInventoryRef->GetInventoryContainerID();

	bool bIsPosValid = bIsGridPositionValid(GridPosition);
	if (bIsPosValid)
	{
		SlotUnderMouse = GetSlotByPosition(GridPosition);
	}

	if (!SlotUnderMouse || !ItemCollection)
	{
		return FReply::Unhandled();
	}

	auto ItemInSlot = ItemCollection->GetItemFromSlot(SlotUnderMouse->GetSlotData()->InventorySlotInfo.SlotGuid, InvID);

	if (!ItemInSlot || !PassesRecentSalesFilter(ItemInSlot) ||
		!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemInSlot, TEXT("NativeOnMouseButtonDown")))
	{
		return FReply::Unhandled();
	}

	auto Handler = UInvenzayUtility::FindInventoryHandler(GetOwningPlayerPawn());
	if (!Handler)
	{
		return FReply::Unhandled();
	}

	if (InMouseEvent.GetEffectingButton() == UISettings.ItemSelectKey)
	{
		FItemMoveData ItemMoveData;
		ItemMoveData.SourceInventory = SlotBasedInventoryRef;
		ItemMoveData.SourceSlotID = SlotUnderMouse->GetSlotData()->InventorySlotInfo.SlotGuid;
		ItemMoveData.SourceItem = ItemInSlot;

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
			IObjectDataProvider::Execute_UseItem(ItemInSlot);
			return FReply::Handled();
		}
	}

	if (InMouseEvent.GetEffectingButton() == UISettings.ItemMenuKey)
	{
		Handler->Execute_ItemContextMenuRequest(Handler.GetObject(), InvID,
												SlotUnderMouse->GetSlotData()->InventorySlotInfo.SlotGuid, ItemInSlot);

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply USlotbasedInventoryWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry,
																 const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);

	if (!SlotBasedInventoryRef)
	{
		return FReply::Unhandled();
	}

	FVector2D ScreenCursorPos = InMouseEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);
	SlotUnderMouse = nullptr;

	auto ItemCollection = SlotBasedInventoryRef->GetItemCollectionLinked();
	auto InvID = SlotBasedInventoryRef->GetInventoryContainerID();

	if (InMouseEvent.GetEffectingButton() == UISettings.ItemSelectKey)
	{
		if (bIsGridPositionValid(GridPosition))
		{
			SlotUnderMouse = GetSlotByPosition(GridPosition);
		}

		if (!SlotUnderMouse)
		{
			return FReply::Unhandled();
		}

		auto ItemInSlot =
			ItemCollection->GetItemFromSlot(SlotUnderMouse->GetSlotData()->InventorySlotInfo.SlotGuid, InvID);

		if (!ItemInSlot || !PassesRecentSalesFilter(ItemInSlot) ||
			!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemInSlot,
																			   TEXT("NativeOnMouseButtonDoubleClick")))
		{
			return FReply::Unhandled();
		}

		SlotBasedInventoryRef->RequestSplitStack(ItemInSlot, IObjectDataProvider::Execute_GetQuantity(ItemInSlot) / 2);
	}

	return Reply;
}

FReply USlotbasedInventoryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	FVector2D ScreenCursorPos = InMouseEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);
	SlotUnderMouse = nullptr;

	if (!SlotBasedInventoryRef)
	{
		return Reply;
	}

	auto ItemCollection = SlotBasedInventoryRef->GetItemCollectionLinked();
	auto InvID = SlotBasedInventoryRef->GetInventoryContainerID();

	if (bIsGridPositionValid(GridPosition))
	{
		SlotUnderMouse = GetSlotByPosition(GridPosition);
	}

	if (!SlotUnderMouse || !ItemCollection)
	{
		return FReply::Unhandled();
	}

	auto ItemInSlot = ItemCollection->GetItemFromSlot(SlotUnderMouse->GetSlotData()->InventorySlotInfo.SlotGuid, InvID);

	if (ItemInSlot && PassesRecentSalesFilter(ItemInSlot) && ItemTooltipWidget)
	{
		ItemTooltipWidget->SetTooltipData(ItemInSlot, SlotBasedInventoryRef);
		ItemTooltipWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else if (ItemTooltipWidget)
	{
		ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return Reply;
}

void USlotbasedInventoryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
													 UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	if (!IsValid(SlotBasedInventoryRef) || !IsValid(SlotUnderMouse) || !SlotUnderMouse->GetSlotData() ||
		!IsValid(SlotBasedInventoryRef->GetItemCollectionLinked()))
	{
		return;
	}

	auto ItemCollection = SlotBasedInventoryRef->GetItemCollectionLinked();
	auto InvID = SlotBasedInventoryRef->GetInventoryContainerID();
	auto Item = ItemCollection->GetItemFromSlot(SlotUnderMouse->GetSlotData()->InventorySlotInfo.SlotGuid, InvID);
	if (!PassesRecentSalesFilter(Item))
	{
		return;
	}

	auto ItemMap = ItemCollection->FindItemMappingByContainerName(Item, InvID);
	auto DraggedWidget = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(), UISettings.DraggedWidgetClass);

	if (!DraggedWidget || !Item || !ItemMap)
	{
		return;
	}

	DraggedWidget->AddToPlayerScreen(1);
	DraggedWidget->SetPositionInViewport(FVector2D(-10000, -10000));

	bool IgnoreSize = SlotBasedInventoryRef->GetInventorySettings().InventorySlotBasedSettings.bIgnoreItemSize;
	auto TotalSize = UInvenzayUtility::CalculateItemVisualSize(Item, ItemMap->ItemOrientation,
															   UISettings.DragWidgetSlotSize, SlotSpacing, IgnoreSize);

	DraggedWidget->UpdateItemVisual(Item, ItemMap->ItemOrientation, TotalSize, FVector2D(0.0f), IgnoreSize);

	UItemDragDropOperation* DragOp = NewObject<UItemDragDropOperation>();
	DragOp->SetUISettings(UISettings);
	DragOp->DefaultDragVisual = DraggedWidget;
	DragOp->Pivot = EDragPivot::TopLeft;
	DragOp->ItemMoveData.SourceItem = Item;
	DragOp->ItemMoveData.SourceInventory = SlotBasedInventoryRef;
	DragOp->ItemMoveData.SourceSlotID = SlotUnderMouse->GetSlotData()->InventorySlotInfo.SlotGuid;
	DragOp->ItemMoveData.SavedOrientation = ItemMap->ItemOrientation;
	DragOp->ItemMoveData.TargetOrientation = ItemMap->ItemOrientation;

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle,
										   FTimerDelegate::CreateWeakLambda(DraggedWidget,
																			[DraggedWidget]()
																			{
																				DraggedWidget->SetVisibility(
																					ESlateVisibility::Visible);
																			}),
										   0.125f, false);

	DraggedWidget->SetVisibility(ESlateVisibility::Hidden);
	OutOperation = DragOp;

	if (!ItemMap->ItemVisualLinked)
	{
		return;
	}

	ItemMap->ItemVisualLinked->ChangeOpacity(0.33f);

	if (!HighlightWidgetPreview)
	{
		CreateHighlightWidget();
	}
	if (HighlightWidgetPreview)
	{
		HighlightWidgetPreview->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USlotbasedInventoryWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
												  UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

	if (!HighlightWidgetPreview)
	{
		CreateHighlightWidget();
	}
	if (HighlightWidgetPreview)
	{
		HighlightWidgetPreview->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USlotbasedInventoryWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent,
												  UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	if (HighlightWidgetPreview)
	{
		HighlightWidgetPreview->SetVisibility(ESlateVisibility::Collapsed);
	}
}

bool USlotbasedInventoryWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
												 UDragDropOperation* InOperation)
{
	auto DragOp = Cast<UItemDragDropOperation>(InOperation);
	if (!DragOp)
	{
		return false;
	}

	if (!SlotsGridPanel)
	{
		return false;
	}

	FVector2D ScreenCursorPos = InDragDropEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);

	if (HighlightWidgetPreview)
	{
		HighlightWidgetPreview->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (bIsGridPositionValid(GridPosition) &&
		DoesItemFootprintFitGrid(DragOp->ItemMoveData.SourceItem, DragOp->ItemMoveData.TargetOrientation, GridPosition))
	{
		// The full footprint must remain inside real cells; overflow must not size the overlay.

		if (!HighlightWidgetPreview)
		{
			return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
		}

		auto TargetSlot = GetSlotByPosition(GridPosition);
		if (!TargetSlot)
		{
			return false;
		}

		DragOp->ItemMoveData.TargetInventory = SlotBasedInventoryRef;
		DragOp->ItemMoveData.TargetSlotID = TargetSlot->GetSlotData()->InventorySlotInfo.SlotGuid;

		bool IgnoreSize = SlotBasedInventoryRef->GetInventorySettings().InventorySlotBasedSettings.bIgnoreItemSize;
		FVector2D VisualPosition;
		FVector2D TotalSize;
		CalculateItemVisualRect(DragOp->ItemMoveData.SourceItem, DragOp->ItemMoveData.TargetOrientation, GridPosition,
			HighlightVisualsPanel, VisualPosition, TotalSize);

		HighlightWidgetPreview->UpdateItemVisual(DragOp->ItemMoveData.SourceItem,
												 DragOp->ItemMoveData.TargetOrientation, TotalSize, VisualPosition,
												 IgnoreSize);
		HighlightWidgetPreview->ChangeOpacity(UISettings.HighlightItemOpacity);
		HighlightWidgetPreview->SetVisibility(ESlateVisibility::HitTestInvisible);

		if (auto* Pawn = GetOwningPlayerPawn())
		{
			if (auto* Manager = Pawn->FindComponentByClass<UIInventoryManager>())
			{
				bool bCanTransfer = false;
				if (Manager->CheckEquipmentTransfer(DragOp->ItemMoveData, bCanTransfer))
				{
					HighlightWidgetPreview->SetHighlightState(bCanTransfer ? EHighlightState::Allowed
																		   : EHighlightState::NotAllowed);
					return true;
				}
			}
		}
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
		{
			HighlightWidgetPreview->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

bool USlotbasedInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
											 UDragDropOperation* InOperation)
{
	if (HighlightWidgetPreview)
	{
		HighlightWidgetPreview->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (!InOperation || !SlotsGridPanel || !SlotBasedInventoryRef)
	{
		return false;
	}

	FVector2D ScreenCursorPos = InDragDropEvent.GetScreenSpacePosition();
	FIntPoint GridPosition = CalculateGridPosition(InGeometry, ScreenCursorPos);

	auto ItemCollection = SlotBasedInventoryRef->GetItemCollectionLinked();
	auto InvID = SlotBasedInventoryRef->GetInventoryContainerID();

	//UE_LOG(LogTemp, Log, TEXT("Row: %d, Column: %d"),  GridPosition.X,  GridPosition.Y);
	if (bIsGridPositionValid(GridPosition))
	{
		auto DragOp = Cast<UItemDragDropOperation>(InOperation);
		if (!DragOp || !DoesItemFootprintFitGrid(DragOp->ItemMoveData.SourceItem,
												 DragOp->ItemMoveData.TargetOrientation, GridPosition))
		{
			return false;
		}
		auto TargetSlot = GetSlotByPosition(GridPosition);
		if (!TargetSlot || !TargetSlot->GetSlotData())
		{
			return false;
		}

		if (auto ItemMap = ItemCollection->FindItemMappingByContainerName(DragOp->ItemMoveData.SourceItem, InvID))
		{
			if (ItemMap->ItemVisualLinked)
			{
				ItemMap->ItemVisualLinked->ChangeOpacity(1.0f);
			}
		}

		DragOp->ItemMoveData.TargetInventory = SlotBasedInventoryRef;
		DragOp->ItemMoveData.TargetSlotID = TargetSlot->GetSlotData()->InventorySlotInfo.SlotGuid;

		if (OnItemDroppedDelegate.IsBound())
		{
			OnItemDroppedDelegate.Broadcast(DragOp->ItemMoveData);
		}

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
		{
			ItemMap->ItemVisualLinked->ChangeOpacity(1.0f);
		}
	}
}
