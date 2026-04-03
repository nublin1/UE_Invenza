// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStructures.h"
#include "EquipmentSlotDefinition.generated.h"

USTRUCT(BlueprintType)
struct FEquipmentSlotRuntime
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag SlotTag;

	UPROPERTY()
	EItemCategory AllowedCategory;

	UPROPERTY()
	TObjectPtr<UItemBase> EquippedItem = nullptr;
};

USTRUCT(BlueprintType)
struct FEquipmentSlotDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag SlotTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemCategory AllowedCategory = EItemCategory::All;
};