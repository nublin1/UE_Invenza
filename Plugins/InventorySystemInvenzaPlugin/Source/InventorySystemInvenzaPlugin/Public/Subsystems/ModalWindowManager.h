// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/Core/Modal/ModalDialogBase.h"
#include "UI/Core/Modal/ModalTypes.h"
#include "UI/ModalLayout/ModalLayout.h"
#include "ModalWindowManager.generated.h"

/**
 *
 */

class UPrimaryLayout;

UCLASS(Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API UModalWindowManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UModalWindowManager();

protected:
	// Unreal overrides
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
	// Properties and variables
	UPROPERTY()
	FModalResultDelegate FinalDelegate;

	// Functions
	UFUNCTION(Category = "Invenza|ModalWindowManager", BlueprintCallable)
	virtual void InitializeUI();

	UFUNCTION(BlueprintCallable, Category = "Modal|Manager", meta = (WorldContext = "WorldContextObject"))
	void ForceCancelModalFlow();

	/** Cancels only the flow belonging to this interaction, leaving other dialogs alone. */
	void CancelModalFlowFor(const UObject* ContextObject);

	UFUNCTION(BlueprintCallable, Category = "Modal|Controls Help")
	void ShowControlsHelp();

	UFUNCTION(BlueprintCallable, Category = "Modal|Controls Help")
	void CloseControlsHelp();

	UFUNCTION(BlueprintPure, Category = "Modal|Controls Help")
	bool IsControlsHelpOpen() const { return ControlsHelpWindow != nullptr; }

	/**
	* Creates a modal window using the type configured in global settings.
	*/
	UFUNCTION(BlueprintCallable, Category = "Modal|Manager", meta = (WorldContext = "WorldContextObject"))
	void OpenModalFlow(UObject* InObject, FModalHeaderData HeaderData, EModalFooterType FooterType,
					   const TMap<EObjectInteractionType, FModalActionConfig>& Actions, FModalResultDelegate OnResult);

	void ShowModalStep(FModalHeaderData HeaderData, EModalFooterType FooterType,
					   const TMap<EObjectInteractionType, FModalActionConfig>& Actions);

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Transient)
	TObjectPtr<UControlsHelpWidget> ControlsHelpWindow;

	UPROPERTY()
	TObjectPtr<UObject> CachedContextObject;

	UPROPERTY()
	FModalResult PendingOriginalResult;

	UPROPERTY()
	FModalActionConfig PendingOriginalAction;

	UPROPERTY()
	TMap<EObjectInteractionType, FModalActionConfig> CurrentStepActionsMap;

	// Refs
	UPROPERTY(Category = "Invenza|ModalWindowManager", VisibleInstanceOnly, BlueprintReadWrite)
	const UInvenzaInventorySettingsAsset* InvenzaInventorySettingsAsset;

	UPROPERTY(Category = "Invenza|ModalWindowManager", VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UPrimaryLayout> PrimaryLayoutRef;

	UPROPERTY(Category = "Invenza|ModalWindowManager", VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UUILayer> ModalLayoutRef;

	// Functions
	UFUNCTION()
	void HandleModalResponse(FModalResult Result);

	void ClearModalWindows();

	static void AttachChildWidget(UWorld* World, UPanelWidget* Slot, TSubclassOf<UUserWidget> WidgetClass);
};
