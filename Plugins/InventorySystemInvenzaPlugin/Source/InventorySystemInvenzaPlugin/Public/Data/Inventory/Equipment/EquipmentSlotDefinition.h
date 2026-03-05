// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStructures.h"
#include "EquipmentSlotDefinition.generated.h"

USTRUCT(BlueprintType)
struct FEquipmentSlotDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FName SlotName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	EItemCategory AllowedCategory = EItemCategory::None;
};