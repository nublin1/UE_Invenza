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
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UPROPERTY()
	FModalResultDelegate FinalDelegate;
	
	UFUNCTION(BlueprintCallable)
	virtual void InitializeUI();
	
	/**
	* Создает модальное окно на основе типа из глобальных настроек.
	*/
    UFUNCTION(BlueprintCallable, Category = "Modal|Manager", meta = (WorldContext = "WorldContextObject"))
	void OpenModalFlow(
		EModalHeaderType HeaderType,
		const FText& HeaderText,
		EModalFooterType FooterType,
		const TMap<EObjectInteractionType, FModalActionConfig>& Actions,
		FModalResultDelegate OnResult);
	
	void ShowModalStep(EModalHeaderType HeaderType, const FText& HeaderText, EModalFooterType FooterType,
		const TMap<EObjectInteractionType, FModalActionConfig>& Actions);

protected:

	UPROPERTY()
	TWeakObjectPtr<UObject> CachedContextObject;
	
	UPROPERTY()
	FModalResult PendingOriginalResult;
	
	UPROPERTY()
	FModalActionConfig PendingOriginalAction;
	
	UPROPERTY()
	TMap<EObjectInteractionType, FModalActionConfig> CurrentStepActionsMap;
	
	// Refs
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	const UInvenzaInventorySettingsAsset* InvenzaInventorySettingsAsset;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UPrimaryLayout> PrimaryLayoutRef;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UUILayer> ModalLayoutRef;
	
	UFUNCTION()
	void HandleModalResponse(FModalResult Result);
	
	static void AttachChildWidget(UWorld* World, UPanelWidget* Slot, TSubclassOf<UUserWidget> WidgetClass);
};
