// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "ItemFiltersPanel.generated.h"

class UEditableText;
class UUIButton;
class UItemCategoryButton;
class UVerticalBox;
class UHorizontalBox;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UItemFiltersPanel : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Enable filter color override"))
	bool bUseFilterColor = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Only for Grid inventory", EditCondition = "bUseFilterColor"))
	FLinearColor ItemFilterBorderColor = FLinearColor::Green;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="Only for Grid inventory"))
	float FilterOpacity = 0.15f;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UUIButton* GetClearFiltersButton() const {return ClearFiltersButton; }
	TArray<TObjectPtr<UItemCategoryButton>> GetFilteredCategores() const {return CategoryButtonList;}
	UEditableText* GetSearchText() const {return SearchText; }
	bool IsSearchInFilteredSlots() const {return bSearchInFilteredSlots; }

	UFUNCTION()
	virtual void DisableAllFilters();
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HorizontalContentBox;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> VerticalContentBox;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UEditableText> SearchText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> ClearFiltersButton;

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsShowSearchField = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UItemCategoryButton>> CategoryButtonList;
	
	/** Whether to search in filtered inventory slots instead of the full inventory slots array */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="If true, search will be performed in the filtered inventory slots"))
	bool bSearchInFilteredSlots;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeConstruct() override;

	UFUNCTION()
	virtual void OnClearFiltersButtonPressed();
};
