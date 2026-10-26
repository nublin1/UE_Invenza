//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InvenzaBaseWidget.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaBaseWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void NativeConstruct() override;

public:
	// Functions
	UFUNCTION()
	virtual void CalculateParentWidget();

	UFUNCTION()
	virtual void SetParentWidget(UInvenzaBaseWidget* Parent) { ParentWidget = Parent; }

protected:
	// Runtime
	// Properties and variables
	UPROPERTY()
	TObjectPtr<UInvenzaBaseWidget> ParentWidget;
};
