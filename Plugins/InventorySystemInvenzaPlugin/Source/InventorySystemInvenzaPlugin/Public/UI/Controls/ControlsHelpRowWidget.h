// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "UI/InvenzaBaseWidget.h"
#include "UI/UIStructs.h"
#include "ControlsHelpRowWidget.generated.h"

class ULabelBaseText;

/** Behavior for a designer-authored action/key row. */
UCLASS(Abstract)
class INVENTORYSYSTEMINVENZAPLUGIN_API UControlsHelpRowWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, Category = "Controls Help", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> ActionLabel;

	UPROPERTY(BlueprintReadOnly, Category = "Controls Help", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> KeysLabel;

	// Functions
	void SetRow(const FText& Action, const FText& Keys, const FLabelStyle& Style);
};
