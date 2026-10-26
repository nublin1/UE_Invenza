// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "CraftMenuRecipeActions.generated.h"

class UUIButton;
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
	// Unreal overrides
	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;

public:
	// Properties and variables
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UUIButton> Btn_Craft;
};
