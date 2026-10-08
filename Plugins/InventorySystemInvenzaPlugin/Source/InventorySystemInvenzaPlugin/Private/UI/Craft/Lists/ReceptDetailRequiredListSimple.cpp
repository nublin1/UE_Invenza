// Nublin Studio 2026 All Rights Reserved.
#include "UI/Craft/Lists/ReceptDetailRequiredListSimple.h"
#include "Components/ListView.h"
#include "Data/CraftSystem/Entries/RecipeRequiredIListEntryObject.h"

UReceptDetailRequiredListSimple::UReceptDetailRequiredListSimple()
{
	RequiredListEntryObjectClass = URecipeRequiredIListEntryObject::StaticClass();
}
void UReceptDetailRequiredListSimple::NativeConstruct() { Super::NativeConstruct(); }

void UReceptDetailRequiredListSimple::ClearRequirements()
{
	if (!RequiredList) return;
	for (UObject* Object : RequiredList->GetListItems())
		if (auto* Item = Cast<URecipeRequiredIListEntryObject>(Object)) Item->OnSelectionChanged.RemoveAll(this);
	RequiredList->ClearListItems();
}

void UReceptDetailRequiredListSimple::RefreshRequiredList(const FItemRecipeRow& RecipeRow,
	const TArray<FRecipeItemRequirementCheck>& Requirements)
{
	if (!RequiredList) return;
	if (Requirements.IsEmpty()) { ClearRequirements(); return; }
	if (!RequiredListEntryObjectClass) { ClearRequirements(); return; }
	const TArray<UObject*>& Existing = RequiredList->GetListItems();
	bool bReuse = Existing.Num() == Requirements.Num();
	for (UObject* Object : Existing)
	{
		const auto* Item = Cast<URecipeRequiredIListEntryObject>(Object);
		bReuse &= Item && Item->RecipeRow.ID == RecipeRow.ID;
	}
	if (!bReuse)
	{
		ClearRequirements();
		for (int32 Index = 0; Index < Requirements.Num(); ++Index)
		{
			auto* Item = NewObject<URecipeRequiredIListEntryObject>(this, RequiredListEntryObjectClass);
			Item->RecipeRow = RecipeRow;
			Item->Index = Index;
			Item->OnSelectionChanged.AddUObject(this, &ThisClass::HandleOptionChanged);
			RequiredList->AddItem(Item);
		}
	}
	for (int32 Index = 0; Index < Requirements.Num(); ++Index)
	{
		auto* Item = CastChecked<URecipeRequiredIListEntryObject>(RequiredList->GetItemAt(Index));
		Item->RecipeRow = RecipeRow;
		Item->RecipeCheckResult = Requirements[Index];
		Item->SelectedOptionIndex = FMath::Clamp(Item->SelectedOptionIndex, 0, Requirements[Index].Alternatives.Num());
		Item->OnDataChanged.Broadcast();
	}
	RequiredList->RequestRefresh();
}
void UReceptDetailRequiredListSimple::UpdateRequirementsCheck(const FItemRecipeRow& RecipeRow,
	const TArray<FRecipeItemRequirementCheck>& Requirements)
{
	RefreshRequiredList(RecipeRow, Requirements);
}
TArray<int32> UReceptDetailRequiredListSimple::GetAllSelectedOptions()
{
	TArray<int32> Result;
	if (RequiredList)
		for (UObject* Object : RequiredList->GetListItems())
			if (const auto* Item = Cast<URecipeRequiredIListEntryObject>(Object)) Result.Add(Item->SelectedOptionIndex);
	return Result;
}
void UReceptDetailRequiredListSimple::HandleOptionChanged() { OnOptionsChanged.Broadcast(); }
