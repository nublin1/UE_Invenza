// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "UI/InvenzaBaseWidget.h"
#include "GameplayTagContainer.h"
#include "UI/UIStructs.h"
#include "Engine/TimerHandle.h"
#include "LayersDebuggerWidget.generated.h"

class UDynamicEntryBox;
class UPrimaryLayout;

/** Event-driven diagnostics; all entries and their layout are authored in Widget Blueprint. */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API ULayersDebuggerWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	// Properties and variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Invenza|UI|Layout|Debug")
	FLabelStyle EntryStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Invenza|UI|Layout|Debug")
	FSlateColor ActiveColor;

	// Functions
	void SetLayout(UPrimaryLayout* Layout);

	UFUNCTION(BlueprintCallable, Category = "Invenza|UI|Layout|Debug")
	void RefreshDiagnostics();

	UFUNCTION(BlueprintPure, Category = "Invenza|UI|Layout|Debug")
	FText GetLayerDiagnosticText(FGameplayTag LayerTag) const;

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Transient)
	TWeakObjectPtr<UPrimaryLayout> PrimaryLayout;

	TArray<TWeakObjectPtr<UUserWidget>> ObservedWidgets;
	FTimerHandle RefreshTimer;
	bool bRefreshQueued = false;

	// Functions
	UFUNCTION()
	void QueueRefresh();

	void ObserveWidget(UUserWidget* Widget);
	void ClearObservedWidgets();
	void VisibilityChanged(ESlateVisibility NewVisibility);
};
