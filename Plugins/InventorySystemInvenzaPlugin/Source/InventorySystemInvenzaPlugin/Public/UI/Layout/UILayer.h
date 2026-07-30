//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "UILayer.generated.h"

class UBorder;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UUILayer : public UInvenzaBaseWidget
{
	GENERATED_BODY()

	
public:
	UUILayer();
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent )
	UInvenzaBaseWidget* PushContent(const TSoftClassPtr<UUserWidget>& WidgetClass);
	
	UFUNCTION()
	TArray<UInvenzaBaseWidget*> GetStack() { return Stack; }
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent )
	void ClearStack();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent )
	void PopContent();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UBorder> MainBorder;

	//
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInvenzaBaseWidget> PushedWidget;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TArray<TObjectPtr<UInvenzaBaseWidget>> Stack;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TEnumAsByte<EHorizontalAlignment> DefaultHorizontalAlignment = HAlign_Center;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TEnumAsByte<EVerticalAlignment> DefaultVerticalAlignment = VAlign_Center;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
