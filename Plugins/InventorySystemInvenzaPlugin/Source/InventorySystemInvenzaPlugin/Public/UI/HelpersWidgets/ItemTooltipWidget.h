//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "Components/Image.h"
#include "ItemTooltipWidget.generated.h"

class UUInventoryWidgetBase;
class UItemBase;
struct FItemMetaData;
class UTextBlock;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UItemTooltipWidget : public UBaseUserWidget
{
	GENERATED_BODY()
	
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UItemTooltipWidget();
	
	virtual void SetTooltipData(UItemBase* Item, UUInventoryWidgetBase* Inventory = nullptr);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemName;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemType;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemDescription;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> MaxStackSize;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> StackSizeValue;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> StackWeightValue;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> MoneyIcon;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> PriceText;

	//====================================================================
	// FUNCTIONS
	//====================================================================
};
