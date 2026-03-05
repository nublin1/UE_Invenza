// Nublin Studio 2026 All Rights Reserved.

#include "World/Buildings/Storage/StorageVisualRepresentation.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Data/ItemDataStructures.h"
#include "Data/Items/itemBase.h"

AStorageVisualRepresentation::AStorageVisualRepresentation()
{
	StaticMeshVisual = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("VisualRepresentation"));
	StaticMeshVisual->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void AStorageVisualRepresentation::BeginPlay()
{
	Super::BeginPlay();
}

void AStorageVisualRepresentation::UpdateVisual()
{
	if (!ItemBase)
		return;
	
	FTransform InstanceTransform(FRotator::ZeroRotator, FVector(0.0f), FVector(1.f, 1.f, 1.f));

	StaticMeshVisual->SetStaticMesh(ItemBase->GetItemRef().ItemAssetData.MeshAsStorage);

	switch (ItemBase->GetItemRef().StorageMethod)
	{
	case EStorageMethod::Single:
		
		StaticMeshVisual->AddInstance(InstanceTransform, false);
		break;
	default:
		break;
	}
}
