//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "DragContainerWidget.generated.h"

class UCoreCellWidget;
class USizeBox;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UDragContainerWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UDragContainerWidget();
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TitleBar", meta = (BindWidgetOptional))
	TObjectPtr<UCoreCellWidget> CoreCellWidget;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
