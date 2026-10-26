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

public:
	UEditableLabelBaseText();

protected:
	// Unreal overrides
	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEditableTextChanged, const FText&, NewText);
#pragma endregion Delegates

	// Properties and variables
	// WIDGETS

	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UEditableTextBox> MainEditableTextBox;

	// CONFIG

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FText HintText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FSlateColor Color;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FSlateFontInfo FontInfo;

	// EVENTS

	UPROPERTY(BlueprintAssignable, Category = "UI|Events")
	FOnEditableTextChanged OnEditableTextChanged;

	// Functions
	// API

	UFUNCTION(Category = "Invenza|UI|Core", BlueprintCallable)
	void UpdateText(FText NewText);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHintText(FText NewHintText);

	UFUNCTION(Category = "Invenza|UI|Core", BlueprintCallable)
	void UpdateFont(FSlateFontInfo NewFontInfo);

	UFUNCTION(Category = "Invenza|UI|Core", BlueprintCallable)
	void UpdateTextAndFont(FLabelStyle LabelStyle);

	UFUNCTION(Category = "Invenza|UI|Core", BlueprintCallable)
	void SyncTextAndFont();

	UFUNCTION(Category = "Invenza|UI|Core", BlueprintCallable)
	void ClearText();

	UFUNCTION(Category = "Invenza|UI|Core", BlueprintCallable)
	FText GetText() const;

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	bool bNumericOnly = false;

	UPROPERTY()
	FText CachedText;

	// Functions
	UFUNCTION()
	void HandleTextChanged(const FText& NewText);

	UFUNCTION()
	void ApplyNumericFilter(FString& InOutString);
};
