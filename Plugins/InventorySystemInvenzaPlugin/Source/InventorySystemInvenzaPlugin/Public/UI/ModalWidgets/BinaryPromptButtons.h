// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "BinaryPromptButtons.generated.h"

class UUIButton;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UBinaryPromptButtons : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(meta=(bindWidget))
	TObjectPtr<UUIButton> ConfirmButton;
	UPROPERTY(meta=(bindWidget))
	TObjectPtr<UUIButton> CancelButton;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UBinaryPromptButtons();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
