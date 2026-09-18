// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GridStorageCell.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "StorageGridVisualizer.generated.h"


class USlotbasedInventory;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UStorageGridVisualizer : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:
	UStorageGridVisualizer();
	
protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	
public:
	virtual void TickComponent(float DeltaTime,	ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
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
	
	// Geometry
	/**
	 * Scale of each mesh instance.
	 * Grid spacing is calculated from the scaled mesh bounds.
	 */
	UPROPERTY(EditAnywhere,	BlueprintReadOnly,Category = "Storage Grid|Geometry", meta = (ClampMin = "0.001", UIMin = "0.001"))
	FVector CellScale = FVector::OneVector;
	
	UPROPERTY(EditAnywhere,	BlueprintReadOnly,Category = "Storage Grid|Geometry", meta = (ClampMin = "0.0", UIMin = "0.0"))
	FVector2D CellGap = FVector2D::ZeroVector;

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
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	USlotbasedInventory* FindTargetInventory() const;

	void SynchronizeInventory();
	void BuildGrid(FIntPoint GridSize);
	
};
