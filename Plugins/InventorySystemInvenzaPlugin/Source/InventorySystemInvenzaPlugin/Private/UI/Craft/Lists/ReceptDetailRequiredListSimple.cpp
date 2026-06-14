// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/Lists/ReceptDetailRequiredListSimple.h"

#include "Components/ListView.h"
#include "Data/CraftSystem/Entries/RecipeRequiredIListEntryObject.h"

UReceptDetailRequiredListSimple::UReceptDetailRequiredListSimple()
{
}

void UReceptDetailRequiredListSimple::NativeConstruct()
{
	Super::NativeConstruct();
}

void UReceptDetailRequiredListSimple::RefreshRequiredList(const FItemRecipeRow& RecipeRow,
	const TArray<FRecipeItemRequirementCheck>& Requirements)
{
	if (!RequiredList) return;
	
	if (!RequiredListEntryObjectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - RequiredListEntryObjectClass is not set!"), *FString(__FUNCTION__));
		return;
	}

	if (Requirements.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - Requirements array is empty!"), *FString(__FUNCTION__));
		return;
	}

	RequiredList->ClearListItems();

	for (int32 Index = 0; Index < Requirements.Num(); ++Index)
	{
		auto* ItemObj = NewObject<URecipeRequiredIListEntryObject>(this, RequiredListEntryObjectClass);
		if (!ItemObj) continue;

		ItemObj->RecipeRow = RecipeRow;
		ItemObj->RecipeCheckResult = Requirements[Index];
		ItemObj->Index = Index;

		RequiredList->AddItem(ItemObj);
	}
	
	RequiredList->RequestRefresh();
}

void UReceptDetailRequiredListSimple::UpdateRequirementsCheck(const FItemRecipeRow& UpdateRecipeRow,
	const TArray<FRecipeItemRequirementCheck>& NewRequirements)
{
	if (!RequiredList) return;
	
	const TArray<UObject*>& CurrentItems = RequiredList->GetListItems();

	if (CurrentItems.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - List is empty, nothing to update!"), *FString(__FUNCTION__));
		return;
	}

	if (NewRequirements.Num() != CurrentItems.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("%s - Count mismatch! New checks: %d, Items in list: %d"), 
			*FString(__FUNCTION__), NewRequirements.Num(), CurrentItems.Num());
		return;
	}
	
	for (int32 Index = 0; Index < CurrentItems.Num(); ++Index)
	{
		if (auto* RecipeItem = Cast<URecipeRequiredIListEntryObject>(CurrentItems[Index]))
		{
			if (RecipeItem->RecipeRow.ID == UpdateRecipeRow.ID)
				RecipeItem->RecipeCheckResult = NewRequirements[Index];
		}
	}
	
	RequiredList->RequestRefresh();
}

TArray<int32> UReceptDetailRequiredListSimple::GetAllSelectedOptions()
{
	TArray<int32> ResultIndices;
	if (!RequiredList) return ResultIndices;
	
	const TArray<UObject*>& AllItems = RequiredList->GetListItems();

	for (UObject* ItemObj : AllItems)
	{
		if (auto* RecipeItem = Cast<URecipeRequiredIListEntryObject>(ItemObj))
		{
			ResultIndices.Add(RecipeItem->SelectedOptionIndex);
		}
	}

	return ResultIndices;
}
