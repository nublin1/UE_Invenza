// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/UIStructs.h"
#include "BorderFrameWidget.generated.h"

class UBorder;

/**
 *
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UBorderFrameWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UBorderFrameWidget();

protected:
	// Unreal overrides
	virtual void NativePreConstruct() override;

public:
	// Properties and variables
	// Widgets
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> TopBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> BottomBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> LeftBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> RightBorder;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UNamedSlot> NamedSlot;

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Top Border")
	FBorderFrameStyle TopBorderStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bottom Border")
	FBorderFrameStyle BottomBorderStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Left Border")
	FBorderFrameStyle LeftBorderStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Right Border")
	FBorderFrameStyle RightBorderStyle;
};
