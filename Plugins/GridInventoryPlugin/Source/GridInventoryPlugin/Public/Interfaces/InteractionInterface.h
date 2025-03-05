// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionInterface.generated.h"

/**
 * 
 */

class UInteractionComponent;

UENUM()
enum class EInteractableType: uint8 
{
	Pickup UMETA(DisplayName = "Pickup"),
	NPC UMETA(DisplayName = "NPC"),
	Device UMETA(DisplayName = "Device"),
	Toggle UMETA(DisplayName = "Toggle"),
	Container UMETA(DisplayName = "Container")
};

USTRUCT(Blueprintable, BlueprintType)
struct FInteractableData
{
	GENERATED_USTRUCT_BODY()

	FInteractableData() :
	DefaultInteractableType(EInteractableType::Pickup),
	Name(FText::GetEmpty()),
	Action(FText::GetEmpty()),
	Quantity(0),
	InteractableDuration(0.0f)
	{

	};

	UPROPERTY(EditDefaultsOnly)
	EInteractableType DefaultInteractableType;
	UPROPERTY(EditInstanceOnly)
	FText Name;	
	UPROPERTY(EditInstanceOnly)
	FText Action;
	UPROPERTY(EditInstanceOnly)
	int32 Quantity;
	UPROPERTY(EditInstanceOnly)
	float InteractableDuration;	
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

class GRIDINVENTORYPLUGIN_API IInteractionInterface
{
	GENERATED_BODY()
	
public:
	virtual void BeginFocus();
	virtual void EndFocus();
	virtual void BeginInteract(UInteractionComponent* InteractionComponent);
	virtual void EndInteract(UInteractionComponent* InteractionComponent);
	virtual void Interact(UInteractionComponent* InteractionComponent);
	
};
