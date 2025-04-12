//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventorySlot.h"
#include "UI/BaseUserWidget.h"
#include "SlotbasedInventorySlot.generated.h"

class UCoreCellWidget;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API USlotbasedInventorySlot : public UInventorySlot
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	USlotbasedInventorySlot();

	bool operator==(const USlotbasedInventorySlot& other) const
	{
		return SlotPosition == other.GetSlotPosition();
	}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCoreCellWidget> CoreCellWidget;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
