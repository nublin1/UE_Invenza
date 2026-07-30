// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/InvenzaBaseWidget.h"
#include "UIButton.generated.h"

class UBorder;
class UInputAction;
class USizeBox;
class UButton;
class UImage;
class UTextBlock;

/**
 * UI Button Widget - A customizable button with optional toggle functionality.
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UUIButton : public UInvenzaBaseWidget
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnButtonClicked, UUIButton*, UIButton);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToggled, bool, Status);
#pragma endregion Delegates

public:
	UUIButton();
	
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Delegates
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "UI|Events")
	FOnButtonClicked OnButtonClicked;
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "UI|Events")
	FOnToggled OnToggled;
	
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MainLabel;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UImage> MainImage;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UButton> MainButton;


	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION(BlueprintCallable, Category = "UI|Config")
	FGameplayTag GetBtnTag() const {return BtnTag;}
	UFUNCTION(BlueprintCallable, Category = "UI|Config")
	void SetBtnTag(FGameplayTag NewTag) {BtnTag = NewTag;}
	
	UFUNCTION(BlueprintCallable, Category = "UI|State")
	bool GetToggleStatus() const {return bIsToggleOn;}
	UFUNCTION(BlueprintCallable, Category = "UI|State")
	void SetToggleStatus(const bool bNewStatus);
	
	UFUNCTION(BlueprintCallable, Category = "UI|Config")
	void UpdateUseAction(UInputAction* NewAction);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Input")
	TObjectPtr<UInputAction> ClickAction;
	uint32 ClickActionHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|State")
	bool bIsToggleButton;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|State")
	bool bIsToggleOn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|State")
	FLinearColor ToggleColor = FLinearColor(FColor::FromHex(TEXT("FFD369FF")));
	
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FGameplayTag BtnTag;

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Defaults")
	FText DefaultText = FText::FromString(" ");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Defaults")
	FVector2D DefaultSize = FVector2D(64.f, 64.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Defaults")
	TObjectPtr<UTexture2D> DefaultImage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Defaults")
	FLinearColor DefaultButtonBackgroundColor;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION()
	virtual void SetupInput();
	UFUNCTION()
	virtual void ClickButton();
	UFUNCTION()
	virtual void OnMainButtonClicked();
};
