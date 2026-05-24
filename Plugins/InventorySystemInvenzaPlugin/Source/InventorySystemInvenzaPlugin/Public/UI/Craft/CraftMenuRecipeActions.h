// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "CraftMenuRecipeActions.generated.h"

class UActionButtonUI;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UCraftMenuRecipeActions : public UInvenzaBaseWidget
{
	GENERATED_BODY()

	
public:
	UCraftMenuRecipeActions();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UActionButtonUI> Btn_Craft;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
