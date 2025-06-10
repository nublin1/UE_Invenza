// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemDataStructures.h"
#include "EquipmentStructures.generated.h"

class UItemBase;

USTRUCT(BlueprintType)
struct FEquipmentSlot
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Equipment")
	FName SlotName;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Equipment")
	EItemCategory AllowedCategory = EItemCategory::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Equipment")
	UItemBase* EquippedItem = nullptr;
};
