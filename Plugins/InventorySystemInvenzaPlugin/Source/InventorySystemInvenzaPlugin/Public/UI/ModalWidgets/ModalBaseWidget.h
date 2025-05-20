// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "Components/NamedSlot.h"
#include "ModalBaseWidget.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UModalBaseWidget : public UBaseUserWidget
{
	GENERATED_BODY()
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UModalBaseWidget();

	UNamedSlot* GetSlot_Content() { return Slot_Content; }
	UNamedSlot* GetSlot_Interaction() { return Slot_Interaction; }

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModalWidgets", meta=(BindWidget))
	TObjectPtr<UNamedSlot> Slot_Content;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModalWidgets", meta=(BindWidget))
	TObjectPtr<UNamedSlot> Slot_Interaction;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
