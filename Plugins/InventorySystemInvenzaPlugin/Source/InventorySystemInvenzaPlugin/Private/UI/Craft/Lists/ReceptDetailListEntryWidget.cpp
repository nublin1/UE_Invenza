// Nublin Studio 2026 All Rights Reserved.

#include "UI/Craft/Lists/ReceptDetailListEntryWidget.h"

#include "Components/Button.h"
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

	if (auto RecipeDetail = Cast<URecipeRequiredIListEntryObject>(DetailItemObject))
	{
		if (RequirementOptionClass && RequirementsContainer)
		{
			TArray<URequirementOptionEntry*> Entries = CreateRequirementOptionEntries(RecipeDetail->RecipeCheckResult);
			if (Entries.IsEmpty())
				return;

			RequirementsContainer->ClearChildren();

			for (URequirementOptionEntry* Entry : Entries)
			{
				RequirementsContainer->AddChild(Entry);
				Entry->OnButtonClicked.AddDynamic()
			}
		}
	}
}

URequirementOptionEntry* UReceptDetailListEntryWidget:: CreateRequirementOptionEntry(
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
