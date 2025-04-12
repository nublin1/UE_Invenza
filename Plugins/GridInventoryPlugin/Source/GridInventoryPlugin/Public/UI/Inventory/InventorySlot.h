//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "InventorySlot.generated.h"

/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UInventorySlot : public UBaseUserWidget
{
	GENERATED_BODY()
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInventorySlot();
	
	//Getters
	FORCEINLINE FIntVector2 GetSlotPosition() const { return SlotPosition; }	
	
	//Setters	
	FORCEINLINE void SetSlotPosition(const FIntVector2 InSlotPosition) { this->SlotPosition = InSlotPosition; }
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY()
	FIntVector2 SlotPosition{};
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
