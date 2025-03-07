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

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    UBorder* Left_Border;
    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    UBorder* Right_Border;
    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    UBorder* Top_Border;
    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    UBorder* BottomBorder;
    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    UTextBlock* Content_Text_Name;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
