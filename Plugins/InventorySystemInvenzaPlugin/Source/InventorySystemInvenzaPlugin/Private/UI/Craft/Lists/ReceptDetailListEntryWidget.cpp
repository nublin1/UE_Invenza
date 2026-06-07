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
	{
		return;
	}

	CashedListEntryObject = RecipeItem;

	const TArray<URequirementOptionEntry*> OptionEntries =
		CreateRequirementOptionEntries(RecipeItem->RecipeCheckResult);

	if (OptionEntries.IsEmpty())
	{
		return;
	}

	RequirementsContainer->ClearChildren();
	ButtonToEntryMap.Empty();
	EntryToIndexMap.Empty();

	int32 Index = 0;

	for (URequirementOptionEntry* OptionEntry : OptionEntries)
	{
		if (!OptionEntry || !OptionEntry->MainButton)
		{
			continue;
		}

		RequirementsContainer->AddChild(OptionEntry);

		OptionEntry->MainButton->OnButtonClicked.AddDynamic(
			this,
			&UReceptDetailListEntryWidget::HandleOptionButtonClicked
		);

		ButtonToEntryMap.Add(OptionEntry->MainButton, OptionEntry);
		EntryToIndexMap.Add(OptionEntry, Index);

		++Index;
	}
	
	if (OptionEntries.Num() > 0 && OptionEntries[0] && OptionEntries[0]->MainButton)
	{
		OptionEntries[0]->MainButton->SetToggleStatus(true);
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
		if (Element.Value->MainButton == ClickedButton)
		{
			Element.Value->MainButton->SetToggleStatus(true);
		}
		else
			Element.Value->MainButton->SetToggleStatus(false);
	}

	SelectedOption = EntryToIndexMap.FindRef(FindResult);
	
	CashedListEntryObject->Index = SelectedOption;
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
