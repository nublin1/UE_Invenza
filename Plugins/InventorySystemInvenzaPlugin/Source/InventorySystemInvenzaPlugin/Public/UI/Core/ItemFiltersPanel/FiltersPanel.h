// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/InvenzaBaseWidget.h"
#include "FiltersPanel.generated.h"

class UEditableText;
class UUIButton;
class UVerticalBox;
class UHorizontalBox;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UFiltersPanel : public UInvenzaBaseWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	

	//====================================================================
	// FUNCTIONS
	//====================================================================
	bool IsSearchInFilteredSlots() const {return bSearchInFilteredSlots; }
	
	UUIButton* GetClearFiltersButton() const {return ClearFiltersButton; }
	UUIButton* GetClearSearchTextButton() const {return ClearSearchTextButton; }
	TArray<TObjectPtr<UUIButton>> GetFilteredCategores() const {return CategoryButtonList;}
	UEditableText* GetSearchText() const {return SearchText; }

	UFUNCTION(BlueprintCallable, Category = "Filters")
	FGameplayTagContainer GetActiveFilterTags() const;

	UFUNCTION()
	virtual void DisableAllFilters();
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(BlueprintReadWrite, Category = "Filters|UI Elements", meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> FilterButtons;
	UPROPERTY(BlueprintReadWrite, Category = "Filters|UI Elements", meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> VerticalContentBox;
	UPROPERTY(BlueprintReadWrite, Category = "Filters|UI Elements", meta = (BindWidgetOptional))
	TObjectPtr<UEditableText> SearchText;

	UPROPERTY(BlueprintReadWrite, Category = "Filters|UI Elements", meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> ClearFiltersButton;

	UPROPERTY(BlueprintReadWrite, Category = "Filters|UI Elements", meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> ClearSearchTextButton;

	//
	UPROPERTY(Category = "Filters|Settings",EditAnywhere, BlueprintReadWrite)
	bool bIsShowFiltersButtons = true;
	UPROPERTY(Category = "Filters|Settings",EditAnywhere, BlueprintReadWrite)
	bool bIsShowSearchField = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Filters")
	TArray<TObjectPtr<UUIButton>> CategoryButtonList;
	
	/** Whether to search in filtered inventory slots instead of the full inventory slots array */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filters", meta=(ToolTip="If true, search will be performed in the filtered inventory slots"))
	bool bSearchInFilteredSlots;

	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION()
	virtual void OnClearFiltersButtonPressed();

	UFUNCTION()
	virtual void OnClearSearchTextButtonPressed();
};
