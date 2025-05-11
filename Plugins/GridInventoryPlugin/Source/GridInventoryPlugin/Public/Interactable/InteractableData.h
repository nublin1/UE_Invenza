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
	None		UMETA(DisplayName = "None"),
	Pickup		UMETA(DisplayName = "Pickup"),
	NPC			UMETA(DisplayName = "NPC"),
	Toggle		UMETA(DisplayName = "Toggle"),
	Container	UMETA(DisplayName = "Container"),
	InfoOnly    UMETA(DisplayName = "Informational"),
	Vendor		UMETA(DisplayName = "Vending machine"),
};

USTRUCT(Blueprintable, BlueprintType)
struct FInteractableData
{
	GENERATED_USTRUCT_BODY()

	FInteractableData() :
		DefaultInteractableType(EInteractableType::Pickup),
		Name(FText::GetEmpty()),
		Action(FText::GetEmpty()),
		InteractableDuration(0.0f), Quantity(0)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	EInteractableType DefaultInteractableType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	bool bHoldToInteract = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	FText Name;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	FText Action;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	float InteractableDuration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	int32 Quantity;
};

