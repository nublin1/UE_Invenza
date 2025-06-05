// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "UIButton.generated.h"

class UInputAction;
class USizeBox;
class UButton;
class UImage;
class UTextBlock;

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnButtonClicked, UUIButton*, UIButton);
#pragma endregion Delegates

/**
 * UI Button Widget - A customizable button with optional toggle functionality.
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UUIButton : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Delegates
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "UI|Events")
	FOnButtonClicked OnButtonClicked;
	
	// Widgets
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> MainBox;
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MainLabel;
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UImage> MainImage;
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UButton> MainButton;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UUIButton();

	UFUNCTION(BlueprintCallable, Category = "UI|State")
	bool GetToggleStatus() const {return bIsToggleOn;}

	UFUNCTION(BlueprintCallable, Category = "UI|State")
	void SetToggleStatus(const bool bNewStatus);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Input")
	TObjectPtr<UInputAction> ClickAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|State")
	bool bIsToggleButton;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|State")
	bool bIsToggleOn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|State")
	FLinearColor ToggleColor = FLinearColor(FColor::FromHex(TEXT("FFD369FF")));

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Defaults")
	FText DefaultText = FText::FromString(" ");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Defaults")
	FVector2D DefaultSize = FVector2D(64.f, 64.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Defaults")
	TObjectPtr<UTexture2D> DefaultImage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Defaults")
	FLinearColor DefaultButtonBackgroundImage;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	virtual void ClickButton();
	UFUNCTION()
	virtual void OnMainButtonClicked();
};
