#pragma once


#include "CoreMinimal.h"
#include "ItemDataStructures.h"
#include "EquipmentStructures.generated.h"

class UItemBase;

USTRUCT(BlueprintType)
struct FEquipmentSlot
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	FName SlotName;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	EItemCategory AllowedCategory = EItemCategory::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	UItemBase* EquippedItem = nullptr;
};
