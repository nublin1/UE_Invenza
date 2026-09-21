#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"

class FSmallBoxStackLayout
{
public:
	static constexpr int32 Capacity = 78;
	inline static const FName ComponentTag = FName(TEXT("StorageGrid.SmallBoxes"));

	static FTransform GetTransform(int32 Index, const FBox& MeshBounds, const FVector2D& Gap = FVector2D::ZeroVector)
	{
		check(Index >= 0 && Index < Capacity);
		const FVector Size = MeshBounds.GetSize();
		const FVector Center = MeshBounds.GetCenter();
		const FVector2D Step(Size.X + FMath::Max(0.0, Gap.X), Size.Y + FMath::Max(0.0, Gap.Y));
		int32 Layer = 0;
		int32 Width = 4;
		while (Index >= Width * Width)
		{
			Index -= Width * Width;
			++Layer;
			Width = Layer < 4 ? 4 : 7 - Layer;
		}
		return FTransform(FQuat::Identity, FVector(
		   (Index % Width - (Width - 1) * 0.5) * Step.X - Center.X,
		   (Index / Width - (Width - 1) * 0.5) * Step.Y - Center.Y,
		   Layer * Size.Z - MeshBounds.Min.Z));
	}

	// Every quantity uses a prefix of the same layout, so existing boxes never move.
	static void SetQuantity(UInstancedStaticMeshComponent& Stack, int32 Quantity, const FBox& MeshBounds, const FVector2D& Gap = FVector2D::ZeroVector)
	{
		const int32 Count = FMath::Clamp(Quantity, 0, Capacity);
		for (int32 Index = Stack.GetInstanceCount() - 1; Index >= Count; --Index)
		{
			Stack.RemoveInstance(Index);
		}
		for (int32 Index = Stack.GetInstanceCount(); Index < Count; ++Index)
		{
			Stack.AddInstance(GetTransform(Index, MeshBounds, Gap), false);
		}
	}
};