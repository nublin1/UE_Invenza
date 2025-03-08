// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "BaseInventorySlot.generated.h"

class UBorder;
class UTextBlock;
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

	//Getters
	FORCEINLINE FIntVector2 GetSlotPosition() const { return SlotPosition; }	
	
	//Setters	
	FORCEINLINE void SetSlotPosition(const FIntVector2 InSlotPosition) { this->SlotPosition = InSlotPosition; }

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    TObjectPtr<UBorder> Left_Border;
    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    TObjectPtr<UBorder>  Right_Border;
    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    TObjectPtr<UBorder>  Top_Border;
    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    TObjectPtr<UBorder>  BottomBorder;
    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    TObjectPtr<UTextBlock> Content_Text_Name;

	UPROPERTY()
	FIntVector2 SlotPosition;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
