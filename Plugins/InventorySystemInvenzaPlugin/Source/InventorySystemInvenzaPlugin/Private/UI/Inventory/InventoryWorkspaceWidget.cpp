//  Nublin Studio 2026 All Rights Reserved.

#include "UI/Inventory/InventoryWorkspaceWidget.h"
#include "UI/Inventory/InventoryWorkspaceRowWidget.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "Components/EditableTextBox.h"
#include "Components/NamedSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"

UInventoryWorkspaceWidget::UInventoryWorkspaceWidget()
{
}

void UInventoryWorkspaceWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SearchText->OnTextChanged.AddUniqueDynamic(this, &ThisClass::FilterChanged);
	ClearSearch->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::ClearClicked);
	for (const auto& Pair : CollapseButtons)
	{
		Pair.Key->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::CollapseClicked);
	}
	for (auto Widget : InstalledWidgets)
	{
		ApplyContainerOptions(Widget.Get());
	}
	ApplyContainerOptions(PlayerWidget);
	UpdatePlayerColumnWidth();
	FilterChanged(SearchText->GetText());
}

void UInventoryWorkspaceWidget::NativeDestruct()
{
	SearchText->OnTextChanged.RemoveDynamic(this, &ThisClass::FilterChanged);
	ClearSearch->OnButtonClicked.RemoveDynamic(this, &ThisClass::ClearClicked);
	for (const auto& Pair : CollapseButtons)
	{
		Pair.Key->OnButtonClicked.RemoveDynamic(this, &ThisClass::CollapseClicked);
	}
	RestoreAllContainerOptions();
	Super::NativeDestruct();
}

void UInventoryWorkspaceWidget::ConfigureWorkspace(const FInventoryWorkspaceSettings& Settings)
{
	const bool bRebuildRows = DisplaySettings.bAllowInventoryCollapse != Settings.bAllowInventoryCollapse;
	DisplaySettings = Settings;
	if (bRebuildRows && !InstalledWidgets.IsEmpty())
	{
		TArray<UInventoryContainerWidget*> Widgets;
		for (auto Widget : InstalledWidgets)
		{
			Widgets.Add(Widget.Get());
		}
		const TArray<FText> Titles = RowTitles;
		InstalledWidgets.Reset();
		SetInventoryWidgets(Widgets, Titles);
	}
	else
	{
		for (auto Widget : InstalledWidgets)
		{
			ApplyContainerOptions(Widget.Get());
		}
	}
	ApplyContainerOptions(PlayerWidget);
	UpdatePlayerColumnWidth();
}

void UInventoryWorkspaceWidget::SetInventoryWidgets(const TArray<UInventoryContainerWidget*>& Widgets,
													const TArray<FText>& Titles)
{
	TArray<UInventoryContainerWidget*> ValidWidgets;
	TArray<FText> ValidTitles;
	for (int32 Index = 0; Index < Widgets.Num(); ++Index)
	{
		if (!IsValid(Widgets[Index]))
		{
			continue;
		}
		ValidWidgets.Add(Widgets[Index]);
		ValidTitles.Add(Titles.IsValidIndex(Index) ? Titles[Index] : FText::GetEmpty());
	}
	bool bSame = InstalledWidgets.Num() == ValidWidgets.Num();
	for (int32 Index = 0; bSame && Index < ValidWidgets.Num(); ++Index)
	{
		bSame = InstalledWidgets[Index] == ValidWidgets[Index] && RowTitles[Index].EqualTo(ValidTitles[Index]);
	}
	if (bSame)
	{
		for (auto* Widget : ValidWidgets)
		{
			ApplyContainerOptions(Widget);
		}
		UpdatePlayerColumnWidth();
		FilterChanged(SearchText->GetText());
		return;
	}

	for (auto It = OriginalCloseButtonSettings.CreateIterator(); It; ++It)
	{
		auto* Widget = It.Key().Get();
		if (Widget != PlayerWidget && !ValidWidgets.Contains(Widget))
		{
			if (IsValid(Widget))
			{
				Widget->SetShowCloseButton(It.Value());
				Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			}
			CollapsedInventories.Remove(It.Key());
			It.RemoveCurrent();
		}
	}
	for (auto It = CollapsedInventories.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}

	const float PreviousOffset = Inventories->GetScrollOffset();
	for (const auto& Pair : CollapseButtons)
	{
		Pair.Key->OnButtonClicked.RemoveDynamic(this, &ThisClass::CollapseClicked);
	}
	CollapseButtons.Reset();
	CollapseArrows.Reset();
	InstalledWidgets.Reset();
	Inventories->ClearChildren();
	Rows.Reset();
	RowTitles.Reset();

	for (int32 Index = 0; Index < ValidWidgets.Num(); ++Index)
	{
		auto* Container = ValidWidgets[Index];
		const FText& Title = ValidTitles[Index];
		const auto* Settings = GetDefault<UInvenzaInventoryDeveloperSettings>();
		const auto RowClass = Settings->StorageInventoryRowClass.LoadSynchronous();
		if (!RowClass)
		{
			UE_LOG(LogTemp, Error, TEXT("StorageInventoryRowClass is not configured."));
			continue;
		}
		auto* Row = CreateWidget<UInventoryWorkspaceRowWidget>(this, RowClass);
		Row->ConfigureRow(Title, Container, DisplaySettings.bAllowInventoryCollapse, IsInventoryExpanded(Container));
		if (DisplaySettings.bAllowInventoryCollapse && Row->ToggleButton)
		{
			Row->ToggleButton->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::CollapseClicked);
			CollapseButtons.Add(Row->ToggleButton, Container);
			CollapseArrows.Add(Row->ToggleButton, Row);
		}
		ApplyContainerOptions(Container);
		Container->SetVisibility(IsInventoryExpanded(Container) ? ESlateVisibility::SelfHitTestInvisible
																: ESlateVisibility::Collapsed);
		auto* RowSlot = Cast<UScrollBoxSlot>(Inventories->AddChild(Row));
		RowSlot->SetPadding(InventoryRowPadding);
		RowSlot->SetHorizontalAlignment(HAlign_Left);
		Rows.Add(Row);
		RowTitles.Add(Title);
		InstalledWidgets.Add(Container);
		Container->ReDrawRequest();
	}
	FilterChanged(SearchText->GetText());
	Inventories->SetScrollOffset(PreviousOffset);
	UpdatePlayerColumnWidth();
}

bool UInventoryWorkspaceWidget::IsInventoryExpanded(UInventoryContainerWidget* Widget) const
{
	return !DisplaySettings.bAllowInventoryCollapse || !CollapsedInventories.Contains(Widget);
}

void UInventoryWorkspaceWidget::SetInventoryExpanded(UInventoryContainerWidget* Widget, bool bExpanded)
{
	if (!DisplaySettings.bAllowInventoryCollapse || !IsValid(Widget) || !InstalledWidgets.Contains(Widget))
	{
		return;
	}
	if (bExpanded)
	{
		CollapsedInventories.Remove(Widget);
	}
	else
	{
		CollapsedInventories.Add(Widget);
	}
	Widget->SetVisibility(bExpanded ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	for (const auto& Pair : CollapseButtons)
	{
		if (Pair.Value == Widget)
		{
			UpdateCollapseArrow(Pair.Key, bExpanded);
		}
	}
}

void UInventoryWorkspaceWidget::CollapseClicked(UUIButton* Button)
{
	if (auto* Widget = CollapseButtons.Find(Button))
	{
		SetInventoryExpanded(Widget->Get(), !IsInventoryExpanded(Widget->Get()));
	}
}

void UInventoryWorkspaceWidget::UpdateCollapseArrow(UUIButton* Button, bool bExpanded)
{
	if (auto* Arrow = CollapseArrows.Find(Button))
	{
		(*Arrow)->SetExpanded(bExpanded);
	}
}

void UInventoryWorkspaceWidget::ApplyContainerOptions(UInventoryContainerWidget* Widget)
{
	if (!IsValid(Widget))
	{
		return;
	}
	if (DisplaySettings.bHideInventoryCloseButtons)
	{
		if (!OriginalCloseButtonSettings.Contains(Widget))
		{
			OriginalCloseButtonSettings.Add(Widget, Widget->GetShowCloseButton());
		}
		Widget->SetShowCloseButton(false);
	}
	else
	{
		RestoreContainerOptions(Widget);
	}
}

void UInventoryWorkspaceWidget::RestoreContainerOptions(UInventoryContainerWidget* Widget)
{
	if (IsValid(Widget))
	{
		if (const bool* Original = OriginalCloseButtonSettings.Find(Widget))
		{
			Widget->SetShowCloseButton(*Original);
		}
	}
	OriginalCloseButtonSettings.Remove(Widget);
}

void UInventoryWorkspaceWidget::RestoreAllContainerOptions()
{
	for (const auto& Pair : OriginalCloseButtonSettings)
	{
		if (Pair.Key.IsValid())
		{
			Pair.Key->SetShowCloseButton(Pair.Value);
		}
	}
	OriginalCloseButtonSettings.Reset();
}

void UInventoryWorkspaceWidget::UpdatePlayerColumnWidth()
{
	auto* ColumnSize = Cast<USizeBox>(GetWidgetFromName(TEXT("PlayerColumnSize")));
	if (!ColumnSize)
	{
		return;
	}
	float Width = FMath::Max(0.f, DisplaySettings.PlayerColumnMinWidth);
	if (DisplaySettings.bFitPlayerColumnToContent && IsValid(PlayerWidget))
	{
		PlayerWidget->TakeWidget();
		PlayerWidget->ForceLayoutPrepass();
		// Include the player's operations and spare space for the vertical scrollbar.
		Width = FMath::Max(Width, PlayerWidget->GetDesiredSize().X + PlayerColumnExtraWidth);
	}
	ColumnSize->SetWidthOverride(Width);
}

void UInventoryWorkspaceWidget::FilterChanged(const FText& Text)
{
	const FString Query = Text.ToString().TrimStartAndEnd();
	int32 Visible = 0;
	for (int32 Index = 0; Index < Rows.Num(); ++Index)
	{
		const bool bMatch = Query.IsEmpty() || RowTitles[Index].ToString().Contains(Query, ESearchCase::IgnoreCase);
		Rows[Index]->SetVisibility(bMatch ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		Visible += bMatch;
	}
	EmptyLabel->UpdateText(
		Rows.IsEmpty() ? GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("StorageAccess.Empty"))
					   : GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("StorageAccess.NoMatches")));
	EmptyLabel->SetVisibility(Visible ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}

void UInventoryWorkspaceWidget::ClearClicked(UUIButton*)
{
	SearchText->SetText(FText::GetEmpty());
	FilterChanged(FText::GetEmpty());
}

void UInventoryWorkspaceWidget::SetPlayerWidget(UInventoryContainerWidget* Widget)
{
	if (PlayerWidget == Widget && PlayerSlot->GetContent() == Widget)
	{
		ApplyContainerOptions(Widget);
		UpdatePlayerColumnWidth();
		return;
	}
	ReleasePlayerWidget();
	PlayerWidget = Widget;
	if (IsValid(Widget))
	{
		ApplyContainerOptions(Widget);
		Widget->RemoveFromParent();
		Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		PlayerSlot->AddChild(Widget);
		Widget->ReDrawRequest();
	}
	UpdatePlayerColumnWidth();
}

UInventoryContainerWidget* UInventoryWorkspaceWidget::ReleasePlayerWidget()
{
	auto* Result = PlayerWidget.Get();
	PlayerSlot->ClearChildren();
	RestoreContainerOptions(Result);
	PlayerWidget = nullptr;
	return Result;
}
