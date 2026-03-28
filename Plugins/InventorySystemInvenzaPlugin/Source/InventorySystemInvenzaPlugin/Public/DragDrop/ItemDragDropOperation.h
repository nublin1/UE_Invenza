//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Data/Inventory/InventoryTypes.h"
#include "Blueprint/UserWidget.h"
#include "ItemDragDropOperation.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRotationChanged, EItemOrientationType, NewOrientationType);
#pragma endregion Delegates

public:
	UItemDragDropOperation();
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable, Category = "Drag & Drop")
	FOnRotationChanged OnRotationChanged;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Drag & Drop")
	FItemMoveData ItemMoveData;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Drag & Drop")
	FVector2D DragOffset;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Drag & Drop")
	void RotateDraggedWidget();

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
