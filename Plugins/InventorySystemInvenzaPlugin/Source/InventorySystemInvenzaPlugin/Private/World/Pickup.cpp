//  Nublin Studio 2025 All Rights Reserved.

#include "World/Pickup.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Data/ItemData.h"

// Sets default values
APickup::APickup()
{
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
	PickupMesh->SetSimulatePhysics(true);
	SetRootComponent(PickupMesh);

	BoxCollider = CreateDefaultSubobject<UBoxComponent>("BoxCollider");	
	BoxCollider->SetHiddenInGame(false);
	BoxCollider->AttachToComponent(PickupMesh, FAttachmentTransformRules::KeepRelativeTransform);
}

void APickup::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

