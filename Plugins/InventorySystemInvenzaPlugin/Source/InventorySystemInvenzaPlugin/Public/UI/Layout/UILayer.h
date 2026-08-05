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

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackgroundClicked);
#pragma endregion Delegates
	
public:
	UUILayer();
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable, Category = "ModalLayout")
	FOnBackgroundClicked OnBackgroundClicked;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent )
	void ShowTop();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent )
	void CollapseTop();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintImplementableEvent )
	UUserWidget* Peek();
	
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCollapseWhenStackIsEmpty = true;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	TEnumAsByte<EHorizontalAlignment> DefaultHorizontalAlignment = HAlign_Center;
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	TEnumAsByte<EVerticalAlignment> DefaultVerticalAlignment = VAlign_Center;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
};
