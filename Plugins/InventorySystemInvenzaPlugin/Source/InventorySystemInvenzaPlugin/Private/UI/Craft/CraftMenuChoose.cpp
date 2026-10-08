// Nublin Studio 2026 All Rights Reserved.
#include "UI/Craft/CraftMenuChoose.h"
#include "Components/WidgetSwitcher.h"
#include "Data/CraftSystem/Entries/RecipeListEntryObject.h"
#include "UI/Core/Buttons/ActionButtonUI.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Core/MovableTitleBar/MovableTitleBar.h"
#include "UI/Craft/CraftingQuantitySelector.h"
#include "UI/Craft/CraftMenuDetail.h"
#include "UI/Craft/CraftMenuRecipeActions.h"
#include "UI/Craft/Lists/CraftRecipesList.h"
#include "UI/Craft/Lists/ReceptDetailRequiredListSimple.h"

UCraftMenuChoose::UCraftMenuChoose() {}
void UCraftMenuChoose::NativePreConstruct() { Super::NativePreConstruct(); }
void UCraftMenuChoose::NativeConstruct()
{
	Super::NativeConstruct();
	if (MovableTitleBar && MovableTitleBar->Button_Close)
		MovableTitleBar->Button_Close->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::OnBtnClosePressed);
	if (CraftRecipesList && CraftRecipesList->ObjectList)
	{
		CraftRecipesList->ObjectList->OnItemSelectionChanged().RemoveAll(this);
		CraftRecipesList->ObjectList->OnItemSelectionChanged().AddUObject(this, &ThisClass::HandleItemSelectionChanged);
	}
	if (CraftMenuDetail)
	{
		if (CraftMenuDetail->CraftingQuantitySelector)
			CraftMenuDetail->CraftingQuantitySelector->OnQuantityChanged.AddUniqueDynamic(this, &ThisClass::HandleOnCraftAmountChanged);
		if (CraftMenuDetail->CraftMenuActionButtons && CraftMenuDetail->CraftMenuActionButtons->Btn_Craft)
			CraftMenuDetail->CraftMenuActionButtons->Btn_Craft->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::CraftBtnPressed);
		if (CraftMenuDetail->RecipeDetailRequiredListSimple)
			CraftMenuDetail->RecipeDetailRequiredListSimple->OnOptionsChanged.AddUniqueDynamic(this, &ThisClass::HandleOptionsChanged);
	}
	SetCraftComponentPtr(CraftComponentPtr);
}
void UCraftMenuChoose::NativeDestruct()
{
	if (IsValid(CraftComponentPtr)) CraftComponentPtr->OnAvailableRecipesChanged.RemoveDynamic(this, &ThisClass::HandleAvailableRecipesChanged);
	if (MovableTitleBar && MovableTitleBar->Button_Close)
		MovableTitleBar->Button_Close->OnButtonClicked.RemoveDynamic(this, &ThisClass::OnBtnClosePressed);
	if (CraftRecipesList && CraftRecipesList->ObjectList) CraftRecipesList->ObjectList->OnItemSelectionChanged().RemoveAll(this);
	if (CraftMenuDetail)
	{
		if (CraftMenuDetail->CraftingQuantitySelector)
			CraftMenuDetail->CraftingQuantitySelector->OnQuantityChanged.RemoveDynamic(this, &ThisClass::HandleOnCraftAmountChanged);
		if (CraftMenuDetail->CraftMenuActionButtons && CraftMenuDetail->CraftMenuActionButtons->Btn_Craft)
			CraftMenuDetail->CraftMenuActionButtons->Btn_Craft->OnButtonClicked.RemoveDynamic(this, &ThisClass::CraftBtnPressed);
		if (CraftMenuDetail->RecipeDetailRequiredListSimple)
			CraftMenuDetail->RecipeDetailRequiredListSimple->OnOptionsChanged.RemoveDynamic(this, &ThisClass::HandleOptionsChanged);
	}
	Super::NativeDestruct();
}
void UCraftMenuChoose::ResetSelection()
{
	SelectedObj = nullptr;
	SelectedOptions.Reset();
	bRefreshMaximum = true;
	if (CraftMenuDetail)
	{
		CraftMenuDetail->ClearDetail();
		if (CraftMenuDetail->CraftingQuantitySelector)
		{
			CraftMenuDetail->CraftingQuantitySelector->SetResourceMaximum(0);
			CraftMenuDetail->CraftingQuantitySelector->SetToMin(nullptr);
		}
	}
}
void UCraftMenuChoose::SetAvailableRecipes(const TArray<FItemRecipeRow>& Recipes)
{
	if (!CraftRecipesList) return;
	{
		TGuardValue<bool> Guard(bRefreshingDetails, true);
		CraftRecipesList->SetRecipes(Recipes);
		if (SelectedObj && (!CraftRecipesList->ItemsArray.Contains(SelectedObj)
			|| !CraftRecipesList->ObjectList || !CraftRecipesList->ObjectList->GetListItems().Contains(SelectedObj))) ResetSelection();
	}
	bRefreshMaximum = true;
	RefreshCurrentSelectionDetails();
}
void UCraftMenuChoose::SetCraftComponentPtr(UCraftingComponent* NewComponent)
{
	if (IsValid(CraftComponentPtr)) CraftComponentPtr->OnAvailableRecipesChanged.RemoveDynamic(this, &ThisClass::HandleAvailableRecipesChanged);
	{
		TGuardValue<bool> Guard(bRefreshingDetails, true);
		if (CraftComponentPtr != NewComponent)
		{
			ResetSelection();
			if (CraftRecipesList && CraftRecipesList->ObjectList) CraftRecipesList->ObjectList->ClearSelection();
		}
		CraftComponentPtr = IsValid(NewComponent) ? NewComponent : nullptr;
	}
	if (CraftComponentPtr)
	{
		CraftComponentPtr->OnAvailableRecipesChanged.AddUniqueDynamic(this, &ThisClass::HandleAvailableRecipesChanged);
		SetAvailableRecipes(CraftComponentPtr->GetAvailableRecipes());
	}
	else RefreshCurrentSelectionDetails();
}
void UCraftMenuChoose::HandleAvailableRecipesChanged()
{
	if (IsValid(CraftComponentPtr)) SetAvailableRecipes(CraftComponentPtr->GetAvailableRecipes());
}
void UCraftMenuChoose::RefreshCurrentSelectionDetails()
{
	if (bRefreshingDetails || !WidgetSwitcher || !CraftMenuDetail) return;
	TGuardValue<bool> Guard(bRefreshingDetails, true);
	if (!IsValid(SelectedObj) || SelectedObj->RecipeRow.ID.IsNone())
	{
		if (EmptySelectionText) WidgetSwitcher->SetActiveWidget(EmptySelectionText);
		return;
	}
	auto* Quantity = CraftMenuDetail->CraftingQuantitySelector.Get();
	const FItemRecipeRow& Recipe = SelectedObj->RecipeRow;
	SelectedOptions.SetNumZeroed(Recipe.RequiredItems.Num());
	for (int32 Index = 0; Index < SelectedOptions.Num(); ++Index)
		SelectedOptions[Index] = FMath::Clamp(SelectedOptions[Index], 0, Recipe.RequiredItems[Index].Alternatives.Num());
	const int32 Amount = Quantity ? Quantity->GetCurrentQuantity() : 1;
	const FRecipeCheckResult Check = IsValid(CraftComponentPtr)
		? CraftComponentPtr->CanCraft(Recipe, SelectedOptions, Amount)
		: UCraftingComponent::CanCraftWithItemsOptions(Recipe, {}, SelectedOptions, Amount);
	CraftMenuDetail->SetCraftDetail(Recipe, Check);
	if (Quantity && bRefreshMaximum)
	{
		Quantity->SetResourceMaximum(IsValid(CraftComponentPtr) ? CraftComponentPtr->GetMaxCraftAmount(Recipe, SelectedOptions) : 0);
		bRefreshMaximum = false;
	}
	WidgetSwitcher->SetActiveWidget(CraftMenuDetail);
}
void UCraftMenuChoose::HandleItemSelectionChanged(UObject* Item)
{
	if (bRefreshingDetails) return;
	{
		TGuardValue<bool> Guard(bRefreshingDetails, true);
		auto* RecipeItem = Cast<URecipeListEntryObject>(Item);
		if (SelectedObj != RecipeItem)
		{
			ResetSelection();
			SelectedObj = RecipeItem;
			if (SelectedObj) SelectedOptions.Init(0, SelectedObj->RecipeRow.RequiredItems.Num());
		}
	}
	RefreshCurrentSelectionDetails();
}
void UCraftMenuChoose::HandleOptionsChanged()
{
	if (bRefreshingDetails || !CraftMenuDetail || !CraftMenuDetail->RecipeDetailRequiredListSimple) return;
	SelectedOptions = CraftMenuDetail->RecipeDetailRequiredListSimple->GetAllSelectedOptions();
	bRefreshMaximum = true;
	RefreshCurrentSelectionDetails();
}
void UCraftMenuChoose::HandleOnCraftAmountChanged(int32 NewAmount) { RefreshCurrentSelectionDetails(); }
void UCraftMenuChoose::OnBtnClosePressed(UUIButton* Btn) { SetVisibility(ESlateVisibility::Collapsed); }
void UCraftMenuChoose::CraftBtnPressed(UUIButton* Btn)
{
	if (!CraftMenuDetail || !CraftMenuDetail->CraftingQuantitySelector) return;
	CraftMenuDetail->CraftingQuantitySelector->CommitPendingQuantity();
	if (!IsValid(SelectedObj) || SelectedObj->RecipeRow.ID.IsNone()) return;
	AmountToCraft = CraftMenuDetail->CraftingQuantitySelector->GetCurrentQuantity();
	if (AmountToCraft <= 0) return;
	// Copy before broadcasting: listeners can close the menu or change its selection.
	const FItemRecipeRow Recipe = SelectedObj->RecipeRow;
	const TArray<int32> Options = SelectedOptions;
	const int32 Amount = AmountToCraft;
	TWeakObjectPtr<UCraftingComponent> Component = CraftComponentPtr;
	OnCraftRequested.Broadcast(Recipe, Amount, Options);
	if (Component.IsValid()) Component->EnqueueRecipeRequest(Recipe, Options, Amount);
}
