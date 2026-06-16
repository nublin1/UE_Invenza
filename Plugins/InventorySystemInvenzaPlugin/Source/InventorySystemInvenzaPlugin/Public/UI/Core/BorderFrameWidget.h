// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "BorderFrameWidget.generated.h"

class UBorder;

USTRUCT(BlueprintType)
struct FBorderFrameStyle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateBrush Brush;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BrushColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMargin Padding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D DesiredSize = FVector2D(1.f);
};

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UBorderFrameWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()
	
public:
	UBorderFrameWidget();
	
	virtual void NativePreConstruct() override;
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> TopBorder;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> BottomBorder;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> LeftBorder;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> RightBorder;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UNamedSlot> NamedSlot;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Top Border")
	FBorderFrameStyle TopBorderStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bottom Border")
	FBorderFrameStyle BottomBorderStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Left Border")
	FBorderFrameStyle LeftBorderStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Right Border")
	FBorderFrameStyle RightBorderStyle;
};
