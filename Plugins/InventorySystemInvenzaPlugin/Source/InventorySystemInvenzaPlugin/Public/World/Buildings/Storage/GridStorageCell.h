// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridStorageCell.generated.h"

UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UGridStorageCell : public UObject
{
	GENERATED_BODY()

public:
	UGridStorageCell();

	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	/** Coordinates in the grid. */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	FIntPoint Coordinates = FIntPoint::ZeroValue;
	
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	int32 InstanceIndex = INDEX_NONE;
	
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	FTransform LocalTransform = FTransform::Identity;
	
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	FVector LocalCenter = FVector::ZeroVector;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsCellValid() const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	FTransform GetWorldTransform() const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	FVector GetWorldCenter() const;
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
private:
	friend class UStorageGridVisualizer;

	TWeakObjectPtr<UStorageGridVisualizer> Visualizer;
};
