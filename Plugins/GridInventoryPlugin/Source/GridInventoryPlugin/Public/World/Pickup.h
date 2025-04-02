// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Pickup.generated.h"

class UPickupComponent;
class UBoxComponent;

UCLASS()
class GRIDINVENTORYPLUGIN_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	APickup();


protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void PostInitializeComponents() override;

	UPROPERTY()
	UStaticMeshComponent* PickupMesh;
	UPROPERTY()
	UBoxComponent* BoxCollider;
};
