// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/CraftMenuChoose.h"

#include "Components/WidgetSwitcher.h"
#include "Data/CraftSystem/Entries/RecipeListEntryObject.h"
#include "UI/Core/Buttons/ActionButtonUI.h"
#include "UI/Craft/CraftingQuantitySelector.h"
#include "UI/Craft/CraftMenuDetail.h"
#include "UI/Craft/CraftMenuRecipeActions.h"
#include "Data/ItemDataStructures.h"
#include "UI/Craft/Lists/CraftRecipesList.h"


UCraftMenuChoose::UCraftMenuChoose()
{
}

void UCraftMenuChoose::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UCraftMenuChoose::NativeConstruct()
{
	Super::NativeConstruct();

	if (CraftRecipesList && CraftRecipesList->AvailableRecipesList)
	{
		 CraftRecipesList->AvailableRecipesList->OnItemSelectionChanged().AddUObject(this,	&UCraftMenuChoose::HandleItemSelectionChanged);
	}
	if (CraftMenuDetail)
	{
		CraftMenuDetail->CraftingQuantitySelector->OnQuantityChanged.AddDynamic(this, &UCraftMenuChoose::HandleOnCraftAmountChanged);
		CraftMenuDetail->CraftMenuActionButtons->Btn_Craft->OnButtonClicked.AddDynamic(this, &UCraftMenuChoose::CraftBtnPressed);
	}

	if (WidgetSwitcher)
	{
		WidgetSwitcher->SetActiveWidget(static_cast<UWidget*>(EmptySelectionText));
	}

	if (Btn_Close)
	{
		Btn_Close->OnButtonClicked.AddDynamic(this, &UCraftMenuChoose::OnBtnClosePressed);
	}

	CraftComponentPtr = NewObject<UCraftingComponent>();
}

void UCraftMenuChoose::SetAvailableRecipes(const TArray<FItemRecipeRow>& Recipes)
{
	CraftRecipesList->SetRecipes(Recipes);
}

void UCraftMenuChoose::HandleItemSelectionChanged(UObject* Item)
{
	if (auto RecipeListEntryObject = Cast<URecipeListEntryObject>(Item))
	{
		SelectedObj = RecipeListEntryObject;

		CraftMenuDetail->CraftingQuantitySelector->SetToMin(CraftMenuDetail->CraftingQuantitySelector->Btn_SetMin);
		
		if (!CraftComponentPtr)
		{
			UE_LOG(LogTemp, Warning, TEXT("UCraftMenuChoose::HandleItemSelectionChanged - CraftComponentPtr is nullptr!"));

			TArray<FItemIDEntry> InventoryItemsEmpty;
			auto CheckResult = UCraftingComponent::CanCraftWithItems(SelectedObj->RecipeRow, InventoryItemsEmpty);
			CraftMenuDetail->SetCraftDetail(SelectedObj->RecipeRow, CheckResult);
			WidgetSwitcher->SetActiveWidget(CraftMenuDetail);
			return;
		}
		
		auto CheckResult = CraftComponentPtr->CanCraft(SelectedObj->RecipeRow);
		CraftMenuDetail->SetCraftDetail(SelectedObj->RecipeRow, CheckResult);
		WidgetSwitcher->SetActiveWidget(CraftMenuDetail);
	}
	else
	{
		SelectedObj = nullptr;
		WidgetSwitcher->SetActiveWidget(static_cast<UWidget*>(EmptySelectionText));
	}
}

void UCraftMenuChoose::HandleOnCraftAmountChanged(int32 NewAmount)
{
	auto CheckResult = CraftComponentPtr->CanCraft(SelectedObj->RecipeRow, NewAmount);
	CraftMenuDetail->SetCraftDetail(SelectedObj->RecipeRow, CheckResult);
}

void UCraftMenuChoose::OnBtnClosePressed(UUIButton* Btn)
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCraftMenuChoose::CraftBtnPressed(UUIButton* Btn)
{
	if (!SelectedObj || SelectedObj->RecipeRow.ID.IsNone())
		return;

	AmountToCraft = CraftMenuDetail->CraftingQuantitySelector->GetCurrentQuantity();
	
	if (OnCraftRequested.IsBound())
	{
		OnCraftRequested.Broadcast(SelectedObj->RecipeRow, AmountToCraft);
	}

	if (CraftComponentPtr)
	{
		CraftComponentPtr->EnqueueRecipe(SelectedObj->RecipeRow, AmountToCraft);
	}
}
