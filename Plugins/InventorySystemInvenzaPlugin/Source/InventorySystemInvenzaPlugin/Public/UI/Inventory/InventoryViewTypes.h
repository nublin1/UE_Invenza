// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryViewTypes.generated.h"

/** Sorting affects the view only, never the source inventory. */
UENUM(BlueprintType)
enum class EInventoryViewSortMode : uint8
{
	SourceOrder,
	NameAscending,
	NameDescending,
	QuantityDescending
};
