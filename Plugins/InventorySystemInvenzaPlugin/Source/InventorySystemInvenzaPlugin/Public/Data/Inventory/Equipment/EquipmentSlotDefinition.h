// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStructures.h"
#include "EquipmentSlotDefinition.generated.h"

USTRUCT(BlueprintType)
struct FEquipmentSlotRuntime
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FGameplayTag SlotTag;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	EItemCategory AllowedCategory;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TObjectPtr<UItemBase> EquippedItem = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FName AttachSocket;
};

USTRUCT(BlueprintType)
struct FEquipmentSlotDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag SlotTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemCategory AllowedCategory = EItemCategory::All;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AttachSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTransform RelativeTransform;
};