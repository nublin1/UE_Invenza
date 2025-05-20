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
 * 
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
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnButtonClicked OnButtonClicked;
	
	// Widgets
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<USizeBox> MainBox;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> MainLabel;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> MainImage;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> MainButton;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UUIButton();

	bool GetToggleStatus() const {return bIsToggleOn;}

	void SetToggleStatus(const bool bNewStatus);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UInputAction> ClickAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsToggleButton;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsToggleOn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FLinearColor ToggleColor = FLinearColor(FColor::FromHex(TEXT("FFD369FF")));

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	FText DefaultText = FText::FromString(" ");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	FVector2D DefaultSize = FVector2D(64.f, 64.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	TObjectPtr<UTexture2D> DefaultImage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
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
