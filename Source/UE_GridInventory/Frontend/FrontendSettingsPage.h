//  Nublin Studio 2026 All Rights Reserved.

#pragma once
#include "UI/InvenzaBaseWidget.h"
#include "Frontend/FrontendTypes.h"
#include "FrontendSettingsPage.generated.h"

class UCheckBox;
class UComboBoxString;
class USlider;
class UTextBlock;
class UUIButton;
class UWidgetSwitcher;

UCLASS(Abstract)
class UE_GRIDINVENTORY_API UFrontendSettingsPage : public UInvenzaBaseWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	// Properties and variables
	UPROPERTY(BlueprintAssignable, Category = "Frontend")
	FFrontendPageBackRequested OnBackRequested;
	UPROPERTY(EditDefaultsOnly, Category = "Options")
	TArray<FIntPoint> ResolutionPresets;
	UPROPERTY(EditDefaultsOnly, Category = "Options")
	TArray<FText> WindowModeLabels;
	UPROPERTY(EditDefaultsOnly, Category = "Options")
	TArray<FText> QualityLabels;
	UPROPERTY(EditDefaultsOnly, Category = "Options")
	TArray<float> FrameLimitPresets;
	UPROPERTY(EditDefaultsOnly, Category = "Options", meta = (ClampMin = "1"))
	int32 DisplayConfirmationSeconds = 15;
	bool IsConfirming() const
	{
		return bConfirming;
	}

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Frontend")
	void Activate();

protected:
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> SettingsPages;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UComboBoxString> Resolution;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UComboBoxString> WindowMode;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UComboBoxString> Quality;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UComboBoxString> FrameLimit;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UCheckBox> VSync;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<USlider> MasterVolume;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<USlider> MusicVolume;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<USlider> EffectsVolume;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UTextBlock> SettingsStatus;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UTextBlock> ConfirmLabel;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> ApplySettings;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> ResetSettings;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> SettingsBack;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> KeepDisplay;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> RevertDisplay;
	// Runtime
	TArray<FIntPoint> Resolutions;
	TArray<float> FrameLimits;
	FTimerHandle ConfirmTimer;
	int32 ConfirmSeconds = 0;
	bool bConfirming = false;
	// Functions
	void LoadControls();
	void ApplyControls();
	void FinishConfirmation(bool bKeep);
	void ConfirmTick();
	UFUNCTION()
	void Clicked(UUIButton* Button);
};
