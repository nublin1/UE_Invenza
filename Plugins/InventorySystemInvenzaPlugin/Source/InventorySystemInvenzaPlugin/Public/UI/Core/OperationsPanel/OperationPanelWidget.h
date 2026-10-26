//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/UIStructs.h"
#include "OperationPanelWidget.generated.h"

class UUIButton;
class UButton;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UOperationPanelWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UOperationPanelWidget();

protected:
	// Unreal overrides
	virtual void NativePreConstruct() override;

public:
	// Properties and variables
	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> Button_TakeAll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> Button_PlaceAll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> Button_Sort;

	UPROPERTY(BlueprintReadOnly, Category = UI, meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> Button_PlaceMatching;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Restock")
	FLabelStyle RestockLabelStyle;
};
