//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UBaseUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	virtual void CalculateParentWidget();
	
	UFUNCTION()
	virtual void SetParentWidget(UBaseUserWidget* Parent) {ParentWidget = Parent;}

protected:
	UPROPERTY()
	TObjectPtr<UUserWidget> ParentWidget;

	//
	virtual void NativeConstruct() override;

};
