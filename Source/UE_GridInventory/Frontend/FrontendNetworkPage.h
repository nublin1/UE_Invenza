//  Nublin Studio 2026 All Rights Reserved.

#pragma once
#include "UI/InvenzaBaseWidget.h"
#include "Frontend/FrontendTypes.h"
#include "FrontendNetworkPage.generated.h"

class UComboBoxString;
class UEditableTextBox;
class UTextBlock;
class UUIButton;
class UVerticalBox;

UCLASS(Abstract)
class UE_GRIDINVENTORY_API UFrontendNetworkPage : public UInvenzaBaseWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	// Properties and variables
	UPROPERTY(BlueprintAssignable, Category = "Frontend")
	FFrontendPageBackRequested OnBackRequested;
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUIButton> ServerButtonClass;
	UPROPERTY(EditDefaultsOnly, Category = "Options")
	int32 MinimumPlayers = 2;
	UPROPERTY(EditDefaultsOnly, Category = "Options")
	int32 MaximumPlayers = 8;
	UPROPERTY(EditDefaultsOnly, Category = "Options")
	int32 DefaultPlayers = 4;
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Frontend")
	void Activate();

protected:
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UTextBlock> NetworkStatus;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UTextBlock> BackendLabel;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UEditableTextBox> ServerName;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UEditableTextBox> IPAddress;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UComboBoxString> PlayerCount;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UVerticalBox> ServerList;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> HostGame;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> ConnectIP;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> FindGames;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> CancelSearch;
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UUIButton> BackHome;
	// Runtime
	UPROPERTY(Transient)
	TObjectPtr<class UInvenzaSessionsSubsystem> Network;
	UPROPERTY(Transient)
	TMap<TObjectPtr<UUIButton>, int32> ServerButtons;
	// Functions
	UFUNCTION()
	void Clicked(UUIButton* Button);
	UFUNCTION()
	void RefreshNetwork();
};
