// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/CraftMenuChoose.h"

#include "Components/WidgetSwitcher.h"
#include "Data/CraftSystem/Entries/RecipeListEntryObject.h"
#include "UI/Core/Buttons/ActionButtonUI.h"
#include "UI/Craft/CraftingQuantitySelector.h"
#include "UI/Craft/CraftMenuDetail.h"
#include "UI/Craft/CraftMenuRecipeActions.h"
#include "Data/ItemDataStructures.h"
#include "UI/Core/MovableTitleBar/MovableTitleBar.h"
#include "UI/Craft/Lists/CraftRecipesList.h"
#include "UI/Craft/Lists/ReceptDetailRequiredListSimple.h"


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

	if (MovableTitleBar)
	{
		MovableTitleBar->Button_Close->OnButtonClicked.AddDynamic(this, &UCraftMenuChoose::OnBtnClosePressed);
	}

	if (CraftRecipesList && CraftRecipesList->ObjectList)
	{
		 CraftRecipesList->ObjectList->OnItemSelectionChanged().AddUObject(this, &UCraftMenuChoose::HandleItemSelectionChanged);
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
}

void UCraftMenuChoose::SetAvailableRecipes(const TArray<FItemRecipeRow>& Recipes)
{
	CraftRecipesList->SetRecipes(Recipes);
}

void UCraftMenuChoose::SetCraftComponentPtr(UCraftingComponent* NewCraftingComponent)
{	
	if (CraftComponentPtr)
	{
		CraftComponentPtr->OnAvailableRecipesChanged.RemoveAll(this);
	}

	CraftComponentPtr = NewCraftingComponent;
	SelectedObj = nullptr;

	if (CraftComponentPtr)
	{
		CraftComponentPtr->OnAvailableRecipesChanged.AddDynamic(this, &UCraftMenuChoose::HandleAvailableRecipesChanged);
		SetAvailableRecipes(CraftComponentPtr->GetAvailableRecipes());
	}
	else
	{
		SetAvailableRecipes(TArray<FItemRecipeRow>());
	}

	RefreshCurrentSelectionDetails();
}

void UCraftMenuChoose::HandleAvailableRecipesChanged()
{
	if (!CraftComponentPtr) return;
	
	SetAvailableRecipes(CraftComponentPtr->GetAvailableRecipes());
	
	RefreshCurrentSelectionDetails();
}

void UCraftMenuChoose::RefreshCurrentSelectionDetails()
{
	if (!SelectedObj || SelectedObj->RecipeRow.ID.IsNone())
	{
		WidgetSwitcher->SetActiveWidget(static_cast<UWidget*>(EmptySelectionText));
		return;
	}

	int32 CurrentAmount = CraftMenuDetail->CraftingQuantitySelector->GetCurrentQuantity();

	if (!CraftComponentPtr)
	{
		TArray<FItemIDEntry> InventoryItemsEmpty;
		auto CheckResult = UCraftingComponent::CanCraftWithItems(SelectedObj->RecipeRow, InventoryItemsEmpty, CurrentAmount);
		CraftMenuDetail->SetCraftDetail(SelectedObj->RecipeRow, CheckResult);
		WidgetSwitcher->SetActiveWidget(CraftMenuDetail);
		return;
	}

	FRecipeCheckResult CachedCheckResult;
	if (CraftComponentPtr->GetCachedResultForRecipe(SelectedObj->RecipeRow.ID, CachedCheckResult))
	{
		if (CurrentAmount > 1)
		{
			CachedCheckResult = CraftComponentPtr->CanCraft(SelectedObj->RecipeRow,SelectedOptions, CurrentAmount);
		}
        
		CraftMenuDetail->SetCraftDetail(SelectedObj->RecipeRow, CachedCheckResult);
	}
	else
	{
		auto DirectResult = CraftComponentPtr->CanCraft(SelectedObj->RecipeRow,SelectedOptions, CurrentAmount);
		CraftMenuDetail->SetCraftDetail(SelectedObj->RecipeRow, DirectResult);
	}

	WidgetSwitcher->SetActiveWidget(CraftMenuDetail);
}

void UCraftMenuChoose::HandleItemSelectionChanged(UObject* Item)
{
	if (auto RecipeListEntryObject = Cast<URecipeListEntryObject>(Item))
	{
		SelectedObj = RecipeListEntryObject;
		CraftMenuDetail->CraftingQuantitySelector->SetToMin(CraftMenuDetail->CraftingQuantitySelector->Btn_SetMin);
       
		RefreshCurrentSelectionDetails();
	}
	else
	{
		SelectedObj = nullptr;
		WidgetSwitcher->SetActiveWidget(static_cast<UWidget*>(EmptySelectionText));
	}
}

void UCraftMenuChoose::HandleOnCraftAmountChanged(int32 NewAmount)
{
	RefreshCurrentSelectionDetails();
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
	SelectedOptions = CraftMenuDetail->RecipeDetailRequiredListSimple->GetAllSelectedOptions();
	
	OnCraftRequested.Broadcast(SelectedObj->RecipeRow, AmountToCraft, SelectedOptions);
	
	if (CraftComponentPtr)
	{
		CraftComponentPtr->EnqueueRecipeRequest(SelectedObj->RecipeRow,SelectedOptions, AmountToCraft);
	}
}
