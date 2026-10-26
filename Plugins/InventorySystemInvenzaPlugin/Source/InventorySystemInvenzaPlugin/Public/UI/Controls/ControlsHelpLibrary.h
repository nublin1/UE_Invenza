//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/Controls/ControlsHelpTypes.h"
#include "ControlsHelpLibrary.generated.h"

class APlayerController;
class UInputAction;
class UInvenzaInventorySettingsAsset;

UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UControlsHelpLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Functions
	UFUNCTION(BlueprintPure, Category = "Input|Help")
	static FText GetActionKeys(APlayerController* Player, const UInputAction* Action);

	UFUNCTION(BlueprintCallable, Category = "Input|Help")
	static TArray<FControlsHelpRow> BuildRows(APlayerController* Player,
											  const UInvenzaInventorySettingsAsset* Settings);
};
