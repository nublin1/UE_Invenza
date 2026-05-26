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
		UpdateIngredientImage(RecipeDetail->RecipeCheckResult.ItemMetaData.ItemAssetData.Icon);
		
		RequiredItemName->UpdateText(RecipeDetail->RecipeCheckResult.ItemMetaData.ItemTextData.DisplayName);
		if (RemainingCounter->CurrentValue)
			RemainingCounter->CurrentValue->UpdateText(FText::AsNumber(RecipeDetail->RecipeCheckResult.AmountNeed));
		RemainingCounter->MaxValue->UpdateText(FText::AsNumber(RecipeDetail->RecipeCheckResult.AmountHave));
	}
}

void UReceptDetailListEntryWidget::UpdateIngredientImage(const TSoftObjectPtr<UTexture2D>& NewIngredientIcon)
{
	if (!IngredientIcon || NewIngredientIcon.IsNull())
		return;

	UTexture2D* LoadedTexture = NewIngredientIcon.LoadSynchronous();
	if (!LoadedTexture)
		return;

	FSlateBrush NewBrush;
	NewBrush.SetResourceObject(LoadedTexture);
	//NewBrush.ImageSize = FVector2D(LoadedTexture->GetSizeX(), LoadedTexture->GetSizeY());
	IngredientIcon->UpdateBrush(NewBrush);
}

