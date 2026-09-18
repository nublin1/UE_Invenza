// Nublin Studio 2026 All Rights Reserved.


#include "World/Buildings/Storage/StorageGridVisualizer.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"

UStorageGridVisualizer::UStorageGridVisualizer()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = 0.5f;

	USceneComponent::SetMobility(EComponentMobility::Movable);

	UPrimitiveComponent::SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetCanEverAffectNavigation(false);
}

void UStorageGridVisualizer::OnRegister()
{
	Super::OnRegister(); 
	
	// Enforce visualization-only behavior, including editor instances.
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetCanEverAffectNavigation(false);

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	SetComponentTickEnabled(World->IsGameWorld() && World->GetNetMode() != NM_DedicatedServer);

	RebuildGrid();
}

void UStorageGridVisualizer::OnUnregister()
{
	ClearGrid();
	TargetInventory = nullptr;

	Super::OnUnregister();
}

#if WITH_EDITOR
void UStorageGridVisualizer::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (IsValid(this) && IsRegistered())
	{
		RebuildGrid();
	}
}
#endif

void UStorageGridVisualizer::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	const UWorld* World = GetWorld();

	if (!World
		|| !World->IsGameWorld()
		|| World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	SynchronizeInventory();
}

USlotbasedInventory* UStorageGridVisualizer::FindTargetInventory() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	UItemCollection* Collection = Owner->FindComponentByClass<UItemCollection>();
	if (!IsValid(Collection))
	{
		return nullptr;
	}

	for (UInventoryBase* Inventory : Collection->GetActorInventories())
	{
		USlotbasedInventory* GridInventory = Cast<USlotbasedInventory>(Inventory);
		if (!IsValid(GridInventory))
		{
			continue;
		}

		if (!InventorySearchTag.IsValid() || GridInventory->GetInventorySettings().InventoryTag.MatchesTag(InventorySearchTag))
		{
			return GridInventory;
		}
	}

	return nullptr;
}

void UStorageGridVisualizer::SynchronizeInventory()
{
	USlotbasedInventory* FoundInventory = FindTargetInventory();
	const FIntPoint DesiredSize = FoundInventory ? FoundInventory->GetInventorySize(): FIntPoint::ZeroValue;

	if (TargetInventory != FoundInventory || BuiltGridSize != DesiredSize)
	{
		TargetInventory = FoundInventory;
		BuildGrid(DesiredSize);
	}
}

void UStorageGridVisualizer::RebuildGrid()
{
	UWorld* World = GetWorld();
	if (!World || IsTemplate())
	{
		return;
	}

	if (World->IsGameWorld())
	{
		if (World->GetNetMode() == NM_DedicatedServer)
		{
			ClearGrid();
			TargetInventory = nullptr;
			return;
		}

		TargetInventory = FindTargetInventory();
		BuildGrid(TargetInventory ? TargetInventory->GetInventorySize()	: FIntPoint::ZeroValue);

		return;
	}

	TargetInventory = nullptr;
	if (bPreviewInEditor)
	{
		BuildGrid(PreviewGridSize);
	}
	else
	{
		ClearGrid();
	}
}

void UStorageGridVisualizer::ClearGrid()
{
	// Invalidate external references to cells from the previous generation.
	for (UGridStorageCell* Cell : Cells)
	{
		if (Cell)
		{
			Cell->Visualizer.Reset();
			Cell->InstanceIndex = INDEX_NONE;
		}
	}

	Cells.Reset();
	ClearInstances();

	BuiltGridSize = FIntPoint::ZeroValue;
}

void UStorageGridVisualizer::BuildGrid(FIntPoint GridSize)
{
	ClearGrid();
	if (GridSize.X <= 0 || GridSize.Y <= 0)
	{
		return;
	}

	UStaticMesh* Mesh = GetStaticMesh();
	if (!Mesh)
	{
		return;
	}

	const int64 CellCount =	static_cast<int64>(GridSize.X) * GridSize.Y;
	if (CellCount > FMath::Max(1, MaxCellCount))
	{
		return;
	}

	if (CellScale.X <= 0.0 || CellScale.Y <= 0.0 || CellScale.Z <= 0.0)
	{
		return;
	}

	const FBox MeshBounds = Mesh->GetBoundingBox();
	if (!MeshBounds.IsValid)
	{
		return;
	}

	const FVector ScaledSize = MeshBounds.GetSize() * CellScale;
	if (ScaledSize.X <= UE_SMALL_NUMBER || ScaledSize.Y <= UE_SMALL_NUMBER)
	{
		return;
	}

	const FVector2D Step(
		ScaledSize.X + FMath::Max(0.0, CellGap.X),
		ScaledSize.Y + FMath::Max(0.0, CellGap.Y));

	// Place the minimum mesh bounds at the cell corner.
	// This compensates for a centered or offset mesh pivot.
	// The bottom of every mesh lies at local Z = 0.
	const FVector PivotCorrection = -(MeshBounds.Min * CellScale);

	Cells.Reserve(static_cast<int32>(CellCount));

	for (int32 Y = 0; Y < GridSize.Y; ++Y)
	{
		for (int32 X = 0; X < GridSize.X; ++X)
		{
			const FVector CellCorner(X * Step.X, Y * Step.Y,0.0);
			const FVector Translation = CellCorner + PivotCorrection;

			const FTransform InstanceTransform(FQuat::Identity, Translation, CellScale);
			const int32 Index = AddInstance(InstanceTransform,false); // Component-local space.

			UGridStorageCell* Cell = NewObject<UGridStorageCell>(this, NAME_None,RF_Transient);

			Cell->Visualizer = this;
			Cell->Coordinates = FIntPoint(X, Y);
			Cell->InstanceIndex = Index;
			Cell->LocalTransform = InstanceTransform;
			Cell->LocalCenter =	Translation + MeshBounds.GetCenter() * CellScale;

			Cells.Add(Cell);
		}
	}

	BuiltGridSize = GridSize;
}

UGridStorageCell* UStorageGridVisualizer::GetCell(FIntPoint Coordinates) const
{
	if (Coordinates.X < 0
		|| Coordinates.Y < 0
		|| Coordinates.X >= BuiltGridSize.X
		|| Coordinates.Y >= BuiltGridSize.Y)
	{
		return nullptr;
	}

	const int32 Index =	Coordinates.Y * BuiltGridSize.X + Coordinates.X;

	return Cells.IsValidIndex(Index) ? Cells[Index].Get() : nullptr;
}