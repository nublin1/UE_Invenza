// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "UIButton.generated.h"

class USizeBox;
class UButton;
class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UUIButton : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
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

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere)
	bool bIsToggleButton;
	UPROPERTY(EditAnywhere)
	bool bIsToggleOn;

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	FText DefaultText = FText::FromString(" ");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	FVector2D DefaultSize = FVector2D(64.f, 64.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	TObjectPtr<UTexture2D> DefaultImage;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativePreConstruct() override;
};
