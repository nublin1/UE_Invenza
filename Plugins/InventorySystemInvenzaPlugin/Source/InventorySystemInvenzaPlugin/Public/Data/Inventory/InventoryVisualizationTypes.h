// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryVisualizationTypes.generated.h"

UENUM(BlueprintType)
enum class EVisualizerMode : uint8
{
	IndividualItems UMETA(DisplayName = "Spawn Each Item"),
	OccupancyMeshSwap UMETA(DisplayName = "Swap Mesh by Percentage")
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FCachedSocketData
{
	GENERATED_BODY()

	UPROPERTY()
	FName SocketName;

	UPROPERTY()
	FTransform RelativeTransform;
};
