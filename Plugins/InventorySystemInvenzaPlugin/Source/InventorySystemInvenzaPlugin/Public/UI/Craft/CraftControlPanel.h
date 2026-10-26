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
	// Unreal overrides
	virtual void NativeConstruct() override;

public:
	// Properties and variables
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> Btn_AddTask;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> Btn_Pause;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> Btn_AutomaticCrafting;

	// Functions
	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	//TObjectPtr<UUIButton> ClearQueueButton;

	virtual TArray<UUIButton*> GetButtons_Implementation() const override;
};
