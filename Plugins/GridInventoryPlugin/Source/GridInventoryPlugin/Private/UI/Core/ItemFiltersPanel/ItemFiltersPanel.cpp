// Nublin Studio 2025 All Rights Reserved.


#include "UI/Core/ItemFiltersPanel/ItemFiltersPanel.h"

#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "UI/Core/Buttons/ItemCategoryButton.h"

void UItemFiltersPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (HorizontalBox && HorizontalBox->GetChildrenCount()> 0)
	{
		for (auto HorizontalChild : HorizontalBox->GetAllChildren())
		{
			if (auto ItemCategoryButton =  Cast<UItemCategoryButton>(HorizontalChild))
			{
				CategoryButtonList.Add(ItemCategoryButton);
			}
		}
	}

	if (VerticalBox && VerticalBox->GetChildrenCount()>0)
	{
		for (auto VerticalChild : VerticalBox->GetAllChildren())
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

void UItemFiltersPanel::DisableAllFilters()
{
	if (CategoryButtonList.IsEmpty())
		return;

	for (auto CategoryButton : CategoryButtonList)
	{
		CategoryButton->SetToggleStatus(false);
	}
}
