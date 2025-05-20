// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "InvContainerDragDropOperation.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvContainerDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D DragOffset;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInvContainerDragDropOperation();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//===================================================================
	// FUNCTIONS
	//====================================================================
};
