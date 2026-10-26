//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"

#include "Pickup.generated.h"

class UPickupComponent;
class UBoxComponent;

UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API APickup : public AActor
{
	GENERATED_BODY()

public:
	APickup();

protected:
	// Unreal overrides
	virtual void PostInitializeComponents() override;

protected:
	// Runtime
	// Properties and variables
	UPROPERTY()
	UStaticMeshComponent* PickupMesh;

	UPROPERTY()
	UBoxComponent* BoxCollider;
};
