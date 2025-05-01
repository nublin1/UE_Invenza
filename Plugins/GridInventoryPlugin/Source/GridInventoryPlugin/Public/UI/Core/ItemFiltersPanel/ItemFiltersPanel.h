// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "ItemFiltersPanel.generated.h"

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
	FLinearColor FilterColor = FLinearColor::Green;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="Only for Grid inventory"))
	float FilterOpacity = 0.15f;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UUIButton* GetClearFiltersButton() const {return ClearFiltersButton; }
	TArray<TObjectPtr<UItemCategoryButton>> GetFilteredCategores() const {return CategoryButtonList;}

	UFUNCTION()
	virtual void DisableAllFilters();
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HorizontalBox;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> VerticalBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> ClearFiltersButton;

	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UItemCategoryButton>> CategoryButtonList;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeConstruct() override;

	UFUNCTION()
	virtual void OnClearFiltersButtonPressed();
};
