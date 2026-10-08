// Nublin Studio 2026 All Rights Reserved.
#include "UI/Craft/Lists/CraftRecipesList.h"
#include "Components/EditableText.h"
#include "UI/Core/ItemFiltersPanel/FiltersPanel.h"
#include "UI/Craft/Lists/RecipeListEntryWidget.h"

UCraftRecipesList::UCraftRecipesList() { RecipeEntryObjectClass = URecipeListEntryObject::StaticClass(); }
void UCraftRecipesList::NativePreConstruct() { Super::NativePreConstruct(); }
void UCraftRecipesList::NativeConstruct()
{
	Super::NativeConstruct();
	if (ItemFiltersPanel && ItemFiltersPanel->GetSearchText()) ActiveSearchText = ItemFiltersPanel->GetSearchText()->GetText();
	RefreshList();
}
void UCraftRecipesList::SetRecipes(const TArray<FItemRecipeRow>& Recipes)
{
	RecipesData = Recipes;
	RefreshList();
}
void UCraftRecipesList::RefreshList()
{
	if (!ObjectList || !RecipeEntryObjectClass) return;
	TMap<FName, URecipeListEntryObject*> Existing;
	for (URecipeListEntryObject* Item : ItemsArray) if (Item) Existing.Add(Item->RecipeRow.ID, Item);
	TArray<TObjectPtr<URecipeListEntryObject>> Updated;
	TSet<FName> Seen;
	for (const FItemRecipeRow& Recipe : RecipesData)
	{
		if (Recipe.ID.IsNone() || Seen.Contains(Recipe.ID)) continue;
		Seen.Add(Recipe.ID);
		auto* Item = Existing.FindRef(Recipe.ID);
		if (!Item) Item = NewObject<URecipeListEntryObject>(this, RecipeEntryObjectClass);
		Item->RecipeRow = Recipe;
		Item->Text = Recipe.DisplayName;
		Updated.Add(Item);
		if (auto* Entry = ObjectList->GetEntryWidgetFromItem<URecipeListEntryWidget>(Item)) Entry->NativeOnListItemObjectSet(Item);
	}
	ItemsArray = MoveTemp(Updated);
	FilteredItemsArray.RemoveAll([this](const auto& Item) { return !ItemsArray.Contains(Item); });
	ApplySearch();
}
void UCraftRecipesList::SearchTextChanged(const FText& NewText)
{
	ActiveSearchText = NewText;
	ApplySearch();
}
void UCraftRecipesList::ApplySearch()
{
	if (!ObjectList) return;
	const auto& Source = ItemFiltersPanel && ItemFiltersPanel->IsSearchInFilteredSlots() ? FilteredItemsArray : ItemsArray;
	TArray<UObject*> Visible;
	const FString Search = ActiveSearchText.ToString();
	for (URecipeListEntryObject* Item : Source)
		if (Item && (Search.IsEmpty() || Item->Text.ToString().Contains(Search, ESearchCase::IgnoreCase))) Visible.Add(Item);
	if (UObject* Selection = ObjectList->GetSelectedItem(); Selection && !Visible.Contains(Selection)) ObjectList->ClearSelection();
	if (ObjectList->GetListItems() != Visible) ObjectList->SetListItems(Visible);
	ObjectList->RequestRefresh();
}
