// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Core/List/SimpleUserObjectListEntry.h"
#include "RecipeListEntryWidget.generated.h"

class ULabelBaseText;
class UImageBaseWidget;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API URecipeListEntryWidget : public USimpleUserObjectListEntry
{
	GENERATED_BODY()

public:
	URecipeListEntryWidget();

protected:
	// Unreal overrides
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;

	virtual void NativeOnEntryReleased() override;

	virtual void NativeOnMouseEnter(const FGeometry& Geometry, const FPointerEvent& Event) override;

	virtual void NativeOnMouseLeave(const FPointerEvent& Event) override;

public:
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UImageBaseWidget> SelectionIndicator;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UImageBaseWidget> SelectionHighlight;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UImageBaseWidget> HoverHighlight;

	// Functions
	void RefreshRecipe(UObject* ListItemObject);
};
