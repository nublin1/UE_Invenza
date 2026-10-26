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

public:
	// Properties and variables
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UPanelWidget> Panel_Container;

	// Functions
	virtual TArray<UUIButton*> GetButtons_Implementation() const override;
};
