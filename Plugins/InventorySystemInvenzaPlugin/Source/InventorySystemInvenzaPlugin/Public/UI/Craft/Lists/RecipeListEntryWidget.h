// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "UI/InvenzaBaseWidget.h"
#include "RecipeListEntryWidget.generated.h"

class ULabelBaseText;
class UImageBaseWidget;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API URecipeListEntryWidget : public UInvenzaBaseWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
	URecipeListEntryWidget();

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UImageBaseWidget> Recipe_Image;
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> Recipe_Text;

	//====================================================================
	// FUNCTIONS
	//====================================================================

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void UpdateRecipeImage(const TSoftObjectPtr<UTexture2D>& RecipeIcon);
	UFUNCTION()
	void UpdateRecipeText(const FText& Text);
};
