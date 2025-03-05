#pragma once

#include "CoreMinimal.h"
#include "InteractionData.generated.h"

USTRUCT(BlueprintType)
struct FInteractionData
{
	GENERATED_USTRUCT_BODY()

	FInteractionData() : CurrentInteractable(nullptr), LastInteractable(nullptr),LastInteractionCheckTime(0.0f){};

	UPROPERTY()
	TObjectPtr<AActor> CurrentInteractable;
	UPROPERTY()
	TObjectPtr<AActor> LastInteractable;

	UPROPERTY()
	float LastInteractionCheckTime;
};