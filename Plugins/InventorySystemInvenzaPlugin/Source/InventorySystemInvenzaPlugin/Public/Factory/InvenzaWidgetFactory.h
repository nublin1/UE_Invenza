//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InvenzaWidgetFactory.generated.h"

class UCraftDashboard;
class UInvenzaBaseWidget;
class UOperationPanelWidget;
class UUInventoryBaseWidget;
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
		TSubclassOf<UUInventoryBaseWidget> InventoryWidgetClass,
		TSubclassOf<UOperationPanelWidget> OperationPanelClass);

	UFUNCTION(blueprintCallable, Category = "InvenzaWidgetFactory")
	static UInvenzaBaseWidget* CreateInvenzaWidget(APlayerController* Owner,
		TSubclassOf<UInvenzaBaseWidget> InvenzaBaseWidgetClass);
};
