//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "Components/Image.h"
#include "ItemTooltipWidget.generated.h"

class ULabelBaseText;
class UInventoryBase;
class UUInventoryBaseWidget;
class UObject;
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
	UItemTooltipWidget();
	
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
	UFUNCTION(BlueprintCallable, Category="Tooltip|Core")
	virtual void SetTooltipData(UObject* InItem, UInventoryBase* InInventory = nullptr);

	UFUNCTION(BlueprintCallable, Category="Tooltip|Price")
	virtual void UpdatePrice();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Tooltip|Data")
	TObjectPtr<UObject> Item;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Tooltip|Data")
	TObjectPtr<UInventoryBase> Inventory;	

	//====================================================================
	// FUNCTIONS
	//====================================================================
};
