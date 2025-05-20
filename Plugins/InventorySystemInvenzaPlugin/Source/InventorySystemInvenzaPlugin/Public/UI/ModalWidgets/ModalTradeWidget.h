// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/ModalWidgets/ModalBaseWidget.h"
#include "ModalTradeWidget.generated.h"

class UBinaryPromptButtons;
class UUIButton;
class USlider;

USTRUCT(BlueprintType)
struct FModalTradeData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FText ActionText;
	UPROPERTY(BlueprintReadOnly)
	int32 MaxAmount;
	UPROPERTY(BlueprintReadOnly)
	FText ItemName;
	UPROPERTY(BlueprintReadOnly)
	float UnitPrice;
	
	FModalTradeData(): MaxAmount(0), UnitPrice(0)
	{
	}

	FModalTradeData(const FText& NewActionText, int32 NewMaxAmount, const FText& NewItemName, float NewUnitPrice):
		MaxAmount(NewMaxAmount), UnitPrice(NewUnitPrice)
	{
		ActionText = NewActionText;
		ItemName = NewItemName;
	}
};

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UModalTradeWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	TFunction<void(int32)> ConfirmCallback;
	TFunction<void()> CancelCallback;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UModalTradeWidget();

	UFUNCTION(BlueprintImplementableEvent)
	void InitializeTradeWidget(FModalTradeData ModalTradeData);

	//UModalBaseWidget* GetModalBaseWidget() {return ModalWindowBase;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<USlider> QuantitySlider;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UModalBaseWidget> ModalWindowBase;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UBinaryPromptButtons> PromtButtomsWidget;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeConstruct() override;

	UFUNCTION()
	virtual void Confirm(UUIButton* Button);
	UFUNCTION()
	virtual void Cancel(UUIButton* Button);
};
