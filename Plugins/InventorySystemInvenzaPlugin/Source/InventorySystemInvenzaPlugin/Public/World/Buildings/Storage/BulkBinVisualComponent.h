//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "BulkBinVisualComponent.generated.h"

struct FItemStorageData;

/** One transient stack visual, owning its border. All transforms are grid-local. */
UCLASS(Transient)
class INVENTORYSYSTEMINVENZAPLUGIN_API UBulkBinVisualComponent : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:
	UBulkBinVisualComponent();

protected:
	// Unreal overrides
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

public:
	// Functions
	bool UpdateBin(const FItemStorageData& Storage, const FBox& OccupiedBounds, const FQuat& Rotation, int32 Quantity,
				   int32 Capacity, UStaticMesh* WallMesh, UMaterialInterface* WallMaterial, float WallHeight,
				   float WallThickness, float Inset);

	FBox GetBinBounds() const { return GridBounds; }

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> Walls;

	TArray<FTransform> Layout;

	FVector CachedDimensions = FVector::ZeroVector;

	float CachedChunkSize = 0;

	// Functions
	FBox GridBounds = FBox(ForceInit);
};
