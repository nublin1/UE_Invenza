//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"

/** Deterministic, bounded layout. Quantity selects a prefix; existing chunks never move. */
struct FBulkBinLayout
{
	static constexpr int32 MaxChunks = 512;

	static int32 VisibleCount(int32 Quantity, int32 Capacity, int32 LayoutCount)
	{
		if (Quantity <= 0 || Capacity <= 0 || LayoutCount <= 0)
		{
			return 0;
		}

		return FMath::Clamp(FMath::CeilToInt(static_cast<double>(Quantity) / Capacity * LayoutCount), 1, LayoutCount);
	}

	static TArray<FTransform> Build(const FVector2D& Interior, double Height, double ChunkSize, const FBox& MeshBounds)
	{
		TArray<FTransform> Result;
		if (!MeshBounds.IsValid || MeshBounds.GetSize().GetMin() <= UE_SMALL_NUMBER ||
			Interior.GetMin() <= UE_SMALL_NUMBER || Height <= 0 || ChunkSize <= 0)
		{
			return Result;
		}

		const double Diameter = FMath::Min3(ChunkSize, Interior.X, Interior.Y);
		int32 Columns = FMath::Clamp(FMath::FloorToInt(Interior.X / Diameter), 1, 32);
		int32 Rows = FMath::Clamp(FMath::FloorToInt(Interior.Y / Diameter), 1, 32);
		int32 Layers = FMath::Clamp(FMath::CeilToInt(Height / (Diameter * 0.8)), 1, 16);

		while (Columns * Rows * Layers > MaxChunks)
		{
			if (Columns >= Rows && Columns >= Layers)
			{
				--Columns;
			}
			else if (Rows >= Layers)
			{
				--Rows;
			}
			else
			{
				--Layers;
			}
		}

		const double StepX = Interior.X / Columns;
		const double StepY = Interior.Y / Rows;
		const double StepZ = Height / Layers;
		const double ChunkHeight = FMath::Min(Height, StepZ * 1.35);
		TArray<TPair<FTransform, double>> Chunks;
		FRandomStream Random(71839);
		for (int32 Layer = 0; Layer < Layers; ++Layer)
		{
			for (int32 Y = 0; Y < Rows; ++Y)
			{
				for (int32 X = 0; X < Columns; ++X)
				{
					const double NX = (X + 0.5) / Columns * 2.0 - 1.0;
					const double NY = (Y + 0.5) / Rows * 2.0 - 1.0;
					const double Radius = FMath::Sqrt(NX * NX + NY * NY) / FMath::Sqrt(2.0);
					// Lower layers fill the section; upper layers taper toward its center.
					const int32 ColumnHeight = FMath::Max(1, FMath::CeilToInt(Layers * (1.0 - 0.45 * Radius)));
					if (Layer >= ColumnHeight)
					{
						continue;
					}
					const FQuat Rotation =
						FRotator(Random.FRandRange(-12, 12), Random.FRandRange(0, 360), Random.FRandRange(-12, 12))
							.Quaternion();
					const FBox Rotated = MeshBounds.TransformBy(FTransform(Rotation));
					const FVector Size = Rotated.GetSize();
					const FVector Center((X + 0.5 + Random.FRandRange(-0.12f, 0.12f)) * StepX - Interior.X * 0.5,
										 (Y + 0.5 + Random.FRandRange(-0.12f, 0.12f)) * StepY - Interior.Y * 0.5, 0);
					// Interlock neighboring chunks, but keep their entire bounds inside the bin.
					const double FitX = FMath::Min(StepX * 1.3, Interior.X - 2 * FMath::Abs(Center.X));
					const double FitY = FMath::Min(StepY * 1.3, Interior.Y - 2 * FMath::Abs(Center.Y));
					const double Scale = FMath::Min3(FitX / Size.X, FitY / Size.Y, ChunkHeight / Size.Z) *
										 Random.FRandRange(0.93f, 1.0f);
					// Center arbitrary pivots and keep the bottom layer above the platform.
					const double Bottom = Layer * (Height - ChunkHeight) / FMath::Max(1, Layers - 1);
					const FVector Offset(Center.X - Rotated.GetCenter().X * Scale,
										 Center.Y - Rotated.GetCenter().Y * Scale,
										 FMath::Max(0.0, Bottom) - Rotated.Min.Z * Scale);
					Chunks.Add({FTransform(Rotation, Offset, FVector(Scale)), Layer * 10.0 + Radius});
				}
			}
		}

		Chunks.StableSort(
			[](const TPair<FTransform, double>& A, const TPair<FTransform, double>& B)
			{
				return A.Value < B.Value;
			});

		for (const TPair<FTransform, double>& Chunk : Chunks)
		{
			Result.Add(Chunk.Key);
		}

		return Result;
	}
};
