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
	
public:
	UModalDialogBase(){}
	
	virtual void NativeConstruct() override;
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintReadWrite, Category = "Modal")
	FModalResultDelegate DynamicResultDelegate;
	
	// Widgets
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UNamedSlot> Upper_Slot;
	
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UNamedSlot> Down_Slot;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION(BlueprintCallable, Category = "Modal")
	void ForceClose(FModalResult Result);
	
	UFUNCTION(BlueprintCallable)
	void Configure(const TArray<EObjectInteractionType>& Actions, const TArray<FModalAction>& Display);
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY()
	TMap<TObjectPtr<UUIButton>, FModalResult> ButtonToResultMap;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION(BlueprintCallable)
	void ConfigureButtons(TArray<UUIButton*> InBtns, const TArray<EObjectInteractionType>& Actions, const TArray<FModalAction>& Display);
	
	UFUNCTION()
	void OnButtonClicked(UUIButton* UIButton);
};
