// Nublin Studio 2026 All Rights Reserved.


#include "World/Buildings/Storage/StorageGridVisualizer.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Interface/Interaction/ObjectDataProvider.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "World/Buildings/Storage/SmallBoxStackLayout.h"
#include "World/Buildings/Storage/StorageMeshOrientation.h"

DEFINE_LOG_CATEGORY_STATIC(LogStorageGridVisualizer, Log, All);

namespace StorageGridPlacement
{
	// Work in grid-local space so the whole storage can be rotated or scaled.
	static FTransform MakeTransform(const FBox& CellBounds, const FBox& MeshBounds, const FQuat& Rotation)
	{
		const FBox RotatedBounds = MeshBounds.TransformBy(FTransform(Rotation));
		const FVector Center = CellBounds.GetCenter();
		const FVector MeshCenter = RotatedBounds.GetCenter();
		return FTransform(Rotation, FVector(
			Center.X - MeshCenter.X,
			Center.Y - MeshCenter.Y,
			CellBounds.Max.Z - RotatedBounds.Min.Z));
	}
}

UStorageGridVisualizer::UStorageGridVisualizer()
{
	PrimaryComponentTick.bCanEverTick = false;

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

	if (World->IsGameWorld() && World->GetNetMode() != NM_DedicatedServer)
	{
		ObserveInventorySource(GetOwner()->FindComponentByClass<UItemCollection>());
	}

	RebuildGrid();
}

void UStorageGridVisualizer::OnUnregister()
{
	ObserveInventorySource(nullptr);
	ObserveCollection(nullptr);
	BindInventory(nullptr);
	WarnedMissingMeshes.Empty();
	ClearGrid();
	TargetInventory = nullptr;

	Super::OnUnregister();
}

void UStorageGridVisualizer::BeginPlay()
{
	Super::BeginPlay();
	if (GetNetMode() == NM_DedicatedServer) return;
	// Blueprint construction may create ItemCollection after this component's OnRegister.
	ObserveInventorySource(GetOwner()->FindComponentByClass<UItemCollection>());
	SynchronizeInventory();
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

void UStorageGridVisualizer::UpdateInteractionBounds()
{
	AActor* Owner = GetOwner();
	const UWorld* World = GetWorld();
	UStaticMesh* PlatformMesh = GetStaticMesh();

	if (!bEnableInteractionBounds
		|| !IsRegistered()
		|| IsTemplate()
		|| !IsValid(Owner)
		|| Owner->IsTemplate()
		|| !World
		|| World->GetNetMode() == NM_DedicatedServer
		|| !IsValid(PlatformMesh)
		|| Cells.IsEmpty())
	{
		DestroyInteractionBounds();
		return;
	}
	
	const FBox PlatformMeshBounds = PlatformMesh->GetBoundingBox();
	if (!PlatformMeshBounds.IsValid)
	{
		DestroyInteractionBounds();
		return;
	}
	
	FBox LocalBounds(ForceInit);

	// Include every platform, including empty cells.
	for (const UGridStorageCell* Cell : Cells)
	{
		if (IsValid(Cell))
		{
			LocalBounds += PlatformMeshBounds.TransformBy(Cell->LocalTransform);
		}
	}

	if (!LocalBounds.IsValid)
	{
		DestroyInteractionBounds();
		return;
	}

	// Empty storage still has a usable interaction volume.
	LocalBounds.Max.Z += FMath::Max(0.0,	static_cast<double>(InteractionMinHeight));

	// Include items that extend above or outside the platform area.
	for (const auto& Pair : ItemVisuals)
	{
		UStaticMeshComponent* Visual = Pair.Value.Get();
		if (!IsValid(Visual))
		{
			continue;
		}

		UStaticMesh* ItemMesh = Visual->GetStaticMesh();
		if (!IsValid(ItemMesh))
		{
			continue;
		}

		const FBox ItemMeshBounds = ItemMesh->GetBoundingBox();
		if (!ItemMeshBounds.IsValid)
		{
			continue;
		}

		// Current item visuals are attached directly to this component.
		const FTransform VisualToGrid = Visual->GetRelativeTransform();
		if (const UInstancedStaticMeshComponent* Stack = Cast<UInstancedStaticMeshComponent>(Visual))
		{
			for (int32 Index = 0; Index < Stack->GetInstanceCount(); ++Index)
			{
				FTransform InstanceToVisual;
				if (Stack->GetInstanceTransform(Index, InstanceToVisual, false))
				{
					const FTransform InstanceToGrid = InstanceToVisual * VisualToGrid;
					LocalBounds += ItemMeshBounds.TransformBy(InstanceToGrid);
				}
			}
		}
		else
		{
			LocalBounds += ItemMeshBounds.TransformBy(VisualToGrid);
		}
	}

	const FVector Padding(
		FMath::Max(0.0, InteractionPadding.X),
		FMath::Max(0.0, InteractionPadding.Y),
		FMath::Max(0.0, InteractionPadding.Z));

	LocalBounds.Min -= Padding;
	LocalBounds.Max += Padding;

	const FVector Center = LocalBounds.GetCenter();
	const FVector Extent = LocalBounds.GetExtent().ComponentMax(FVector(0.1));

	if (!IsValid(InteractionBoundsComponent))
	{
		InteractionBoundsComponent = NewObject<UBoxComponent>(Owner,NAME_None,RF_Transient | RF_DuplicateTransient);
		InteractionBoundsComponent->SetMobility(EComponentMobility::Movable);
		InteractionBoundsComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		InteractionBoundsComponent->SetGenerateOverlapEvents(false);
		InteractionBoundsComponent->SetCanEverAffectNavigation(false);
		InteractionBoundsComponent->SetHiddenInGame(true);
		InteractionBoundsComponent->SetupAttachment(this);
	}

	UBoxComponent* Box = InteractionBoundsComponent.Get();
	const FTransform Placement(FQuat::Identity, Center);

	if (!Box->GetRelativeTransform().Equals(Placement))
	{
		Box->SetRelativeTransform(Placement);
	}

	if (!Box->GetUnscaledBoxExtent().Equals(Extent))
	{
		Box->SetBoxExtent(Extent, false);
	}

	Box->SetCollisionObjectType(ECC_WorldDynamic);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(InteractionChannel, ECR_Block);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (!Box->IsRegistered())
	{
		Box->RegisterComponent();
	}
}

void UStorageGridVisualizer::DestroyInteractionBounds()
{
	if (IsValid(InteractionBoundsComponent))
	{
		InteractionBoundsComponent->DestroyComponent();
	}

	InteractionBoundsComponent = nullptr;
}

void UStorageGridVisualizer::SynchronizeInventory()
{
	if (!IsRegistered() || !GetWorld() || !GetWorld()->IsGameWorld() || GetNetMode() == NM_DedicatedServer) return;
	USlotbasedInventory* FoundInventory = FindTargetInventory();
	const FIntPoint DesiredSize = FoundInventory ? FoundInventory->GetInventorySize(): FIntPoint::ZeroValue;

	if (TargetInventory != FoundInventory || BuiltGridSize != DesiredSize)
	{
		BindInventory(FoundInventory);
		BuildGrid(DesiredSize);
	}

	// Full reconciliation only on initialization, slot changes or replication.
	RefreshItemVisuals();
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

		ObserveInventorySource(GetOwner()->FindComponentByClass<UItemCollection>());
		BindInventory(FindTargetInventory());
		BuildGrid(TargetInventory ? TargetInventory->GetInventorySize()	: FIntPoint::ZeroValue);
		RefreshItemVisuals();

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
	ClearItemVisuals();
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
	
	DestroyInteractionBounds();
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
			// Mirror placement within the existing grid footprint; keep logical X/Y unchanged.
			const int32 VisualX = bInvertHorizontal ? GridSize.X - 1 - X : X;
			const int32 VisualY = bInvertVertical ? GridSize.Y - 1 - Y : Y;
			const FVector CellCorner(VisualX * Step.X, VisualY * Step.Y, 0.0);
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
	UpdateInteractionBounds();
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

void UStorageGridVisualizer::ObserveCollection(UItemCollection* Collection)
{
	if (ObservedCollection.Get() == Collection) return;
	if (ObservedCollection.IsValid())
	{
		ObservedCollection->OnInventoryItemsChanged.RemoveDynamic(
			this, &UStorageGridVisualizer::HandleInventoryItemsChanged);
	}
	ObservedCollection = Collection;
	if (IsValid(Collection))
	{
		Collection->OnInventoryItemsChanged.AddUniqueDynamic(
			this, &UStorageGridVisualizer::HandleInventoryItemsChanged);
	}
}

void UStorageGridVisualizer::HandleInventoryItemsChanged(const FString& InventoryID)
{
	if (IsValid(TargetInventory) && TargetInventory->GetInventoryContainerID() == InventoryID)
	{
		SynchronizeInventory();
	}
}

void UStorageGridVisualizer::HandleItemQuantityChanged(UObject* Item)
{
	if (!IsValid(Item))
	{
		return;
	}

	if (!UpdateItemVisual(Item))
	{
		// Preserve warning deduplication for a missing StorageMesh.
		if (UStaticMeshComponent* Visual = ItemVisuals.FindRef(Item);
			IsValid(Visual))
		{
			Visual->DestroyComponent();
		}

		ItemVisuals.Remove(Item);
	}
	
	UpdateInteractionBounds();
}

void UStorageGridVisualizer::ClearItemVisuals()
{
	for (const auto& Pair : ItemVisuals)
	{
		if (IsValid(Pair.Value)) Pair.Value->DestroyComponent();
	}
	ItemVisuals.Empty();
}

void UStorageGridVisualizer::RefreshItemVisuals()
{
	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld() || World->GetNetMode() == NM_DedicatedServer || !IsRegistered())
	{
		return;
	}

	UItemCollection* Collection = IsValid(TargetInventory) ? TargetInventory->GetItemCollectionLinked() : nullptr;
	ObserveCollection(Collection);
	if (!IsValid(Collection) || !GetStaticMesh() || Cells.IsEmpty())
	{
		ClearItemVisuals();
		UpdateInteractionBounds();
		return;
	}

	const FString InventoryID = TargetInventory->GetInventoryContainerID();
	TSet<UObject*> VisibleItems;
	TSet<TWeakObjectPtr<UObject>> CurrentItems;
	for (UObject* Item : Collection->GetAllItemsByContainer(InventoryID))
	{
		CurrentItems.Add(TWeakObjectPtr<UObject>(Item));
		if (UpdateItemVisual(Item)) VisibleItems.Add(Item);
	}

	for (auto It = ItemVisuals.CreateIterator(); It; ++It)
	{
		if (!VisibleItems.Contains(It.Key()))
		{
			if (IsValid(It.Value())) It.Value()->DestroyComponent();
			It.RemoveCurrent();
		}
	}
	for (auto It = WarnedMissingMeshes.CreateIterator(); It; ++It)
	{
		if (!CurrentItems.Contains(*It)) It.RemoveCurrent();
	}
	
	UpdateInteractionBounds();
}

void UStorageGridVisualizer::RemoveItemVisual(UObject* Item)
{
	if (UStaticMeshComponent* Visual = ItemVisuals.FindRef(Item); IsValid(Visual)) Visual->DestroyComponent();
	ItemVisuals.Remove(Item);
	WarnedMissingMeshes.Remove(TWeakObjectPtr<UObject>(Item));
	
	UpdateInteractionBounds();
}

bool UStorageGridVisualizer::UpdateLogsVisual(UObject* Item, UStaticMesh* Mesh, const FBox& OccupiedBounds,
	const FQuat& Rotation, int32 Quantity, EStorageMeshAxis MeshLengthAxis)
{
	const int32 VisibleCount = FMath::Clamp(Quantity, 0, 6);
	if (!IsValid(Item) || !IsValid(Mesh) || !OccupiedBounds.IsValid	|| VisibleCount == 0)
	{
		return false;
	}

	const FQuat MeshRotation = FStorageMeshOrientation::ToLengthAlongX(MeshLengthAxis);
	const FBox MeshBounds = Mesh->GetBoundingBox().TransformBy(FTransform(MeshRotation));
	if (!MeshBounds.IsValid)
	{
		return false;
	}

	const FVector MeshSize = MeshBounds.GetSize();

	if (MeshSize.Y <= UE_SMALL_NUMBER || MeshSize.Z <= UE_SMALL_NUMBER)
	{
		return false;
	}

	UStaticMeshComponent* Existing = ItemVisuals.FindRef(Item);
	UInstancedStaticMeshComponent* Stack = Cast<UInstancedStaticMeshComponent>(Existing);

	// Switching from SingleMesh to Logs.
	if (IsValid(Existing) && !IsValid(Stack))
	{
		Existing->DestroyComponent();
		ItemVisuals.Remove(Item);
	}

	bool bCreated = false;
	if (!IsValid(Stack))
	{
		Stack = NewObject<UInstancedStaticMeshComponent>(GetOwner(), NAME_None,RF_Transient);

		Stack->SetMobility(EComponentMobility::Movable);
		Stack->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Stack->SetGenerateOverlapEvents(false);
		Stack->SetCanEverAffectNavigation(false);
		Stack->SetupAttachment(this);

		bCreated = true;
	}

	const bool bMeshChanged = Stack->GetStaticMesh() != Mesh;
	const bool bWasSmallBoxes = Stack->ComponentTags.Remove(FSmallBoxStackLayout::ComponentTag) > 0;

	FTransform FirstInstance;
	const bool bAxisChanged = Stack->GetInstanceCount() > 0
		&& (!Stack->GetInstanceTransform(0, FirstInstance, false)
			|| !FirstInstance.GetRotation().Equals(MeshRotation));

	// Quantity above six does not change the visible instance count.
	const bool bRebuildInstances = bCreated || bMeshChanged || bWasSmallBoxes || bAxisChanged || Stack->GetInstanceCount() != VisibleCount;
	if (bMeshChanged)
	{
		Stack->SetStaticMesh(Mesh);
	}

	// The parent component represents the whole stack.
	// Its origin is the center of the occupied area at platform height.
	const FVector AreaCenter = OccupiedBounds.GetCenter();
	const FTransform StackTransform(Rotation,FVector(AreaCenter.X,AreaCenter.Y,OccupiedBounds.Max.Z));

	if (!Stack->GetRelativeTransform().Equals(StackTransform))
	{
		Stack->SetRelativeTransform(StackTransform);
	}

	if (bRebuildInstances)
	{
		Stack->ClearInstances();

		const FVector MeshCenter = MeshBounds.GetCenter();
		const double Width = MeshSize.Y;

		// Equilateral triangle packing for circular log cross-sections.
		const double RowHeight = MeshSize.Z * FMath::Sqrt(3.0) * 0.5;

		auto AddLog = [&](double CenterY, int32 Row)
		{
			// Compensate for an arbitrary mesh pivot.
			// The bottom row rests on the platform.
			const FVector Location(
				-MeshCenter.X,
				CenterY - MeshCenter.Y,
				Row * RowHeight - MeshBounds.Min.Z);

			Stack->AddInstance(FTransform(MeshRotation, Location),false);
		};

		// Bottom row: 1, 2 or 3 logs, centered as a group.
		const int32 BottomCount = FMath::Min(VisibleCount, 3);
		for (int32 Index = 0; Index < BottomCount; ++Index)
		{
			const double CenterY =(Index - (BottomCount - 1) * 0.5) * Width;
			AddLog(CenterY, 0);
		}

		// Second row rests in the gaps between three bottom logs.
		if (VisibleCount >= 4)
		{
			AddLog(-Width * 0.5, 1);
		}

		if (VisibleCount >= 5)
		{
			AddLog(Width * 0.5, 1);
		}

		if (VisibleCount >= 6)
		{
			AddLog(0.0, 2);
		}
	}

	if (bCreated)
	{
		Stack->RegisterComponent();
		ItemVisuals.Add(Item, Stack);
	}

	return true;
}

bool UStorageGridVisualizer::UpdateSmallBoxesVisual(UObject* Item, UStaticMesh* Mesh,
	const FBox& OccupiedBounds, const FQuat& Rotation, int32 Quantity)
{
	if (!IsValid(Item) || !IsValid(Mesh) || !OccupiedBounds.IsValid || Quantity <= 0) return false;
	const FBox MeshBounds = Mesh->GetBoundingBox();
	const FVector Size = MeshBounds.GetSize();
	if (!MeshBounds.IsValid || Size.GetMin() <= UE_SMALL_NUMBER) return false;

	UStaticMeshComponent* Existing = ItemVisuals.FindRef(Item);
	UInstancedStaticMeshComponent* Stack = Cast<UInstancedStaticMeshComponent>(Existing);
	if (IsValid(Existing) && !IsValid(Stack))
	{
		Existing->DestroyComponent();
		ItemVisuals.Remove(Item);
	}
	const bool bCreated = !IsValid(Stack);
	if (bCreated)
	{
		Stack = NewObject<UInstancedStaticMeshComponent>(GetOwner(), NAME_None, RF_Transient);
		Stack->SetMobility(EComponentMobility::Movable);
		Stack->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Stack->SetGenerateOverlapEvents(false);
		Stack->SetCanEverAffectNavigation(false);
		Stack->SetupAttachment(this);
	}

	// Rebuild only when switching mesh or layout, including Logs -> SmallBoxes.
	if (Stack->GetStaticMesh() != Mesh || !Stack->ComponentHasTag(FSmallBoxStackLayout::ComponentTag))
	{
		Stack->ClearInstances();
		Stack->SetStaticMesh(Mesh);
		Stack->ComponentTags.AddUnique(FSmallBoxStackLayout::ComponentTag);
	}
	const FVector Center = OccupiedBounds.GetCenter();
	const FTransform Placement(Rotation, FVector(Center.X, Center.Y, OccupiedBounds.Max.Z));
	if (!Stack->GetRelativeTransform().Equals(Placement)) Stack->SetRelativeTransform(Placement);
	FSmallBoxStackLayout::SetQuantity(*Stack, Quantity, MeshBounds, SmallBoxGap);
	if (bCreated)
	{
		Stack->RegisterComponent();
		ItemVisuals.Add(Item, Stack);
	}
	return true;
}

bool UStorageGridVisualizer::UpdateItemVisual(UObject* Item)
{
	UItemCollection* Collection = IsValid(TargetInventory) ? TargetInventory->GetItemCollectionLinked() : nullptr;
	if (!IsValid(Collection) || !GetStaticMesh() || Cells.IsEmpty()) return false;
	const FString InventoryID = TargetInventory->GetInventoryContainerID();
	const FBox PlatformBounds = GetStaticMesh()->GetBoundingBox();
	if (!IsValid(Item) || !Item->Implements<UObjectDataProvider>()) return false;

	const FItemStorageData Storage = IObjectDataProvider::Execute_GetItemRef(Item).ItemStorageData;
	if (!IsValid(Storage.StorageMesh))
	{
		const TWeakObjectPtr<UObject> ItemKey(Item);

		if (!WarnedMissingMeshes.Contains(ItemKey))
		{
			WarnedMissingMeshes.Add(ItemKey);
			UE_LOG(LogStorageGridVisualizer, Warning,
				TEXT("%s: item '%s' in inventory '%s' has no StorageMesh assigned. Visual skipped."),
				*GetPathName(), *Item->GetPathName(), *InventoryID);
		}
		return false;
	}

	WarnedMissingMeshes.Remove(TWeakObjectPtr<UObject>(Item));
	const FItemMapping* Mapping = Collection->FindItemMappingByContainerName(Item, InventoryID);
	if (!Mapping || Mapping->OccupiedSlots.IsEmpty()) return false;

	FBox OccupiedBounds(ForceInit);
	bool bAllSlotsReady = true;
	for (UInventorySlotData* Slot : Mapping->OccupiedSlots)
	{
		const UGridStorageCell* Cell = IsValid(Slot) ? GetCell(Slot->InventorySlotInfo.CellPosition) : nullptr;
		if (!IsValid(Cell))
		{
			bAllSlotsReady = false;
			break;
		}
		OccupiedBounds += PlatformBounds.TransformBy(Cell->LocalTransform);
	}
	// Never center on a partial set while slot references are still replicating.
	if (!bAllSlotsReady || !OccupiedBounds.IsValid) return false;

	const FQuat Rotation = FRotator(0.0,Mapping->ItemOrientation == EItemOrientationType::Vertical ? 90.0 : 0.0, 0.0).Quaternion();
	switch (Storage.StorageMethod)
	{
		case EStorageMethod::SingleMesh:
		{
			const FTransform Placement = StorageGridPlacement::MakeTransform(
				OccupiedBounds, Storage.StorageMesh->GetBoundingBox(), Rotation);

			UStaticMeshComponent* Visual = ItemVisuals.FindRef(Item);
			if (IsValid(Visual) && Visual->IsA<UInstancedStaticMeshComponent>())
			{
				Visual->DestroyComponent();
				ItemVisuals.Remove(Item);
				Visual = nullptr;
			}
			if (!IsValid(Visual))
			{
				Visual = NewObject<UStaticMeshComponent>(GetOwner(), NAME_None, RF_Transient);
				Visual->SetMobility(EComponentMobility::Movable);
				Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Visual->SetGenerateOverlapEvents(false);
				Visual->SetCanEverAffectNavigation(false);
				Visual->SetupAttachment(this);
				Visual->SetStaticMesh(Storage.StorageMesh);
				Visual->SetRelativeTransform(Placement);
				Visual->RegisterComponent();
				ItemVisuals.Add(Item, Visual);
			}
			else
			{
				if (Visual->GetStaticMesh() != Storage.StorageMesh) Visual->SetStaticMesh(Storage.StorageMesh);
				if (!Visual->GetRelativeTransform().Equals(Placement)) Visual->SetRelativeTransform(Placement);
			}
			return true;
		}

		case EStorageMethod::Logs:
		{
			return UpdateLogsVisual(
				Item, Storage.StorageMesh, OccupiedBounds,
				FStorageMeshOrientation::LogsGridRotation(Mapping->ItemOrientation),
				IObjectDataProvider::Execute_GetQuantity(Item), Storage.MeshLengthAxis);
		}

		case EStorageMethod::SmallBoxes:
		{
			return UpdateSmallBoxesVisual(
				Item, Storage.StorageMesh, OccupiedBounds, Rotation,
				IObjectDataProvider::Execute_GetQuantity(Item));
		}

		default:
			return false;
	}
}

void UStorageGridVisualizer::ObserveInventorySource(UItemCollection* Collection)
{
	if (InventorySourceCollection.Get() == Collection) return;
	if (InventorySourceCollection.IsValid())
	{
		InventorySourceCollection->OnActorInventoriesChanged.RemoveDynamic(this, &UStorageGridVisualizer::SynchronizeInventory);
	}
	InventorySourceCollection = Collection;
	if (IsValid(Collection))
	{
		Collection->OnActorInventoriesChanged.AddUniqueDynamic(this, &UStorageGridVisualizer::SynchronizeInventory);
	}
}

void UStorageGridVisualizer::BindInventory(USlotbasedInventory* Inventory)
{
	if (TargetInventory == Inventory) return;
	if (IsValid(TargetInventory))
	{
		TargetInventory->OnAddItemDelegate.RemoveDynamic(this, &UStorageGridVisualizer::HandleItemAdded);
		TargetInventory->OnItemRemovedDelegate.RemoveDynamic(this, &UStorageGridVisualizer::HandleItemRemoved);
		TargetInventory->OnItemReplaceDelegate.RemoveDynamic(this, &UStorageGridVisualizer::HandleItemReplaced);
		TargetInventory->OnStackedItemDelegate.RemoveDynamic(this, &UStorageGridVisualizer::HandleItemQuantityChanged);
		TargetInventory->OnUnstackedItemDelegate.RemoveDynamic(this, &UStorageGridVisualizer::HandleItemQuantityChanged);
		TargetInventory->OnInventorySlotDataUpdated.RemoveDynamic(this, &UStorageGridVisualizer::SynchronizeInventory);
		TargetInventory->OnInventoryRedrawRequested.RemoveDynamic(this, &UStorageGridVisualizer::SynchronizeInventory);
	}
	TargetInventory = Inventory;
	WarnedMissingMeshes.Empty();
	if (IsValid(TargetInventory))
	{
		TargetInventory->OnAddItemDelegate.AddUniqueDynamic(this, &UStorageGridVisualizer::HandleItemAdded);
		TargetInventory->OnItemRemovedDelegate.AddUniqueDynamic(this, &UStorageGridVisualizer::HandleItemRemoved);
		TargetInventory->OnItemReplaceDelegate.AddUniqueDynamic(this, &UStorageGridVisualizer::HandleItemReplaced);
		TargetInventory->OnStackedItemDelegate.AddUniqueDynamic(this, &UStorageGridVisualizer::HandleItemQuantityChanged);
		TargetInventory->OnUnstackedItemDelegate.AddUniqueDynamic(this,	&UStorageGridVisualizer::HandleItemQuantityChanged);
		TargetInventory->OnInventorySlotDataUpdated.AddUniqueDynamic(this, &UStorageGridVisualizer::SynchronizeInventory);
		TargetInventory->OnInventoryRedrawRequested.AddUniqueDynamic(this, &UStorageGridVisualizer::SynchronizeInventory);
	}
}

void UStorageGridVisualizer::HandleItemAdded(FItemMapping& Mapping, UObject* Item)
{
	if (!UpdateItemVisual(Item))
	{
		// Keep warning deduplication until this item is removed or fixed.
		if (UStaticMeshComponent* Visual = ItemVisuals.FindRef(Item); IsValid(Visual)) Visual->DestroyComponent();
		
		ItemVisuals.Remove(Item);
		
	}
	
	UpdateInteractionBounds();
}

void UStorageGridVisualizer::HandleItemRemoved(FItemMapping Mapping, UObject* Item)
{
	RemoveItemVisual(Item);
}

void UStorageGridVisualizer::HandleItemReplaced(TArray<UInventorySlotData*> OldSlots, FItemMapping& Mapping, UObject* Item)
{
	HandleItemAdded(Mapping, Item);
}
