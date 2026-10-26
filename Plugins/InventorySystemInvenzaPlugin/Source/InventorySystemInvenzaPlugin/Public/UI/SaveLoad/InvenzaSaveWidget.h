//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "UI/InvenzaBaseWidget.h"
#include "SaveLoad/InventorySaveTypes.h"
#include "InvenzaSaveWidget.generated.h"

class UUIButton;
class ULabelBaseText;
class UComboBoxString;
class UCheckBox;
class UPanelWidget;

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInvenzaSaveCloseRequested);
#pragma endregion Delegates

/** Behavior for the designer-authored save menu. */
UCLASS(Abstract)
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaSaveWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;

public:
	// Properties and variables
	UPROPERTY(BlueprintAssignable, Category = "Save|Events")
	FInvenzaSaveCloseRequested OnCloseRequested;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Save|View")
	EInvenzaSaveView View = EInvenzaSaveView::All;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Save|View")
	bool bEmbeddedMenuPage = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Save|View")
	TSoftObjectPtr<UWorld> LoadWorld;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Invenza|Save")
	void Refresh();

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> SaveControls;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> LoadControls;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> SessionControls;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> StatusLabel;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidget))
	TObjectPtr<UComboBoxString> Slots;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidget))
	TObjectPtr<UCheckBox> Trust;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidget))
	TObjectPtr<UPanelWidget> Confirmation;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> ContinueButton;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> QuickSaveButton;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> ManualSave1Button;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> ManualSave2Button;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> ManualSave3Button;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> LoadButton;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> RecoverButton;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> ResetButton;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> ConfirmResetButton;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> CancelResetButton;

	UPROPERTY(BlueprintReadOnly, Category = "Save|Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> CloseButton;

	TMap<FString, FString> SlotKeysByLabel;

	bool bRefreshing = false;

	// Functions
	UFUNCTION()
	void Clicked(UUIButton* Button);

	UFUNCTION()
	void TrustChanged(bool bChecked);

	void RequestClose();
};
