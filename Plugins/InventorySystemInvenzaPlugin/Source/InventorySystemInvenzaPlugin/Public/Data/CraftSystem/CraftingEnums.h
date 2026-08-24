// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CraftingEnums.generated.h"

UENUM(BlueprintType)
enum class ECraftingResourceConsumePolicy : uint8
{
	OnQueueAdd      UMETA(DisplayName="Consume On Queue Add"),
	OnCraftStart    UMETA(DisplayName="Consume On Craft Start"),
	OnCraftFinish   UMETA(DisplayName="Consume On Craft Finish")
};