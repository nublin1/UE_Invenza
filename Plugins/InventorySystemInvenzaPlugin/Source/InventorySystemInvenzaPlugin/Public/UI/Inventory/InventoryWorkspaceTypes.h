//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryWorkspaceTypes.generated.h"

/** Presentation options for inventories embedded in a workspace. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventoryWorkspaceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout", meta = (ClampMin = "0"))
	float PlayerColumnMinWidth = 330.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	bool bFitPlayerColumnToContent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventories")
	bool bAllowInventoryCollapse = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventories")
	bool bHideInventoryCloseButtons = false;
};
