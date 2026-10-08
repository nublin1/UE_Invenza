// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/Ingredient/RequirementOptionEntry.h"

#include "Data/CraftSystem/CraftingStructs.h"
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
		MainButton->OnToggled.AddUniqueDynamic(this, &URequirementOptionEntry::URequirementOptionEntry::SetToggleStatus);
	}
}

void URequirementOptionEntry::UpdateData(const FRecipeRequirementResult& NewData)
{
	UpdateIngredientImage(NewData.ItemMetaData.ItemAssetData.Icon);
		
	if (RequiredItemName) RequiredItemName->UpdateText(NewData.ItemMetaData.ItemTextData.DisplayName);
	if (RemainingCounter && RemainingCounter->CurrentValue)
	{
		RemainingCounter->CurrentValue->UpdateText(FText::AsNumber(NewData.AmountNeed));
	}
    
	if (RemainingCounter && RemainingCounter->MaxValue)
	{
		RemainingCounter->MaxValue->UpdateText(FText::AsNumber(NewData.AmountHave));
	}
}

void URequirementOptionEntry::UpdateIngredientImage(const TSoftObjectPtr<UTexture2D>& NewIngredientIcon)
{
	if (!IngredientIcon) return;
	UTexture2D* Texture = NewIngredientIcon.LoadSynchronous();
	FSlateBrush Brush;
	Brush.SetResourceObject(Texture);
	IngredientIcon->UpdateBrush(Brush);
	IngredientIcon->SetRenderOpacity(Texture ? 1.f : 0.f);
}

void URequirementOptionEntry::SetToggleStatus(bool bNewStatus)
{
	
}
