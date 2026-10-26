// Nublin Studio 2026 All Rights Reserved.

#include "UI/Core/ItemFiltersPanel/FiltersPanel.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "UI/Core/Buttons/UIButton.h"

void UFiltersPanel::NativePreConstruct()
{
	Super::NativePreConstruct();
	SetRecentSalesAvailable(false);

	if (!bIsShowFiltersButtons && FilterButtons)
	{
		FilterButtons->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (!bIsShowSearchField && SearchText)
	{
		SearchText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFiltersPanel::NativeConstruct()
{
	Super::NativeConstruct();

	auto CollectButtons = [&](UPanelWidget* Container)
	{
		if (!Container)
		{
			return;
		}
		for (int32 i = 0; i < Container->GetChildrenCount(); ++i)
		{
			if (auto TagButton = Cast<UUIButton>(Container->GetChildAt(i)))
			{
				if (TagButton != RecentlySoldButton)
				{
					CategoryButtonList.Add(TagButton);
				}
			}
		}
	};

	CategoryButtonList.Empty();
	CollectButtons(FilterButtons);
	CollectButtons(VerticalContentBox);

	if (ClearFiltersButton)
	{
		ClearFiltersButton->MainButton->OnPressed.AddUniqueDynamic(this, &UFiltersPanel::OnClearFiltersButtonPressed);
	}

	if (ClearSearchTextButton)
	{
		ClearSearchTextButton->MainButton->OnPressed.AddUniqueDynamic(this,
																	  &UFiltersPanel::OnClearSearchTextButtonPressed);
	}
}

FGameplayTagContainer UFiltersPanel::GetActiveFilterTags() const
{
	FGameplayTagContainer ActiveTags;
	for (auto Button : CategoryButtonList)
	{
		if (Button && Button->GetToggleStatus())
		{
			ActiveTags.AddTag(Button->GetBtnTag());
		}
	}
	return ActiveTags;
}

void UFiltersPanel::DisableAllFilters()
{
	if (RecentlySoldButton)
	{
		RecentlySoldButton->SetToggleStatus(false);
	}
	if (SearchText)
	{
		SearchText->SetText(FText::GetEmpty());
	}
	for (auto CategoryButton : CategoryButtonList)
	{
		CategoryButton->SetToggleStatus(false);
	}
}

void UFiltersPanel::OnClearFiltersButtonPressed()
{
	DisableAllFilters();
}

void UFiltersPanel::OnClearSearchTextButtonPressed()
{
	if (!SearchText)
	{
		return;
	}

	SearchText->SetText(FText::GetEmpty());
	SearchText->OnTextChanged.Broadcast(FText::GetEmpty());
}

void UFiltersPanel::SetRecentSalesAvailable(bool bAvailable)
{
	if (!RecentlySoldButton)
	{
		return;
	}
	RecentlySoldButton->SetVisibility(bAvailable ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (!bAvailable)
	{
		RecentlySoldButton->SetToggleStatus(false);
	}
}

bool UFiltersPanel::IsRecentSalesActive() const
{
	return RecentlySoldButton && RecentlySoldButton->GetVisibility() != ESlateVisibility::Collapsed &&
		   RecentlySoldButton->GetToggleStatus();
}
