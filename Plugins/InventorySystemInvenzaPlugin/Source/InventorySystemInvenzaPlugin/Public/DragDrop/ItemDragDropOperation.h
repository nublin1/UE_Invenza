//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "UI/Inventory/InventoryTypes.h"
#include "Blueprint/UserWidget.h"
#include "ItemDragDropOperation.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Drag & Drop")
	FItemMoveData ItemMoveData;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Drag & Drop")
	FVector2D DragOffset;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Drag & Drop")
	TObjectPtr<UUserWidget> WidgetReference;
	
	//===================================================================
	// FUNCTIONS
	//====================================================================
};
