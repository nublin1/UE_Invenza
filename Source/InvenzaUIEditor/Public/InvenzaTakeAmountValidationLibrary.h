// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "InvenzaTakeAmountValidationLibrary.generated.h"

class UUserWidget;
class UWidget;
class UModalDialogBase;

/** Project-editor-only, transient network probe. Never included in the distributable plugin. */
UCLASS()
class INVENZAUIEDITOR_API UInvenzaTakeAmountValidationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Invenza|Validation")
	static bool StartNetworkPreview();

	UFUNCTION(BlueprintCallable, Category = "Invenza|Validation")
	static bool PrepareNetworkProbe(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Validation")
	static FString GetNetworkSnapshot(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Validation")
	static bool SendClientWithdrawal(UWorld* World, int32 Amount, bool bStaleSnapshot = false);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Validation")
	static void ReleaseProbeReservation(UWorld* World);

	UFUNCTION(BlueprintPure, Category = "Invenza|Validation")
	static UWidget* FindPreviewWidget(UUserWidget* Owner, FName Name);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Validation")
	static UModalDialogBase* OpenProbeContextMenu(UWorld* World, bool bPlayerInventory = false);

	UFUNCTION(BlueprintPure, Category = "Invenza|Validation")
	static FString GetContextMenuReport(UModalDialogBase* Dialog);
};
