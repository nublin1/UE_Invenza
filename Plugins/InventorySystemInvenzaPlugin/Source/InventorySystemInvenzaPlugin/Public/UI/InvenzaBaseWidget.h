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
	virtual void NativeConstruct() override;

public:
	UFUNCTION()
	virtual void CalculateParentWidget();
	
	UFUNCTION()
	virtual void SetParentWidget(UInvenzaBaseWidget* Parent) {ParentWidget = Parent;}

protected:
	UPROPERTY()
	TObjectPtr<UInvenzaBaseWidget> ParentWidget;

};
