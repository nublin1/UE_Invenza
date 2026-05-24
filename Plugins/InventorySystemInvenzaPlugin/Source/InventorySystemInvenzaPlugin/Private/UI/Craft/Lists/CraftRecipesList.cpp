// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/Lists/CraftRecipesList.h"

#include "Components/EditableText.h"
#include "Data/CraftSystem/Entries/RecipeListEntryObject.h"
#include "UI/Core/ItemFiltersPanel/FiltersPanel.h"


UCraftRecipesList::UCraftRecipesList()
{
}

void UCraftRecipesList::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UCraftRecipesList::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemFiltersPanel && ItemFiltersPanel->GetSearchText())
	{
		ItemFiltersPanel->GetSearchText()->OnTextChanged.AddDynamic(this, &UCraftRecipesList::SearchTextChanged);
	}
}

void UCraftRecipesList::SetRecipes(const TArray<FItemRecipeRow>& Recipes)
{
	RecipesData = Recipes;
	RefreshList();
}

void UCraftRecipesList::RefreshList()
{
	if (!AvailableRecipesList) return;

	if (!RecipeEntryObjectClass)
	{
		UE_LOG(LogTemp, Error, TEXT("EquipItem: RecipeEntryObjectClass is NULL! Cannot create recipe entry objects."));
	}

	AvailableRecipesList->ClearListItems();
	
	for (FItemRecipeRow RecipeRow : RecipesData)
	{
		auto ItemObj = NewObject<URecipeListEntryObject>(this, RecipeEntryObjectClass);
		ItemObj->RecipeRow = RecipeRow;
		
		ItemObj->Text = RecipeRow.DisplayName;

		FSlateBrush Brush;
		Brush.SetResourceObject(RecipeRow.RecipeIcon.Get());
		Brush.ImageSize = FVector2D(RecipeRow.RecipeIcon->GetSizeX(), RecipeRow.RecipeIcon->GetSizeY());
		ItemObj->BrushStyle.Brush = Brush;

		AvailableRecipesList->AddItem(ItemObj);
		ItemsArray.AddUnique(ItemObj);
	}
}

void UCraftRecipesList::SearchTextChanged(const FText& NewText)
{
	const TArray<TObjectPtr<URecipeListEntryObject>>& SourceArray =
		ItemFiltersPanel->IsSearchInFilteredSlots() ? FilteredItemsArray : ItemsArray;

	AvailableRecipesList->ClearListItems();
	
	if (NewText.IsEmpty())
	{
		/*if (ActiveFilters.Num() > 0)
		{
			for (auto InvSlot : SourceArray)
			{
				AvailableRecipesList->AddItem(InvSlot);
			}
		}
		else*/
		{
			for (auto ArrayElement : SourceArray)
			{
				AvailableRecipesList->AddItem(ArrayElement);
			}
		}
		return;
	}
	
	for (auto ArrayElement : SourceArray)
	{
		FString StringName = ArrayElement->Text.ToString();
		if (StringName.Contains(NewText.ToString(), ESearchCase::IgnoreCase))
		{
			AvailableRecipesList->AddItem(ArrayElement);
		}
	}
}