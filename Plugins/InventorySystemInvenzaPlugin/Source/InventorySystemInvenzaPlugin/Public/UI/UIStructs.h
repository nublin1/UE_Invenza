// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"
#include "UIStructs.generated.h"

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FLabelStyle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config|Label")
	FText Text;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config|Label")
	FSlateColor Color;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config|Label")
	FSlateFontInfo FontInfo;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FUIBrushStyle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FSlateBrush Brush;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FLinearColor BrushColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FBorderFrameStyle
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|UI|Core", EditAnywhere, BlueprintReadWrite)
	FSlateBrush Brush;

	UPROPERTY(Category = "Invenza|UI|Core", EditAnywhere, BlueprintReadWrite)
	FLinearColor BrushColor = FLinearColor::White;

	UPROPERTY(Category = "Invenza|UI|Core", EditAnywhere, BlueprintReadWrite)
	FMargin Padding;

	UPROPERTY(Category = "Invenza|UI|Core", EditAnywhere, BlueprintReadWrite)
	FVector2D DesiredSize = FVector2D(1.f);
};
