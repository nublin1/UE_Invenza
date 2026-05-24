// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "ProgressPercentTimer.generated.h"

class UCurrentMaxDisplay;
class UTimerWidget;
class UProgressBar;
class ULabelBaseText;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UProgressPercentTimer : public UInvenzaBaseWidget
{
	GENERATED_BODY()
	
	
public:
	UProgressPercentTimer();

protected:


public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> Percent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UCurrentMaxDisplay> RemainingCounter;
	

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void SetPercentText(const FString& InText);

	UFUNCTION()
	void SetProgressPercent(const FString& InText);
};
