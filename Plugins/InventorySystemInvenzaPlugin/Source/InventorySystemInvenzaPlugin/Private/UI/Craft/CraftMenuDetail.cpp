// Nublin Studio 2026 All Rights Reserved.

#include "UI/Craft/CraftMenuDetail.h"

#include "ActorComponents/Crafting/CraftingStructs.h"
#include "Components/ListView.h"
#include "Components/MultiLineEditableTextBox.h"
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
	if (RecipeImage && !RecipeRow.RecipeIcon.IsNull())
	{
		if (UTexture2D* Texture = RecipeRow.RecipeIcon.Get())
		{
			RecipeImage->UpdateImage(Texture);
		}
		else
		{
			RecipeImage->UpdateImage(RecipeRow.RecipeIcon.LoadSynchronous()); 
		}
	}
	
	if (RecipeDetailRequiredListSimple)
	{
		RecipeDetailRequiredListSimple->RefreshRequiredList(RecipeRow, CheckResult.Requirements);
	}
}

void UCraftMenuDetail::OnClickedTabRecipeRequireds(UUIButton* ButtonPressed)
{
	RecipeTabsSwitcher->SetActiveWidget(RecipeDetailRequiredListSimple);
}

void UCraftMenuDetail::OnClickedTabRecipeDescription(UUIButton* ButtonPressed)
{
	
}
