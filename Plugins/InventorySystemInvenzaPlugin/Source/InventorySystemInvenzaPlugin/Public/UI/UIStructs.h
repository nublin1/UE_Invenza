// Nublin Studio 2026 All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "UIStructs.generated.h"

USTRUCT(BlueprintType)
struct FLabelStyle
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
struct FUIBrushStyle
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FSlateBrush Brush;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FLinearColor BrushColor  = FLinearColor::White;
};