// Nublin Studio 2026 All Rights Reserved.

#include "UI/Craft/CraftMenuDetail.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "Components/ListView.h"
#include "Components/WidgetSwitcher.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "Data/CraftSystem/Entries/RecipeRequiredIListEntryObject.h"
#include "UI/Core/Image/ImageBaseWidget.h"
#include "UI/Craft/Lists/ReceptDetailRequiredListSimple.h"
#include "UI/Craft/CraftingQuantitySelector.h"
#include "UI/Core/LabelBaseText.h"
#include "Data/ItemData.h"

UCraftMenuDetail::UCraftMenuDetail()
{
}

void UCraftMenuDetail::NativeConstruct()
{
	Super::NativeConstruct();
}

void UCraftMenuDetail::SetCraftDetail(FItemRecipeRow RecipeRow, FRecipeCheckResult CheckResult)
{
	CurrentRecipe = RecipeRow;
	if (RecipeName)
	{
		RecipeName->UpdateText(RecipeRow.DisplayName);
	}
	if (RecipeDescription)
	{
		RecipeDescription->UpdateText(RecipeRow.Description);
		RecipeDescription->SetVisibility(RecipeRow.Description.IsEmpty() ? ESlateVisibility::Collapsed
																		 : ESlateVisibility::HitTestInvisible);
	}
	const int32 Quantity = CraftingQuantitySelector ? CraftingQuantitySelector->GetCurrentQuantity() : 1;
	if (RecipeWorkAmount)
	{
		RecipeWorkAmount->UpdateText(
			FText::Format(GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("CraftMenu.WorkAmount")),
						  FText::AsNumber(static_cast<double>(RecipeRow.CraftVolume) * Quantity)));
	}
	if (RecipeOutput)
	{
		TArray<FString> Outputs;
		for (const FInitItemsEntry& Entry : RecipeRow.OutputItems)
		{
			const FItemData* Item = Entry.Item.GetRow<FItemData>(TEXT("CraftMenuOutput"));
			if (!Item || Entry.Amount <= 0)
			{
				continue;
			}
			Outputs.Add(
				FText::Format(GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("CraftMenu.OutputAmount")),
							  Item->ItemMetaData.ItemTextData.DisplayName,
							  FText::AsNumber(static_cast<int64>(Entry.Amount) * Quantity))
					.ToString());
		}
		RecipeOutput->UpdateText(
			Outputs.IsEmpty() ? GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("CraftMenu.NoOutput"))
							  : FText::FromString(FString::Join(Outputs, TEXT("\n"))));
	}
	if (NoRequirementsText)
	{
		NoRequirementsText->SetVisibility(CheckResult.Requirements.IsEmpty() ? ESlateVisibility::HitTestInvisible
																			 : ESlateVisibility::Collapsed);
	}
	if (RecipeImage)
	{
		UTexture2D* Texture = RecipeRow.RecipeIcon.LoadSynchronous();
		RecipeImage->UpdateImage(Texture);
		RecipeImage->SetRenderOpacity(Texture ? 1.f : 0.f);
	}

	if (RecipeDetailRequiredListSimple)
	{
		RecipeDetailRequiredListSimple->RefreshRequiredList(RecipeRow, CheckResult.Requirements);
	}
	OnRecipeDataChanged();
}

void UCraftMenuDetail::ClearDetail()
{
	CurrentRecipe = FItemRecipeRow();
	if (RecipeName)
	{
		RecipeName->ClearText();
	}
	if (RecipeDescription)
	{
		RecipeDescription->ClearText();
	}
	if (RecipeWorkAmount)
	{
		RecipeWorkAmount->ClearText();
	}
	if (RecipeOutput)
	{
		RecipeOutput->ClearText();
	}
	if (RecipeDetailRequiredListSimple)
	{
		RecipeDetailRequiredListSimple->ClearRequirements();
	}
	if (RecipeImage)
	{
		RecipeImage->UpdateImage(nullptr);
		RecipeImage->SetRenderOpacity(0.f);
	}
	OnRecipeDataChanged();
}

void UCraftMenuDetail::OnClickedTabRecipeRequireds(UUIButton* ButtonPressed)
{
	if (RecipeTabsSwitcher && RecipeDetailRequiredListSimple)
	{
		RecipeTabsSwitcher->SetActiveWidget(RecipeDetailRequiredListSimple);
	}
}

void UCraftMenuDetail::OnClickedTabRecipeDescription(UUIButton* ButtonPressed)
{
}
