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

protected:
	// Unreal overrides
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackgroundClicked);
#pragma endregion Delegates

	// Properties and variables
	UPROPERTY(BlueprintAssignable, Category = "ModalLayout")
	FOnBackgroundClicked OnBackgroundClicked;

	// Functions
	UFUNCTION(Category = "Invenza|UI|Layout", BlueprintCallable, BlueprintImplementableEvent)
	void ShowTop();

	UFUNCTION(Category = "Invenza|UI|Layout", BlueprintCallable, BlueprintImplementableEvent)
	void CollapseTop();

	UFUNCTION(Category = "Invenza|UI|Layout", BlueprintCallable, BlueprintPure, BlueprintImplementableEvent)
	UUserWidget* Peek();

	UFUNCTION(Category = "Invenza|UI|Layout", BlueprintCallable, BlueprintImplementableEvent)
	UInvenzaBaseWidget* PushContent(const TSoftClassPtr<UUserWidget>& WidgetClass);

	UFUNCTION()
	TArray<UInvenzaBaseWidget*> GetStack() { return Stack; }

	UFUNCTION(Category = "Invenza|UI|Layout", BlueprintCallable, BlueprintImplementableEvent)
	void ClearStack();

	UFUNCTION(Category = "Invenza|UI|Layout", BlueprintCallable, BlueprintImplementableEvent)
	void PopContent();

protected:
	// Runtime
	// Properties and variables
	//
	UPROPERTY(Category = "Invenza|UI|Layout", BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> MainBorder;

	//
	UPROPERTY(Category = "Invenza|UI|Layout", VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInvenzaBaseWidget> PushedWidget;

	UPROPERTY(Category = "Invenza|UI|Layout", VisibleInstanceOnly, BlueprintReadWrite)
	TArray<TObjectPtr<UInvenzaBaseWidget>> Stack;

	UPROPERTY(Category = "Invenza|UI|Layout", EditAnywhere, BlueprintReadWrite)
	bool bCollapseWhenStackIsEmpty = true;

	UPROPERTY(Category = "Invenza|UI|Layout", EditInstanceOnly, BlueprintReadWrite)
	TEnumAsByte<EHorizontalAlignment> DefaultHorizontalAlignment = HAlign_Center;

	UPROPERTY(Category = "Invenza|UI|Layout", EditInstanceOnly, BlueprintReadWrite)
	TEnumAsByte<EVerticalAlignment> DefaultVerticalAlignment = VAlign_Center;
};
