// Nublin Studio 2026 All Rights Reserved.

#include "UI/Craft/CraftMenuDetail.h"

#include "ActorComponents/Crafting/CraftingTypes.h"
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

	/*if (RecipeDetailTabs)
	{
		RecipeDetailTabs->Btn_Requirements->OnButtonClicked.AddDynamic(this, &UCraftMenuDetail::OnClickedTabRecipeRequireds);
		RecipeDetailTabs->Btn_Description->OnButtonClicked.AddDynamic(this, &UCraftMenuDetail::OnClickedTabRecipeDescription);
	}*/
}

void UCraftMenuDetail::SetCraftDetail(FItemRecipeRow RecipeRow, FRecipeCheckResult CheckResult)
{
	if (RecipeImage)
		RecipeImage->UpdateImage(RecipeRow.RecipeIcon.Get());

	if (!RecipeRequiredListEntryObjectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCraftMenuDetail::SetCraftDetail - RecipeRequiredListEntryObjectClass is not set!"));
		return;
	}

	RecipeDetailRequiredListSimple->RequiredList->ClearListItems();
	if (CheckResult.Requirements.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("UCraftMenuDetail::SetCraftDetail - CheckResult.Requirements is empty!"));
	}
	else
	{
		int32 Index = 0;
		for (const auto& Req : CheckResult.Requirements)
		{
			auto ItemObj = NewObject<URecipeRequiredIListEntryObject>(this, RecipeRequiredListEntryObjectClass);

			ItemObj->RecipeRow = RecipeRow;
			ItemObj->RecipeCheckResult = Req;
			ItemObj->Index = Index;

			RecipeDetailRequiredListSimple->RequiredList->AddItem(ItemObj);

			++Index;
		}
	}
}

void UCraftMenuDetail::OnClickedTabRecipeRequireds(UUIButton* ButtonPressed)
{
	RecipeTabsSwitcher->SetActiveWidget(RecipeDetailRequiredListSimple);
}

void UCraftMenuDetail::OnClickedTabRecipeDescription(UUIButton* ButtonPressed)
{
	
}
