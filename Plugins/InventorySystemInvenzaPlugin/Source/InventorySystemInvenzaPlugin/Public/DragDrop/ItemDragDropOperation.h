//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "UI/Inventory/InventoryTypes.h"
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
	UPROPERTY(BlueprintReadWrite)
	FItemMoveData ItemMoveData;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D DragOffset;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UUserWidget> WidgetReference;
	
	//===================================================================
	// FUNCTIONS
	//====================================================================
};
