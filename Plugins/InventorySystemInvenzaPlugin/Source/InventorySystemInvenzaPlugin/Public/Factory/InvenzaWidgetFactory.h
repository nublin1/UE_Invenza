//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InvenzaWidgetFactory.generated.h"

class UOperationPanelWidget;
class UUInventoryWidgetBase;
class UInvBaseContainerWidget;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaWidgetFactory : public UObject
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(blueprintCallable, Category = "InvenzaWidgetFactory")
	static UInvBaseContainerWidget* CreateInventoryWidget(APlayerController* OwningPlayer,
		TSubclassOf<UInvBaseContainerWidget> ContainerWidgetClass,
		TSubclassOf<UUInventoryWidgetBase> InventoryWidgetClass,
		TSubclassOf<UOperationPanelWidget> OperationPanelClass);
};
