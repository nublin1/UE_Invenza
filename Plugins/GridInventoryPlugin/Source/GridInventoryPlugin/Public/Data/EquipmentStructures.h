#pragma once


#include "CoreMinimal.h"
#include "ItemDataStructures.h"
#include "EquipmentStructures.generated.h"

class UItemBase;

USTRUCT(BlueprintType)
struct FEquipmentSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemCategory AllowedCategory = EItemCategory::None;

	UPROPERTY(BlueprintReadWrite)
	UItemBase* EquippedItem = nullptr;
};
