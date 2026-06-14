// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/Lists/CraftRecipesList.h"

#include "Components/EditableText.h"
#include "Data/CraftSystem/Entries/RecipeListEntryObject.h"
#include "UI/Core/ItemFiltersPanel/FiltersPanel.h"


UCraftRecipesList::UCraftRecipesList()
{
	RecipeEntryObjectClass = URecipeListEntryObject::StaticClass();
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
		UE_LOG(LogTemp, Error, TEXT("CraftRecipesList: RecipeEntryObjectClass is NULL!"));
		return;
	}

	// Update the list incrementally instead of recreating all entries.
	// Remove items that no longer exist, update existing ones,
	// and create only the newly added recipe entries.

	TSet<FName> NewIDs;
	for (const FItemRecipeRow& RecipeRow : RecipesData)
	{
		NewIDs.Add(RecipeRow.ID);
	}

	// Remove obsolete entries
	for (int32 i = ItemsArray.Num() - 1; i >= 0; --i)
	{
		URecipeListEntryObject* ItemObj = ItemsArray[i];
		if (!ItemObj)
		{
			ItemsArray.RemoveAt(i);
			continue;
		}

		if (!NewIDs.Contains(ItemObj->RecipeRow.ID))
		{
			AvailableRecipesList->RemoveItem(ItemObj);
			ItemsArray.RemoveAt(i);
		}
	}

	// Create a lookup for existing entries
	TMap<FName, URecipeListEntryObject*> ExistingItems;
	for (URecipeListEntryObject* ItemObj : ItemsArray)
	{
		if (ItemObj)
		{
			ExistingItems.Add(ItemObj->RecipeRow.ID, ItemObj);
		}
	}

	// Add new entries and update existing ones
	for (const FItemRecipeRow& RecipeRow : RecipesData)
	{
		if (URecipeListEntryObject** ExistingItem = ExistingItems.Find(RecipeRow.ID))
		{
			// Update existing entry
			(*ExistingItem)->RecipeRow = RecipeRow;
			(*ExistingItem)->Text = RecipeRow.DisplayName;
		}
		else
		{
			// Add new entry
			URecipeListEntryObject* ItemObj =
				NewObject<URecipeListEntryObject>(this, RecipeEntryObjectClass);

			ItemObj->RecipeRow = RecipeRow;
			ItemObj->Text = RecipeRow.DisplayName;

			AvailableRecipesList->AddItem(ItemObj);
			ItemsArray.Add(ItemObj);
		}
	}

	AvailableRecipesList->RequestRefresh();
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