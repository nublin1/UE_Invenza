// Nublin Studio 2025 All Rights Reserved.


#include "UI/Core/ItemFiltersPanel/ItemFiltersPanel.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "UI/Core/Buttons/ItemCategoryButton.h"

void UItemFiltersPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (!bIsShowSearchField && SearchText)
	{
		SearchText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (HorizontalContentBox && HorizontalContentBox->GetChildrenCount()> 0)
	{
		for (auto HorizontalChild : HorizontalContentBox->GetAllChildren())
		{
			if (auto ItemCategoryButton =  Cast<UItemCategoryButton>(HorizontalChild))
			{
				CategoryButtonList.Add(ItemCategoryButton);
			}
		}
	}

	if (VerticalContentBox && VerticalContentBox->GetChildrenCount()>0)
	{
		for (auto VerticalChild : VerticalContentBox->GetAllChildren())
		{
			if (auto ItemCategoryButton =  Cast<UItemCategoryButton>(VerticalChild))
			{
				CategoryButtonList.Add(ItemCategoryButton);
			}
		}
	}

	if (ClearFiltersButton)
		ClearFiltersButton->MainButton->OnPressed.AddDynamic(this, &UItemFiltersPanel::OnClearFiltersButtonPressed);
}

void UItemFiltersPanel::OnClearFiltersButtonPressed()
{
	DisableAllFilters();
}

UItemFiltersPanel::UItemFiltersPanel(): ItemFilterBorderColor(FLinearColor::Green), bSearchInFilteredSlots(false)
{
}

void UItemFiltersPanel::DisableAllFilters()
{
	if (CategoryButtonList.IsEmpty())
		return;

	SearchText->SetText(FText::FromString(""));
	for (auto CategoryButton : CategoryButtonList)
	{
		CategoryButton->SetToggleStatus(false);
	}
}
