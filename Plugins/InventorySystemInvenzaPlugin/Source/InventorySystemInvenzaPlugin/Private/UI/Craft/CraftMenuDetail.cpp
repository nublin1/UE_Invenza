// Nublin Studio 2026 All Rights Reserved.

#include "UI/Craft/CraftMenuDetail.h"
#include "Components/ListView.h"
#include "Components/WidgetSwitcher.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "Data/CraftSystem/Entries/RecipeRequiredIListEntryObject.h"
#include "UI/Core/Image/ImageBaseWidget.h"
#include "UI/Craft/Lists/ReceptDetailRequiredListSimple.h"


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
	if (RecipeDetailRequiredListSimple) RecipeDetailRequiredListSimple->ClearRequirements();
	if (RecipeImage) { RecipeImage->UpdateImage(nullptr); RecipeImage->SetRenderOpacity(0.f); }
	OnRecipeDataChanged();
}

void UCraftMenuDetail::OnClickedTabRecipeRequireds(UUIButton* ButtonPressed)
{
	if (RecipeTabsSwitcher && RecipeDetailRequiredListSimple) RecipeTabsSwitcher->SetActiveWidget(RecipeDetailRequiredListSimple);
}

void UCraftMenuDetail::OnClickedTabRecipeDescription(UUIButton* ButtonPressed)
{
	
}
