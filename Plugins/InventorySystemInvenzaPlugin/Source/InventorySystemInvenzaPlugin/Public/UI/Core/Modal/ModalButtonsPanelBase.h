// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interface/UIInterface.h"
#include "Interface/UI/ModalInterface.h"
#include "UI/InvenzaBaseWidget.h"
#include "ModalButtonsPanelBase.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UModalButtonsPanelBase : public UInvenzaBaseWidget, public IUIInterface
{
	GENERATED_BODY()
	
public:
	UModalButtonsPanelBase() {}
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UPanelWidget> Panel_Container;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	TArray<UUIButton*> GetButtons_Implementation();
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
