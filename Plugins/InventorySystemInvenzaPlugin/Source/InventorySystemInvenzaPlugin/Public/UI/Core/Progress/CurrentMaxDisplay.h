// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "CurrentMaxDisplay.generated.h"

class ULabelBaseText;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UCurrentMaxDisplay : public UInvenzaBaseWidget
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void NativePreConstruct() override;

public:
	// Properties and variables
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> PrefixText;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> CurrentValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> SeparatorSymbol;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> MaxValue;
};
