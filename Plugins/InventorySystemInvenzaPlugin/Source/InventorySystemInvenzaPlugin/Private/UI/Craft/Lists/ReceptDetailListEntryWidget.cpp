// Nublin Studio 2026 All Rights Reserved.

#include "UI/Craft/Lists/ReceptDetailListEntryWidget.h"

#include "Data/CraftSystem/Entries/RecipeRequiredIListEntryObject.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Core/Image/ImageBaseWidget.h"
#include "UI/Core/Progress/CurrentMaxDisplay.h"

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
		const FRecipeItemRequirement& Requirement =
			RecipeDetail->RecipeRow.RequiredItems[RecipeDetail->Index];
		
		if (Requirement.Item.DataTable && !Requirement.Item.RowName.IsNone())
		{
			const FItemRecipeRow* ItemRow =
				Requirement.Item.GetRow<FItemRecipeRow>(TEXT("Recipe Requirement"));

			if (ItemRow)
			{
				IngredientIcon->UpdateImage(ItemRow->RecipeIcon.Get());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Requirement item is invalid!"));
		}
		
		RequiredItemName->UpdateText(RecipeDetail->RecipeCheckResult.DisplayName);
		RemainingCounter->CurrentValue->UpdateText(FText::AsNumber(RecipeDetail->RecipeCheckResult.AmountNeed));
		RemainingCounter->MaxValue->UpdateText(FText::AsNumber(RecipeDetail->RecipeCheckResult.AmountHave));
	}
}

