//  Nublin Studio 2026 All Rights Reserved.

#include "UI/Inventory/UInventoryBaseWidget.h"

#include "Data/Inventory/InventoryBase.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Components/EditableText.h"
#include "GameFramework/Pawn.h"
#include "Interface/Interaction/ObjectDataProvider.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/HelpersWidgets/ItemTooltipWidget.h"
#include "Utility/InvenzayUtility.h"

UUInventoryBaseWidget::UUInventoryBaseWidget()
{
}

void UUInventoryBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ConfigureTradeFilter();
}

void UUInventoryBaseWidget::NativeDestruct()
{
	if (TradeFilterManager)
	{
		TradeFilterManager->OnTradeSessionItemsChanged.RemoveDynamic(
			this, &UUInventoryBaseWidget::HandleTradeSessionItemsChanged);
	}
	if (ItemFiltersPanel && ItemFiltersPanel->GetRecentlySoldButton())
	{
		ItemFiltersPanel->GetRecentlySoldButton()->OnButtonClicked.RemoveDynamic(
			this, &UUInventoryBaseWidget::HandleRecentSalesClicked);
	}
	TradeFilterManager = nullptr;
	Super::NativeDestruct();
}

void UUInventoryBaseWidget::ConfigureTradeFilter()
{
	if (!TradeFilterManager && GetOwningPlayerPawn())
	{
		TradeFilterManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	}
	if (TradeFilterManager)
	{
		TradeFilterManager->OnTradeSessionItemsChanged.AddUniqueDynamic(
			this, &UUInventoryBaseWidget::HandleTradeSessionItemsChanged);
	}
	if (!ItemFiltersPanel)
	{
		return;
	}
	if (UUIButton* Button = ItemFiltersPanel->GetRecentlySoldButton())
	{
		Button->OnButtonClicked.AddUniqueDynamic(this, &UUInventoryBaseWidget::HandleRecentSalesClicked);
		if (TradeFilterManager && ObservedTradeSessionId != TradeFilterManager->GetTradeSessionId())
		{
			ObservedTradeSessionId = TradeFilterManager->GetTradeSessionId();
			Button->SetToggleStatus(false);
		}
	}
	ItemFiltersPanel->SetRecentSalesAvailable(TradeFilterManager &&
											  TradeFilterManager->IsTradeSessionInventory(InventoryRef));
}

void UUInventoryBaseWidget::HandleTradeSessionItemsChanged()
{
	ConfigureTradeFilter();
	if (InventoryRef)
	{
		RefreshFilteredItemsList();
	}
}

void UUInventoryBaseWidget::HandleRecentSalesClicked(UUIButton* Button)
{
	RefreshFilteredItemsList();
}

bool UUInventoryBaseWidget::PassesRecentSalesFilter(UObject* Item) const
{
	return !ItemFiltersPanel || !ItemFiltersPanel->IsRecentSalesActive() ||
		   (TradeFilterManager && TradeFilterManager->WasSoldThisSession(InventoryRef, Item));
}

bool UUInventoryBaseWidget::PassesCategoryAndSearch(UObject* Item) const
{
	if (!IsValid(Item) || !Item->Implements<UObjectDataProvider>())
	{
		return false;
	}
	const auto Data = IObjectDataProvider::Execute_GetItemRef(Item);
	const FString Search = ItemFiltersPanel && ItemFiltersPanel->GetSearchText()
							   ? ItemFiltersPanel->GetSearchText()->GetText().ToString()
							   : FString();
	const bool bUseCategory = Search.IsEmpty() || !ItemFiltersPanel || ItemFiltersPanel->IsSearchInFilteredSlots();
	return (!bUseCategory || ActiveFilters.IsEmpty() || ActiveFilters.Contains(Data.ItemCategory)) &&
		   (Search.IsEmpty() || Data.ItemTextData.DisplayName.ToString().Contains(Search, ESearchCase::IgnoreCase));
}

bool UUInventoryBaseWidget::HasActiveItemFilters() const
{
	return !ActiveFilters.IsEmpty() ||
		   (ItemFiltersPanel &&
			(ItemFiltersPanel->IsRecentSalesActive() ||
			 (ItemFiltersPanel->GetSearchText() && !ItemFiltersPanel->GetSearchText()->GetText().IsEmpty())));
}

void UUInventoryBaseWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (ItemFiltersPanel && bShowFilterPanel == false)
	{
		ItemFiltersPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UUInventoryBaseWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	GlobalSettings = UInvenzayUtility::GetInvenzaGlobalSettings(GetWorld());
}

void UUInventoryBaseWidget::CreateTooltipWidget()
{
	if (!InventoryRef)
	{
		return;
	}

	auto InvSettings = InventoryRef->GetInventorySettings();

	if (!InvSettings.bShowItemTooltips || !UISettings.ItemTooltipWidgetClass)
	{
		return;
	}

	if (ItemTooltipWidget)
	{
		ItemTooltipWidget->RemoveFromParent();
		SetToolTip(nullptr);
		ItemTooltipWidget = nullptr;
	}

	ItemTooltipWidget = CreateWidget<UItemTooltipWidget>(this, UISettings.ItemTooltipWidgetClass);
	SetToolTip(ItemTooltipWidget);
	ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
}
