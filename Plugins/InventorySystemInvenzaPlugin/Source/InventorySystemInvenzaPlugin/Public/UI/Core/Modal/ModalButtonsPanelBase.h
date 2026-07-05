// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interface/UI/ModalButtonsPanelInterface.h"
#include "UI/InvenzaBaseWidget.h"
#include "ModalButtonsPanelBase.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UModalButtonsPanelBase : public UInvenzaBaseWidget, public IModalButtonsPanelInterface
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

	virtual TArray<UUIButton*> GetButtons_Implementation() override;
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
