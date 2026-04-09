//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "Components/Image.h"
#include "ItemTooltipWidget.generated.h"

class ULabelBaseText;
class UInventoryBase;
class UUInventoryWidgetBase;
class UItemBase;
struct FItemMetaData;
class UTextBlock;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UItemTooltipWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()
	
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<ULabelBaseText> ItemName;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<ULabelBaseText> ItemType;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<ULabelBaseText> ItemDescription;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<ULabelBaseText> MaxStackSize;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<ULabelBaseText> StackSizeValue;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<ULabelBaseText> StackWeightValue;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> MoneyIcon;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<ULabelBaseText> PriceText;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<ULabelBaseText> PriceValue;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UItemTooltipWidget();
	
	virtual void SetTooltipData(UItemBase* Item, UInventoryBase* Inventory = nullptr);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	

	//====================================================================
	// FUNCTIONS
	//====================================================================
};
