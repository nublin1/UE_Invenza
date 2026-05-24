// Nublin Studio 2026 All Rights Reserved.


#include "UI/Core/ItemFiltersPanel/FiltersPanel.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "UI/Core/Buttons/FilterTagButton.h"

void UFiltersPanel::NativePreConstruct()
{
	Super::NativePreConstruct();
	
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
		if (!Container) return;
		for (int32 i = 0; i < Container->GetChildrenCount(); ++i)
		{
			if (auto TagButton = Cast<UFilterTagButton>(Container->GetChildAt(i)))
			{
				CategoryButtonList.Add(TagButton);
			}
		}
	};

	CategoryButtonList.Empty();
	CollectButtons(FilterButtons);
	CollectButtons(VerticalContentBox);

	if (ClearFiltersButton)
		ClearFiltersButton->MainButton->OnPressed.AddDynamic(this, &UFiltersPanel::OnClearFiltersButtonPressed);

	if (ClearSearchTextButton)
		ClearSearchTextButton->MainButton->OnPressed.AddDynamic(this, &UFiltersPanel::OnClearSearchTextButtonPressed);
}

FGameplayTagContainer UFiltersPanel::GetActiveFilterTags() const
{
	FGameplayTagContainer ActiveTags;
	for (auto Button : CategoryButtonList)
	{
		if (Button && Button->GetToggleStatus()) 
		{
			ActiveTags.AddTag(Button->GetFilterTag());
		}
	}
	return ActiveTags;
}

void UFiltersPanel::DisableAllFilters()
{
	if (CategoryButtonList.IsEmpty())
		return;

	SearchText->SetText(FText::FromString(""));
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
	if(!SearchText)
		return;

	SearchText->SetText(FText::GetEmpty());
}

