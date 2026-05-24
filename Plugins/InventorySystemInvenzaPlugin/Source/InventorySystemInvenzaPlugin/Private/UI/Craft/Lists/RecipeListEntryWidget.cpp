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

	if (auto RecipeObj = Cast<URecipeListEntryObject>(ListItemObject))
	{
		Recipe_Text->UpdateText(RecipeObj->Text);
		Recipe_Image->UpdateBrush(RecipeObj->BrushStyle.Brush);
	}
}
