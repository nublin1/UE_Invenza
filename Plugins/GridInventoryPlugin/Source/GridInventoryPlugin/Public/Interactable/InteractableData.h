// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableData.generated.h"

/**
 * 
 */

class UInteractionComponent;

UENUM()
enum class EInteractableType: uint8 
{
	Pickup		UMETA(DisplayName = "Pickup"),
	NPC			UMETA(DisplayName = "NPC"),
	Toggle		UMETA(DisplayName = "Toggle"),
	Container	UMETA(DisplayName = "Container"),
	InfoOnly    UMETA(DisplayName = "Informational") 
};

USTRUCT(Blueprintable, BlueprintType)
struct FInteractableData
{
	GENERATED_USTRUCT_BODY()

	FInteractableData() :
	DefaultInteractableType(EInteractableType::Pickup),
	Name(FText::GetEmpty()),
	Action(FText::GetEmpty()),
	InteractableDuration(0.0f)
	{

	};

	UPROPERTY(EditDefaultsOnly)
	EInteractableType DefaultInteractableType;
	UPROPERTY(EditDefaultsOnly)
	FText Name;	
	UPROPERTY(EditDefaultsOnly)
	FText Action;	
	UPROPERTY(EditDefaultsOnly)
	float InteractableDuration;	
};

