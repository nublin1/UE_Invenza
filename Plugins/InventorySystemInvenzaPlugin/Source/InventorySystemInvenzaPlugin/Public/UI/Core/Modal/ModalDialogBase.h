// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModalTypes.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Core/Buttons/UIButton.h"
#include "ModalDialogBase.generated.h"

class ULabelBaseText;
struct FModalResult;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UModalDialogBase : public UInvenzaBaseWidget
{
	GENERATED_BODY()
	
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModalClosed, FModalResult, Result);
#pragma endregion Delegates
	
public:
	UModalDialogBase(){}
	
	virtual void NativeConstruct() override;
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> Text_Title;
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> Text_Message;
	
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UNamedSlot> Btns_Slot;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION(BlueprintCallable, Category = "Modal")
	void ForceClose(FModalResult Result);
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION(BlueprintCallable)
	void Configure();
	
	UFUNCTION()
	void OnButtonClicked(UUIButton* UIButton);
};
