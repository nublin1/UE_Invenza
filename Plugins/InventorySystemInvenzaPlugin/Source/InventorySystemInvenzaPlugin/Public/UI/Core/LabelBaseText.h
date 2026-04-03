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
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UTextBlock> MainTextBlock;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable)
	void UpdateText(FText NewText);
	UFUNCTION(BlueprintCallable)
	void UpdateFont(FSlateFontInfo NewFontInfo);
	UFUNCTION(BlueprintCallable)
	void UpdateTextAndFont(FLabelStyle LabelStyle);
	UFUNCTION(BlueprintCallable)
	void SyncTextAndFont();
	UFUNCTION(BlueprintCallable)
	void ClearText();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Config
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FText Text;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FSlateColor Color;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FSlateFontInfo FontInfo;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
};
