// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "EditableLabelBaseText.generated.h"

class UEditableTextBox;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UEditableLabelBaseText : public UInvenzaBaseWidget
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEditableTextChanged, const FText&, NewText);
#pragma endregion
	
public:
	UEditableLabelBaseText();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// WIDGETS
	//====================================================================
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UEditableTextBox> MainEditableTextBox;

	//====================================================================
	// CONFIG
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FText Text;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FText HintText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FSlateColor Color;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FSlateFontInfo FontInfo;

	//====================================================================
	// EVENTS
	//====================================================================
	UPROPERTY(BlueprintAssignable, Category="UI|Events")
	FOnEditableTextChanged OnEditableTextChanged;

	//====================================================================
	// API
	//====================================================================
	UFUNCTION(BlueprintCallable)
	void UpdateText(FText NewText);
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHintText(FText NewHintText);

	UFUNCTION(BlueprintCallable)
	void UpdateFont(FSlateFontInfo NewFontInfo);

	UFUNCTION(BlueprintCallable)
	void UpdateTextAndFont(FLabelStyle LabelStyle);

	UFUNCTION(BlueprintCallable)
	void SyncTextAndFont();

	UFUNCTION(BlueprintCallable)
	void ClearText();

	UFUNCTION(BlueprintCallable)
	FText GetText() const;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Config")
	bool bNumericOnly = false;

	UPROPERTY()
	FText CachedText;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void HandleTextChanged(const FText& NewText);

	UFUNCTION()
	void ApplyNumericFilter(FString& InOutString);
};
