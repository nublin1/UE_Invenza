// Nublin Studio 2026 All Rights Reserved.


#include "World/Buildings/Storage/GridStorageCell.h"

#include "World/Buildings/Storage/StorageGridVisualizer.h"

UGridStorageCell::UGridStorageCell()
{
}

bool UGridStorageCell::IsCellValid() const
{
	const UStorageGridVisualizer* Owner = Visualizer.Get();

	return Owner
		&& InstanceIndex != INDEX_NONE
		&& Owner->GetCell(Coordinates) == this;
}

FTransform UGridStorageCell::GetWorldTransform() const
{
	const UStorageGridVisualizer* Owner = Visualizer.Get();

	return Owner
		? LocalTransform * Owner->GetComponentTransform()
		: FTransform::Identity;
}

FVector UGridStorageCell::GetWorldCenter() const
{
	const UStorageGridVisualizer* Owner = Visualizer.Get();
	return Owner
		? Owner->GetComponentTransform().TransformPosition(LocalCenter)
		: FVector::ZeroVector;
}