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
