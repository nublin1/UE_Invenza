#pragma once

#include "CoreMinimal.h"
#include "InteractionData.generated.h"

class UInteractableComponent;

USTRUCT(BlueprintType)
struct FInteractionData
{
	GENERATED_USTRUCT_BODY()

	FInteractionData() : CurrentInteractable(nullptr), LastInteractable(nullptr),LastInteractionCheckTime(0.0f){};
	
	UPROPERTY()
	TObjectPtr<UInteractableComponent> CurrentInteractable;
	UPROPERTY()
	TObjectPtr<UInteractableComponent> LastInteractable;

	UPROPERTY()
	float LastInteractionCheckTime;
};