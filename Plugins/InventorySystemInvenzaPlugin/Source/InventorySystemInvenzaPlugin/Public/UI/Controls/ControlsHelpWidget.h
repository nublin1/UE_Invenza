//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "UI/InvenzaBaseWidget.h"
#include "UI/Controls/ControlsHelpTypes.h"
#include "UI/UIStructs.h"
#include "ControlsHelpWidget.generated.h"

class UUIButton;
class ULabelBaseText;
class UScrollBox;
class UControlsHelpRowWidget;

UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UControlsHelpWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UControlsHelpWidget();

protected:
	// Unreal overrides
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) override;

public:
	// Properties and variables
	UPROPERTY(Category = "Invenza|UI|Controls", BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> ControlsList;

	UPROPERTY(Category = "Invenza|UI|Controls", BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UUIButton> OKButton;

	UPROPERTY(EditDefaultsOnly, Category = "Controls Help")
	TSubclassOf<ULabelBaseText> LabelClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Controls Help|Appearance")
	FLabelStyle RowStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Controls Help|Appearance")
	FLabelStyle HeadingStyle;

	UPROPERTY(EditDefaultsOnly, Category = "Controls Help|Appearance")
	FMargin HeadingPadding = FMargin(0, 12, 0, 6);

	UPROPERTY(EditDefaultsOnly, Category = "Controls Help|Appearance")
	FMargin RowPadding = FMargin(0, 5, 12, 5);

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Controls Help")
	void RefreshControls();

	UFUNCTION(BlueprintCallable, Category = "Controls Help")
	void SetRows(const TArray<FControlsHelpRow>& Rows);

	void ReleaseInput();

protected:
	// Runtime

	// Functions
	UFUNCTION()
	void HandleOK(UUIButton* Button);
};
