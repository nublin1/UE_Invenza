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

public:
	virtual void NativePreConstruct() override;
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> PrefixText;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> CurrentValue;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> SeparatorSymbol;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> MaxValue;

	//====================================================================
	// FUNCTIONS
	//====================================================================

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Editable text values
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FText PreTextValue = FText::FromString(TEXT("Remaining"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	FText SeparatorSymbolValue = FText::FromString(TEXT("/"));

	//====================================================================
	// FUNCTIONS
	//====================================================================
};
