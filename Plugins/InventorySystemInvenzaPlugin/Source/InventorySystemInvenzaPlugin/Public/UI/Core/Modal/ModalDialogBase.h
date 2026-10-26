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
	UModalDialogBase() {}

protected:
	// Unreal overrides
	virtual void NativeConstruct() override;

public:
	// Properties and variables
	UPROPERTY(BlueprintReadWrite, Category = "Modal")
	FModalResultDelegate DynamicResultDelegate;

	// Widgets
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UNamedSlot> Upper_Slot;

	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UNamedSlot> Down_Slot;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Modal")
	void ForceClose(FModalResult Result);

	UFUNCTION(Category = "Invenza|UI|Modal", BlueprintCallable)
	void Configure(FModalHeaderData HeaderData, const TArray<EObjectInteractionType>& Actions,
				   const TArray<FModalAction>& Display);

	UFUNCTION(Category = "Invenza|UI|Modal", BlueprintCallable)
	void ConfigureHeader(FModalHeaderData HeaderData);

	UFUNCTION(Category = "Invenza|UI|Modal", BlueprintCallable)
	void ConfigureFooter(const TArray<EObjectInteractionType>& Actions, const TArray<FModalAction>& Display);

protected:
	// Runtime
	// Properties and variables
	UPROPERTY()
	TMap<TObjectPtr<UUIButton>, FModalResult> ButtonToResultMap;

	// Functions
	UFUNCTION(Category = "Invenza|UI|Modal", BlueprintCallable)
	void ConfigureButtons(TArray<UUIButton*> InBtns, const TArray<EObjectInteractionType>& Actions,
						  const TArray<FModalAction>& Display);

	UFUNCTION()
	void OnButtonClicked(UUIButton* UIButton);
};
