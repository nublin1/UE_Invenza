// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "LabelBaseText.generated.h"

struct FLabelStyle;
class UTextBlock;
/**
 *
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API ULabelBaseText : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	ULabelBaseText();

protected:
	// Unreal overrides
	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;

public:
	// Properties and variables
	//Widgets
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UTextBlock> MainTextBlock;

	// Functions
	UFUNCTION(Category = "Invenza|UI|Core", BlueprintCallable)
	void UpdateText(FText NewText);

	UFUNCTION(Category = "Invenza|UI|Core", BlueprintCallable)
	void UpdateFont(FSlateFontInfo NewFontInfo);

	UFUNCTION(Category = "Invenza|UI|Core", BlueprintCallable)
	void UpdateTextAndFont(FLabelStyle LabelStyle);

	UFUNCTION(Category = "Invenza|UI|Core", BlueprintCallable)
	void SyncTextAndFont();

	UFUNCTION(Category = "Invenza|UI|Core", BlueprintCallable)
	void ClearText();

protected:
	// Runtime
	// Properties and variables
	// Config
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FSlateColor Color;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FSlateFontInfo FontInfo;
};
