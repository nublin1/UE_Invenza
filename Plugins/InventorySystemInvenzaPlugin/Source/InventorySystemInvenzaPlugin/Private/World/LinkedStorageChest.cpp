//  Nublin Studio 2026 All Rights Reserved.

#include "World/LinkedStorageChest.h"
#include "ActorComponents/Interactable/LinkedStorageComponent.h"
#include "Components/StaticMeshComponent.h"

ALinkedStorageChest::ALinkedStorageChest()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));
	LinkedStorage = CreateDefaultSubobject<ULinkedStorageComponent>(TEXT("LinkedStorage"));
}
