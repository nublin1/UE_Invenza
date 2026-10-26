//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CraftProgressTypes.generated.h"

/** With both options disabled, shows completed / required work. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FCraftProgressDisplaySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bUseProgressBar = true;

	/** Shows a percentage beside the bar, or on its own when the bar is disabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bShowPercentage = true;
};
