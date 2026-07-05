// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/Lists/RecipeListEntryWidget.h"

#include "Data/CraftSystem/Entries/RecipeListEntryObject.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Core/Image/ImageBaseWidget.h"

URecipeListEntryWidget::URecipeListEntryWidget()
{
}

void URecipeListEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	const auto RecipeObj = Cast<URecipeListEntryObject>(ListItemObject);
	if (!RecipeObj)
		return;

	UpdateImage(RecipeObj->RecipeRow.RecipeIcon);
	UpdateText(RecipeObj->Text);
}

/*
void URecipeListEntryWidget::UpdateRecipeImage(const TSoftObjectPtr<UTexture2D>& RecipeIcon)
{
	if (!Recipe_Image || RecipeIcon.IsNull())
		return;

	UTexture2D* LoadedTexture = RecipeIcon.LoadSynchronous();
	if (!LoadedTexture)
		return;

	FSlateBrush NewBrush;
	NewBrush.SetResourceObject(LoadedTexture);
	NewBrush.ImageSize = FVector2D(LoadedTexture->GetSizeX(), LoadedTexture->GetSizeY());
	Recipe_Image->UpdateBrush(NewBrush);
}

void URecipeListEntryWidget::UpdateRecipeText(const FText& Text)
{
	if (!Recipe_Text)
		return;

	Recipe_Text->UpdateText(Text);
}
*/
