// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "BaseInventorySlot.generated.h"

class UCoreCellWidget;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UBaseInventorySlot : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UBaseInventorySlot();

	bool operator==(const UBaseInventorySlot& other) const
	{
		return SlotPosition == other.GetSlotPosition();
	}

	//Getters
	FORCEINLINE FIntVector2 GetSlotPosition() const { return SlotPosition; }	
	
	//Setters	
	FORCEINLINE void SetSlotPosition(const FIntVector2 InSlotPosition) { this->SlotPosition = InSlotPosition; }

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCoreCellWidget> CoreCellWidget;
	
	//
	UPROPERTY()
	FIntVector2 SlotPosition;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
