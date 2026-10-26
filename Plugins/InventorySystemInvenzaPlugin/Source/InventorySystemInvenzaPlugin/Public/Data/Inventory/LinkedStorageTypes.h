//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LinkedStorageTypes.generated.h"

UENUM(BlueprintType)
enum class ELinkedStorageAccessMode : uint8
{
	Shared,
	Personal
};
