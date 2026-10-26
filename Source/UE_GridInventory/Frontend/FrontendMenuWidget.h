//  Nublin Studio 2026 All Rights Reserved.

#pragma once
#include "UI/InvenzaBaseWidget.h"
#include "Frontend/FrontendTypes.h"
#include "FrontendMenuWidget.generated.h"

class UUIButton;
class UWidgetSwitcher;
class UPanelWidget;
class ULabelBaseText;
class UFrontendSettingsPage;
class UFrontendNetworkPage;
class UInvenzaSaveWidget;

UCLASS()
class UE_GRIDINVENTORY_API UFrontendSessionBar : public UInvenzaBaseWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> LeaveGame;
	// Functions
	UFUNCTION()
	void ClickMenu(UUIButton* Button);
};

UCLASS(Abstract)
class UE_GRIDINVENTORY_API UFrontendMenuWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) override;

public:
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Frontend")
	void ShowPage(EFrontendPage Page);

protected:
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> Pages;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UPanelWidget> HomePage;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UFrontendSettingsPage> SettingsPage;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UFrontendNetworkPage> NetworkPage;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UInvenzaSaveWidget> SavePage;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UInvenzaSaveWidget> LoadPage;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> ResumeButton;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> OpenSave;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> OpenLoad;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> OpenSettings;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> OpenNetwork;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> LeaveSession;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> QuitGame;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> MenuStatus;
	// Runtime
	bool bPlaying = false;
	// Functions
	UFUNCTION()
	void Clicked(UUIButton* Button);
	UFUNCTION()
	void BackHome();
};
