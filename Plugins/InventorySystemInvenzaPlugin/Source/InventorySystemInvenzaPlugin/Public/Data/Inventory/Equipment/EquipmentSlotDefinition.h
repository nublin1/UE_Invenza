// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStructures.h"
#include "EquipmentSlotDefinition.generated.h"

USTRUCT(BlueprintType)
struct FEquipmentSlotData
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag EquipmentSlotTag;
	UPROPERTY()
	EItemCategory AllowedCategory = EItemCategory::All;
	UPROPERTY()
	TObjectPtr<UItemBase> EquippedItem = nullptr;
};

USTRUCT(BlueprintType)
struct FEquipmentSlotDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FGameplayTag EquipmentSlotTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	EItemCategory AllowedCategory = EItemCategory::All;
};