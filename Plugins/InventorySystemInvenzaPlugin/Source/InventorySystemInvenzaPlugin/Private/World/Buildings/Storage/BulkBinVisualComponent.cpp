//  Nublin Studio 2026 All Rights Reserved.

#include "World/Buildings/Storage/BulkBinVisualComponent.h"
#include "World/Buildings/Storage/BulkBinLayout.h"
#include "Data/ItemDataStructures.h"

UBulkBinVisualComponent::UBulkBinVisualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetMobility(EComponentMobility::Movable);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetCanEverAffectNavigation(false);
	// Stable prefix removal is required for quantity decreases.
	bSupportRemoveAtSwap = false;
}

bool UBulkBinVisualComponent::UpdateBin(const FItemStorageData& Storage, const FBox& OccupiedBounds,
										const FQuat& Rotation, int32 Quantity, int32 Capacity, UStaticMesh* WallMesh,
										UMaterialInterface* WallMaterial, float WallHeight, float WallThickness,
										float Inset)
{
	if (!IsValid(Storage.StorageMesh) || !IsValid(WallMesh) || !OccupiedBounds.IsValid || Quantity <= 0 ||
		Capacity <= 0)
	{
		return false;
	}
	const FBox WallBounds = WallMesh->GetBoundingBox();
	if (!WallBounds.IsValid || WallBounds.GetSize().GetMin() <= UE_SMALL_NUMBER)
	{
		return false;
	}
	const FVector AreaSize = Rotation.UnrotateVector(OccupiedBounds.GetSize()).GetAbs();
	const double SafeInset = FMath::Clamp(static_cast<double>(Inset), 0.0, FMath::Min(AreaSize.X, AreaSize.Y) * 0.1);
	const double Width = AreaSize.X - 2 * SafeInset;
	const double Depth = AreaSize.Y - 2 * SafeInset;
	const double Thickness = FMath::Clamp(static_cast<double>(WallThickness), 0.1, FMath::Min(Width, Depth) * 0.2);
	const double Height = FMath::Max(1.0f, Storage.BulkFillHeight);
	const double BorderHeight = FMath::Max(1.0f, WallHeight);
	const FVector Dimensions(Width - 2 * Thickness, Depth - 2 * Thickness, Height);
	const bool bMeshChanged = GetStaticMesh() != Storage.StorageMesh;
	if (bMeshChanged || !CachedDimensions.Equals(Dimensions) || CachedChunkSize != Storage.BulkChunkSize)
	{
		Layout = FBulkBinLayout::Build(FVector2D(Dimensions.X, Dimensions.Y), Height, Storage.BulkChunkSize,
									   Storage.StorageMesh->GetBoundingBox());
		ClearInstances();
		SetStaticMesh(Storage.StorageMesh);
		CachedDimensions = Dimensions;
		CachedChunkSize = Storage.BulkChunkSize;
	}
	if (Layout.IsEmpty())
	{
		return false;
	}
	for (int32 Index = 0; Index < GetNumMaterials(); ++Index)
	{
		SetMaterial(Index, Storage.BulkMaterial);
	}
	const FVector Center = OccupiedBounds.GetCenter();
	const FTransform Placement(Rotation, FVector(Center.X, Center.Y, OccupiedBounds.Max.Z));
	if (!GetRelativeTransform().Equals(Placement))
	{
		SetRelativeTransform(Placement);
	}
	const int32 Count = FBulkBinLayout::VisibleCount(Quantity, Capacity, Layout.Num());
	while (GetInstanceCount() > Count)
	{
		RemoveInstance(GetInstanceCount() - 1);
	}
	while (GetInstanceCount() < Count)
	{
		AddInstance(Layout[GetInstanceCount()], false);
	}

	if (!IsValid(Walls))
	{
		Walls = NewObject<UInstancedStaticMeshComponent>(GetOwner(), NAME_None, RF_Transient | RF_DuplicateTransient);
		Walls->SetMobility(EComponentMobility::Movable);
		Walls->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Walls->SetGenerateOverlapEvents(false);
		Walls->SetCanEverAffectNavigation(false);
		Walls->SetupAttachment(this);
	}
	Walls->SetStaticMesh(WallMesh);
	for (int32 Index = 0; Index < Walls->GetNumMaterials(); ++Index)
	{
		Walls->SetMaterial(Index, WallMaterial);
	}
	const FVector WallSizes[] = {FVector(Width, Thickness, BorderHeight), FVector(Width, Thickness, BorderHeight),
								 FVector(Thickness, Depth - 2 * Thickness, BorderHeight),
								 FVector(Thickness, Depth - 2 * Thickness, BorderHeight)};
	const FVector Centers[] = {FVector(0, -(Depth - Thickness) * .5, BorderHeight * .5),
							   FVector(0, (Depth - Thickness) * .5, BorderHeight * .5),
							   FVector(-(Width - Thickness) * .5, 0, BorderHeight * .5),
							   FVector((Width - Thickness) * .5, 0, BorderHeight * .5)};
	for (int32 Index = 0; Index < 4; ++Index)
	{
		const FVector Scale = WallSizes[Index] / WallBounds.GetSize();
		const FTransform Transform(FQuat::Identity, Centers[Index] - WallBounds.GetCenter() * Scale, Scale);
		if (Walls->GetInstanceCount() <= Index)
		{
			Walls->AddInstance(Transform, false);
		}
		else
		{
			Walls->UpdateInstanceTransform(Index, Transform, false, Index == 3, true);
		}
	}
	if (!Walls->IsRegistered())
	{
		Walls->RegisterComponent();
	}
	GridBounds =
		FBox(FVector(-Width * .5, -Depth * .5, 0), FVector(Width * .5, Depth * .5, FMath::Max(Height, BorderHeight)))
			.TransformBy(Placement);
	return true;
}

void UBulkBinVisualComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (IsValid(Walls))
	{
		Walls->DestroyComponent();
	}
	Walls = nullptr;
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}
