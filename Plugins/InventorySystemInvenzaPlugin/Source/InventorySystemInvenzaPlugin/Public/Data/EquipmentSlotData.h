#pragma once

#include "CoreMinimal.h"
#include "ItemDataStructures.h"
#include "EquipmentSlotData.generated.h"

USTRUCT(BlueprintType)
struct FEquipmentSlotData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotName = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemCategory AllowedCategory = EItemCategory::None;
};
