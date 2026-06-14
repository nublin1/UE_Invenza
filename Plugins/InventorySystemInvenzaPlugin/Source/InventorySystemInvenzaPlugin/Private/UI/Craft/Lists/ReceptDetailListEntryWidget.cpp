// Nublin Studio 2026 All Rights Reserved.

#include "UI/Craft/Lists/ReceptDetailListEntryWidget.h"

#include "Components/VerticalBox.h"
#include "Data/CraftSystem/Entries/RecipeRequiredIListEntryObject.h"
#include "Factory/InvenzaWidgetFactory.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Craft/Ingredient/RequirementOptionEntry.h"

UReceptDetailListEntryWidget::UReceptDetailListEntryWidget()
{
}

void UReceptDetailListEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UReceptDetailListEntryWidget::NativeOnListItemObjectSet(UObject* DetailItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(DetailItemObject);

	auto* RecipeItem = Cast<URecipeRequiredIListEntryObject>(DetailItemObject);
	if (!RecipeItem || !RequirementOptionClass || !RequirementsContainer)
		return;

	CashedListEntryObject = RecipeItem;
	SelectedOption = CashedListEntryObject->SelectedOptionIndex;

	if (RequirementsContainer->GetChildrenCount() == 0)
	{
		const TArray<URequirementOptionEntry*> OptionEntries =
			CreateRequirementOptionEntries(RecipeItem->RecipeCheckResult);
		if (OptionEntries.IsEmpty()) return;

		for (URequirementOptionEntry* OptionEntry : OptionEntries)
		{
			if (!OptionEntry || !OptionEntry->MainButton) continue;
			RequirementsContainer->AddChild(OptionEntry);
			OptionEntry->MainButton->OnButtonClicked.AddDynamic(
				this,
				&UReceptDetailListEntryWidget::HandleOptionButtonClicked
			);
		}
	}
	else
	{
		TArray<UWidget*> ChildWidgets = RequirementsContainer->GetAllChildren();

		if (ChildWidgets.IsValidIndex(0))
			if (auto* PrimaryEntry = Cast<URequirementOptionEntry>(ChildWidgets[0]))
				PrimaryEntry->UpdateData(RecipeItem->RecipeCheckResult.Primary);

		const TArray<FRecipeRequirementResult>& Alternatives = RecipeItem->RecipeCheckResult.Alternatives;
		
		for (int32 ExcessIndex = ChildWidgets.Num() - 1; ExcessIndex >= Alternatives.Num() + 1; --ExcessIndex)
		{
			RequirementsContainer->RemoveChildAt(ExcessIndex);
		}
		
		for (int32 AltIndex = 0; AltIndex < Alternatives.Num(); ++AltIndex)
		{
			int32 ChildIndex = AltIndex + 1;

			if (ChildWidgets.IsValidIndex(ChildIndex))
			{
				if (auto* AltEntry = Cast<URequirementOptionEntry>(ChildWidgets[ChildIndex]))
					AltEntry->UpdateData(Alternatives[AltIndex]);
			}
			else
			{
				if (URequirementOptionEntry* NewEntry = CreateRequirementOptionEntry(Alternatives[AltIndex], RequirementOptionClassAlt))
				{
					if (!NewEntry->MainButton) continue;

					RequirementsContainer->AddChild(NewEntry);
					NewEntry->MainButton->OnButtonClicked.AddDynamic(
						this,
						&UReceptDetailListEntryWidget::HandleOptionButtonClicked
					);
				}
			}
		}
	}
	
	RebuildMaps();

	for (auto& Element : ButtonToEntryMap)
	{
		int32 TargetIndex = EntryToIndexMap.FindRef(Element.Value);
		Element.Value->MainButton->SetToggleStatus(TargetIndex == SelectedOption);
	}
}

void UReceptDetailListEntryWidget::HandleOptionButtonClicked(UUIButton* ClickedButton)
{
	if (!ClickedButton)
		return;

	auto FindResult = ButtonToEntryMap.FindRef(ClickedButton);
	if (!FindResult)
		return;

	for (auto Element : ButtonToEntryMap)
	{
		Element.Value->MainButton->SetToggleStatus(Element.Value->MainButton == ClickedButton);
	}

	SelectedOption = EntryToIndexMap.FindRef(FindResult);
	CashedListEntryObject->SelectedOptionIndex = SelectedOption;
}

URequirementOptionEntry* UReceptDetailListEntryWidget::CreateRequirementOptionEntry(
	const FRecipeRequirementResult& RequirementResult, TSubclassOf<URequirementOptionEntry> OptionClass)
{
	auto Wid = UInvenzaWidgetFactory::CreateInvenzaWidget(GetOwningPlayer(), OptionClass);
	if (!Wid)
		return nullptr;

	URequirementOptionEntry* Entry = Cast<URequirementOptionEntry>(Wid);
	if (!Entry)
		return nullptr;

	Entry->UpdateData(RequirementResult);
	return Entry;
}

TArray<URequirementOptionEntry*> UReceptDetailListEntryWidget::CreateRequirementOptionEntries(
	const FRecipeItemRequirementCheck& ItemRequirementCheck)
{
	TArray<URequirementOptionEntry*> Result;

	if (URequirementOptionEntry* PrimaryEntry = CreateRequirementOptionEntry(ItemRequirementCheck.Primary, RequirementOptionClass))
		Result.Add(PrimaryEntry);

	for (const FRecipeRequirementResult& Alternative : ItemRequirementCheck.Alternatives)
	{
		if (URequirementOptionEntry* AltEntry = CreateRequirementOptionEntry(Alternative, RequirementOptionClassAlt))
			Result.Add(AltEntry);
	}

	return Result;
}

void UReceptDetailListEntryWidget::RebuildMaps()
{
	ButtonToEntryMap.Empty();
	EntryToIndexMap.Empty();

	TArray<UWidget*> ChildWidgets = RequirementsContainer->GetAllChildren();
	for (int32 Index = 0; Index < ChildWidgets.Num(); ++Index)
	{
		auto* Entry = Cast<URequirementOptionEntry>(ChildWidgets[Index]);
		if (!Entry || !Entry->MainButton) continue;

		ButtonToEntryMap.Add(Entry->MainButton, Entry);
		EntryToIndexMap.Add(Entry, Index);
	}
}
