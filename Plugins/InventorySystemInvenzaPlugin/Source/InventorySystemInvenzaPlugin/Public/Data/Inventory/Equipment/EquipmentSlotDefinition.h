// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStructures.h"
#include "EquipmentSlotDefinition.generated.h"

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FEquipmentSlotRuntime
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Equipment", VisibleInstanceOnly, BlueprintReadOnly)
	FGameplayTag SlotTag;

	UPROPERTY(Category = "Invenza|Equipment", VisibleInstanceOnly, BlueprintReadOnly)
	FGameplayTag AllowedCategory;

	UPROPERTY(Category = "Invenza|Equipment", VisibleInstanceOnly, BlueprintReadOnly)
	TObjectPtr<UObject> EquippedItem = nullptr;

	UPROPERTY(Category = "Invenza|Equipment", VisibleInstanceOnly, BlueprintReadOnly)
	FName AttachSocket;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FEquipmentSlotDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Equipment", EditAnywhere, BlueprintReadOnly)
	FGameplayTag SlotTag;

	UPROPERTY(Category = "Invenza|Equipment", EditAnywhere, BlueprintReadOnly)
	FGameplayTag AllowedCategory;

	UPROPERTY(Category = "Invenza|Equipment", EditAnywhere, BlueprintReadOnly)
	FName AttachSocket;

	UPROPERTY(Category = "Invenza|Equipment", EditAnywhere, BlueprintReadOnly)
	FTransform RelativeTransform;
};