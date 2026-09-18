//  Nublin Studio 2026 All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "InventoryVisualizer.generated.h"

struct FItemMapping;
class UObject;
class UInventoryBase;
class UItemCollection;

UENUM(BlueprintType)
enum class EVisualizerMode : uint8
{
	IndividualItems    UMETA(DisplayName = "Spawn Each Item"),
	OccupancyMeshSwap  UMETA(DisplayName = "Swap Mesh by Percentage")
};

USTRUCT(BlueprintType)
struct FCachedSocketData
{
	GENERATED_BODY()

	UPROPERTY()
	FName SocketName;

	UPROPERTY()
	FTransform RelativeTransform;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryVisualizer : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryVisualizer();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// Public API
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Inventory|Visualizer")
	void InitializeInventoriesByTag(FGameplayTag ContainerTag, bool bTrackAll = false);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Visualizer")
	void InitializeCachedSlots();
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Visualizer")
	void RefreshVisuals();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Visualizer")
	void AddItemVisual(FItemMapping& ItemSlots, UObject* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Visualizer")
	void RemoveItemVisual(FItemMapping ItemSlots, UObject* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Visualizer")
	float GetTotalOccupancy() const;
	
protected:
	//====================================================================
	// Settings: inventory selection
	//====================================================================
	/** Track all inventories owned by the actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualizer|Settings")
	bool bTrackAllInvs = false;
	
	/** Inventory tag used when tracking all inventories is disabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualizer|Settings")
	FGameplayTag DefaultSearchTag;
	
	// Settings: display	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Visualizer|Settings")
	EVisualizerMode DisplayMode = EVisualizerMode::IndividualItems;

	/** A socket is eligible if its name contains any of these keywords. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualizer|Individual")
	TArray<FString> SocketKeywords;

	/**
	 * Minimum occupancy fraction -> mesh.
	 * Keys use 0..1. Include a 0.0 entry for the empty state.
	 * The highest threshold not exceeding current occupancy is selected.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualizer|Occupancy", meta = (EditCondition = "DisplayMode == EVisualizerMode::OccupancyMeshSwap"))
	TMap<float, TObjectPtr<UStaticMesh>> OccupancyMeshes;
	
	
	// Runtime
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_TargetInventories, Category = "Visualizer|Settings")
	TArray<TObjectPtr<UInventoryBase>> TargetInventories;

	TArray<TWeakObjectPtr<UInventoryBase>> BoundInventories;
	TWeakObjectPtr<UItemCollection> ObservedCollection;
	
	// Runtime: visuals and sockets
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly,Category = "Inventory|Visualizer|Runtime")
	TArray<FCachedSocketData> CachedSlots;
	
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly,Category = "Inventory|Visualizer|Runtime")
	TMap<TObjectPtr<UObject>, TObjectPtr<UStaticMeshComponent>> TrackedVisuals;
	
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly,Category = "Inventory|Visualizer|Runtime")
	TMap<int32, TObjectPtr<UObject>> OccupiedSocketIndices;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly,Category = "Inventory|Visualizer|Runtime")
	TObjectPtr<UMeshComponent> ParentMeshPtr;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	// Replication and events
	
	UFUNCTION()
	void OnRep_TargetInventories();

	UFUNCTION()
	void HandleInventoryItemsChanged(const FString& InventoryID);

	UFUNCTION()
	void UnbindInventoryEvents();
	
	// Internal helpers
	void UpdateOccupancyMesh();
	void UpdateIndividualItems();

	void FindParentMesh();
	
	int32 GetFirstFreeSocketIndex() const;
};
