// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "UI/Inventory/InventoryTypes.h"
#include "ItemDragDropOperation.generated.h"

/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	FItemMoveData ItemMoveData;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UUserWidget> WidgetReference;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D DragOffset;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
