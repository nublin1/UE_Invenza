// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

enum class EInventoryGrantOverflow : uint8
{
	Retain,
	DropAndRetainOnFailure,
	DropAndDiscardOnFailure
};

/** Native grant policy; callers supply ordered, authorized destinations. */
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventoryGrantPolicy
{
	bool bAllowPartial = true;
	bool bLogOverflowFailure = true;
	EInventoryGrantOverflow Overflow = EInventoryGrantOverflow::Retain;
	FTransform DropTransform = FTransform::Identity;
};

struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventoryGrantResult
{
	int32 Added = 0;
	int32 Dropped = 0;
	int32 Discarded = 0;
	int32 Remaining = 0;
};
