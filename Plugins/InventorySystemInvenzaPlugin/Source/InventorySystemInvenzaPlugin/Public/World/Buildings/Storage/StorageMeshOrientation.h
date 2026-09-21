#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStructures.h"
class FStorageMeshOrientation
{
public:
	static FQuat ToLengthAlongX(EStorageMeshAxis Axis)
	{
		return FRotator(0.0, Axis == EStorageMeshAxis::Y ? -90.0 : 0.0, 0.0).Quaternion();
	}

	static FQuat LogsGridRotation(EItemOrientationType Orientation)
	{
		// Inventory UI stores (row, column) in (X, Y).
		return FRotator(0.0, Orientation == EItemOrientationType::Horizontal ? 90.0 : 0.0, 0.0).Quaternion();
	}
};