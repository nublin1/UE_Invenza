// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/Ingredient/RequirementOptionEntry.h"

#include "ActorComponents/Crafting/CraftingTypes.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Core/Image/ImageBaseWidget.h"
#include "UI/Core/Progress/CurrentMaxDisplay.h"

URequirementOptionEntry::URequirementOptionEntry()
{
}

void URequirementOptionEntry::NativeConstruct()
{
	Super::NativeConstruct();

	if (MainButton)
	{
		MainButton->OnToggled.AddDynamic(this, &URequirementOptionEntry::URequirementOptionEntry::SetToggleStatus);
	}
}

void URequirementOptionEntry::UpdateData(FRecipeRequirementResult NewData)
{
	UpdateIngredientImage(NewData.ItemMetaData.ItemAssetData.Icon);
		
	RequiredItemName->UpdateText(NewData.ItemMetaData.ItemTextData.DisplayName);
	if (RemainingCounter->CurrentValue)
		RemainingCounter->CurrentValue->UpdateText(FText::AsNumber(NewData.AmountNeed));
	RemainingCounter->MaxValue->UpdateText(FText::AsNumber(NewData.AmountHave));
}

void URequirementOptionEntry::UpdateIngredientImage(const TSoftObjectPtr<UTexture2D>& NewIngredientIcon)
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

void URequirementOptionEntry::SetToggleStatus(bool bNewStatus)
{
	
}
