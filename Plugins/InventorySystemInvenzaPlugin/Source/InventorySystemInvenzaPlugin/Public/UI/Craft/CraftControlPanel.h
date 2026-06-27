// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interface/UIInterface.h"
#include "UI/InvenzaBaseWidget.h"
#include "CraftControlPanel.generated.h"

class UUIButton;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UCraftControlPanel : public UInvenzaBaseWidget, public IUIInterface
{
	GENERATED_BODY()
	
public:
	UCraftControlPanel();

protected:
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> Btn_AddTask;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> Btn_Pause;
	
	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	//TObjectPtr<UUIButton> ClearQueueButton;

	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual TArray<UUIButton*> GetButtons_Implementation() const override;


protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
