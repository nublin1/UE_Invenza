//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "InvBaseContainerWidget.generated.h"

class UBaseInventoryWidget;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UInvBaseContainerWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInvBaseContainerWidget();

	UFUNCTION(BlueprintCallable)
	virtual UBaseInventoryWidget* GetInventoryFromContainerSlot();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta=(BindWidgetOptional))
	TObjectPtr<UNamedSlot> ContainerSlot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta=(BindWidgetOptional))
	TObjectPtr<UNamedSlot> BottomSlot;
	

	//====================================================================
	// FUNCTIONS
	//====================================================================
};
