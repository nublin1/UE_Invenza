// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GridStorageCell.h"
#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "StorageGridVisualizer.generated.h"


class USlotbasedInventory;
enum class EStorageMeshAxis : uint8;
class UItemCollection;
class UInventorySlotData;
struct FItemMapping;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UStorageGridVisualizer : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:
	UStorageGridVisualizer();
	
protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	
public:
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	/**
	 * Editor: builds using PreviewGridSize. Game: finds an inventory and builds using its size.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Storage Grid")
	void RebuildGrid();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Storage Grid")
	void ClearGrid();

	/** Reconcile SingleMesh visuals with the current inventory contents. */
	UFUNCTION(BlueprintCallable, Category = "Storage Grid")
	void RefreshItemVisuals();

	UFUNCTION(BlueprintPure, Category = "Storage Grid")
	UGridStorageCell* GetCell(FIntPoint Coordinates) const;

	UFUNCTION(BlueprintPure, Category = "Storage Grid")
	FIntPoint GetBuiltGridSize() const
	{
		return BuiltGridSize;
	}
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere,	BlueprintReadOnly, Category = "Storage Grid|Inventory")
	FGameplayTag InventorySearchTag;
	
	/** Allow interaction with the entire storage area, including empty cells. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage Grid|Interaction")
	bool bEnableInteractionBounds = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage Grid|Interaction", meta = (EditCondition = "bEnableInteractionBounds"))
	TEnumAsByte<ECollisionChannel> InteractionChannel = ECC_GameTraceChannel1;
	
	/** Minimum interaction height above the platforms, along the grid's local Z. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage Grid|Interaction",
		meta = (
			EditCondition = "bEnableInteractionBounds",
			ClampMin = "0.0",
			UIMin = "0.0",
			Units = "cm"
		))
	float InteractionMinHeight = 100.0f;
	
	/** Additional space around the interaction bounds in grid-local coordinates. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage Grid|Interaction",
		meta = (
			EditCondition = "bEnableInteractionBounds",
			ClampMin = "0.0",
			UIMin = "0.0",
			Units = "cm"
		))
	FVector InteractionPadding = FVector(2.0, 2.0, 0.0);
	
	// Geometry
	/**
	 * Scale of each mesh instance.
	 * Grid spacing is calculated from the scaled mesh bounds.
	 */
	UPROPERTY(EditAnywhere,	BlueprintReadOnly,Category = "Storage Grid|Geometry", meta = (ClampMin = "0.001", UIMin = "0.001"))
	FVector CellScale = FVector::OneVector;
	
	UPROPERTY(EditAnywhere,	BlueprintReadOnly,Category = "Storage Grid|Geometry", meta = (ClampMin = "0.0", UIMin = "0.0"))
	FVector2D CellGap = FVector2D::ZeroVector;

	/** Space between boxes along the stack's local X/Y axes, in centimeters. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage Grid|Small Boxes", meta = (ClampMin = "0.0", UIMin = "0.0", Units = "cm"))
	FVector2D SmallBoxGap = FVector2D(1.0, 1.0);

	/** Reverse column placement along local X without changing logical slot coordinates. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage Grid|Geometry", meta = (DisplayName = "Invert Horizontal (X)"))
	bool bInvertHorizontal = false;

	/** Reverse row placement along local Y without changing logical slot coordinates. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage Grid|Geometry", meta = (DisplayName = "Invert Vertical (Y)"))
	bool bInvertVertical = false;

	/** Limits accidental generation of excessively large grids. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage Grid|Geometry", meta = (ClampMin = "1"))
	int32 MaxCellCount = 200;
	
	// Editor preview
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category = "Storage Grid|Preview")
	bool bPreviewInEditor = true;

	UPROPERTY(EditAnywhere,	BlueprintReadOnly, Category = "Storage Grid|Preview", meta = (EditCondition = "bPreviewInEditor"))
	FIntPoint PreviewGridSize = FIntPoint(4, 6);
	
	// Runtime
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Storage Grid|Runtime")
	TObjectPtr<USlotbasedInventory> TargetInventory;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Storage Grid|Runtime")
	FIntPoint BuiltGridSize = FIntPoint::ZeroValue;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Storage Grid|Runtime")
	TArray<TObjectPtr<UGridStorageCell>> Cells;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Storage Grid|Runtime")
	TMap<TObjectPtr<UObject>, TObjectPtr<UStaticMeshComponent>> ItemVisuals;
	
	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<UBoxComponent> InteractionBoundsComponent;

	TWeakObjectPtr<UItemCollection> ObservedCollection;
	TWeakObjectPtr<UItemCollection> InventorySourceCollection;
	TSet<TWeakObjectPtr<UObject>> WarnedMissingMeshes;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	USlotbasedInventory* FindTargetInventory() const;
	
	void UpdateInteractionBounds();
	void DestroyInteractionBounds();

	UFUNCTION()
	void SynchronizeInventory();
	void BindInventory(USlotbasedInventory* Inventory);
	void ObserveInventorySource(UItemCollection* Collection);
	bool UpdateItemVisual(UObject* Item);
	void RemoveItemVisual(UObject* Item);
	bool UpdateLogsVisual(UObject* Item, UStaticMesh* Mesh, const FBox& OccupiedBounds, const FQuat& Rotation, int32 Quantity, EStorageMeshAxis MeshLengthAxis);
	bool UpdateSmallBoxesVisual(UObject* Item, UStaticMesh* Mesh, const FBox& OccupiedBounds, const FQuat& Rotation, int32 Quantity);

	UFUNCTION()
	void HandleItemAdded(FItemMapping& Mapping, UObject* Item);
	UFUNCTION()
	void HandleItemRemoved(FItemMapping Mapping, UObject* Item);
	UFUNCTION()
	void HandleItemReplaced(TArray<UInventorySlotData*> OldSlots, FItemMapping& Mapping, UObject* Item);
	void BuildGrid(FIntPoint GridSize);
	void ClearItemVisuals();
	void ObserveCollection(UItemCollection* Collection);

	UFUNCTION()
	void HandleInventoryItemsChanged(const FString& InventoryID);
	
	UFUNCTION()
	void HandleItemQuantityChanged(UObject* Item);
	
};
