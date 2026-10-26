//  Nublin Studio 2026 All Rights Reserved.

#include "World/StorageAccessPoint.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "ActorComponents/Interactable/StorageAccessComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

AStorageAccessPoint::AStorageAccessPoint()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));

	if (UStaticMesh* DefaultMesh =
			GetDefault<UInvenzaInventoryDeveloperSettings>()->DefaultAccessPointMesh.LoadSynchronous())
	{
		Mesh->SetStaticMesh(DefaultMesh);
	}

	StorageAccess = CreateDefaultSubobject<UStorageAccessComponent>(TEXT("StorageAccess"));
}
