// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Interactable/InteractableData.h"
#include "InteractionData.generated.h"

class UInteractableComponent;

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInteractionData
{
	GENERATED_USTRUCT_BODY()

	FInteractionData() : CurrentInteractable(nullptr), LastInteractable(nullptr), LastInteractionCheckTime(0.0f) {};

	UPROPERTY()
	TObjectPtr<UInteractableComponent> CurrentInteractable;
	UPROPERTY()
	TObjectPtr<UInteractableComponent> LastInteractable;

	UPROPERTY()
	float LastInteractionCheckTime;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInteractionDisplayEntry
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Interaction", BlueprintReadOnly)
	FText KeyLabel;

	UPROPERTY(Category = "Invenza|Interaction", BlueprintReadOnly)
	FInteractableData Data;
};
