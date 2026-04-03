//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "UILayer.generated.h"

class UBorder;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UUILayer : public UInvenzaBaseWidget
{
	GENERATED_BODY()

	
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UUILayer();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UBorder> MainBorder;

	//
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInvenzaBaseWidget> PushedWidget;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
