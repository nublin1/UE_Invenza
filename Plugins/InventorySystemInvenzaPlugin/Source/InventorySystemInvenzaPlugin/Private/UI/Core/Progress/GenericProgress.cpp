// Nublin Studio 2026 All Rights Reserved.


#include "UI/Core/Progress/GenericProgress.h"

#include "Data/CraftSystem/CraftingStructs.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Core/Image/ImageBaseWidget.h"
#include "UI/Core/Progress/ProgressPercentTimer.h"
#include "UI/Craft/CraftingQuantitySelector.h"

UGenericProgress::UGenericProgress()
{
}

void UGenericProgress::SetNewCraft(const FQueuedRecipe& NewQueuedRecipe)
{
	CurrentItemRecipeRow = NewQueuedRecipe.ItemRecipeRow;
	
	if (Icon && NewQueuedRecipe.ItemRecipeRow.RecipeIcon)
	{
		Icon->UpdateImage(CurrentItemRecipeRow.RecipeIcon.Get());
		DisplayName->UpdateText(CurrentItemRecipeRow.DisplayName);
	}

	if (ProgressPercentTimerWidget->Percent)
	{
		auto MaxValue = CurrentItemRecipeRow.CraftVolume;
		auto CurrentWorkAmount = NewQueuedRecipe.CurrentProgress;
		auto Percent = (CurrentWorkAmount / MaxValue) * 100.f;
		FString PercentString = FString::Printf(TEXT("%f%%"), Percent);
		ProgressPercentTimerWidget->SetPercentText(PercentString);
	}

	if (ProgressPercentTimerWidget->ProgressBar)
	{
		ProgressPercentTimerWidget->SetProgressPercent("0");
	}

	if (CraftingQuantitySelectorMini)
	{
		CraftingQuantitySelectorMini->SetQuantity(NewQueuedRecipe.Count);
	}
}

void UGenericProgress::UpdateProgress(float NewValue)
{
	auto MaxValue = CurrentItemRecipeRow.CraftVolume;
	auto CurrentWorkAmount   =NewValue;
	auto Percent = (CurrentWorkAmount / MaxValue) * 100.f;
	FString PercentString = FString::Printf(TEXT("%f%%"), Percent);
	ProgressPercentTimerWidget->SetProgressPercent(PercentString);
}

