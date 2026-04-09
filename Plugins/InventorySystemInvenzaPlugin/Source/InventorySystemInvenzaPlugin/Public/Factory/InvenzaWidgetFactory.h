//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InvenzaWidgetFactory.generated.h"

class UOperationPanelWidget;
class UUInventoryWidgetBase;
class UInventoryContainerWidget;
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
	static UInventoryContainerWidget* CreateInventoryWidget(APlayerController* OwningPlayer,
		TSubclassOf<UInventoryContainerWidget> ContainerWidgetClass,
		TSubclassOf<UUInventoryWidgetBase> InventoryWidgetClass,
		TSubclassOf<UOperationPanelWidget> OperationPanelClass);
};
