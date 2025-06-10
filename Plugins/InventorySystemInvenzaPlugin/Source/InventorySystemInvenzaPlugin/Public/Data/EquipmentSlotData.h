// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemDataStructures.h"
#include "EquipmentSlotData.generated.h"

USTRUCT(BlueprintType)
struct FEquipmentSlotData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FName SlotName = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	EItemCategory AllowedCategory = EItemCategory::None;
};
