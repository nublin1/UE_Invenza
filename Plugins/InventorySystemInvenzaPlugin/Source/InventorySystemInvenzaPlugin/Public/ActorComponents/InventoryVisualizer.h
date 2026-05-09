//  Nublin Studio 2026 All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "InventoryVisualizer.generated.h"

struct FItemMapping;
class UItemBase;
class UInventoryBase;

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

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Visualizer")
	void InitializeInventoriesByTag(FGameplayTag ContainerTag, bool bTrackAll = false);
	
	UFUNCTION(BlueprintCallable, Category = "Visualizer")
	void InitializeCachedSlots();
	
	UFUNCTION(BlueprintCallable, Category = "Visualizer")
	void RefreshVisuals();

	UFUNCTION(BlueprintCallable, Category = "Visualizer")
	void AddItemVisual(FItemMapping& ItemSlots, UItemBase* Item);

	UFUNCTION(BlueprintCallable, Category = "Visualizer")
	void RemoveItemVisual(FItemMapping ItemSlots, UItemBase* Item);

	UFUNCTION(blueprintCallable, Category = "Visualizer")
	float GetTotalOccupancy() const;
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualizer|Settings")
	bool bTrackAllInvs = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualizer|Settings")
	FGameplayTag DefaultSearchTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Visualizer|Settings")
	TArray<TObjectPtr<UInventoryBase>> TargetInventories;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Visualizer|Settings")
	EVisualizerMode DisplayMode = EVisualizerMode::IndividualItems;

	// Ключевые слова для поиска сокетов (например, "ShelfSlot", "Pos")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualizer|Individual")
	TArray<FString> SocketKeywords;

	// Карта: Процент заполнения (0.0 - 1.0) -> Меш полки
	// Например: 0.0 -> Mesh_Empty, 0.5 -> Mesh_HalfFull, 0.9 -> Mesh_Full
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualizer|Occupancy", meta = (EditCondition = "DisplayMode == EVisualizerMode::OccupancyMeshSwap"))
	TMap<float, TObjectPtr<UStaticMesh>> OccupancyMeshes;

	UPROPERTY()
	TMap<TObjectPtr<UItemBase>, TObjectPtr<UStaticMeshComponent>> TrackedVisuals;
	UPROPERTY()
	TMap<int32, TObjectPtr<UItemBase>> OccupiedSocketIndices;

	UPROPERTY()
	TObjectPtr<UMeshComponent> ParentMeshPtr;

	UPROPERTY()
	TArray<FCachedSocketData> CachedSlots;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	int32 GetFirstFreeSocketIndex() const;
	
	void UpdateOccupancyMesh();
	void UpdateIndividualItems();

	void FindParentMesh();
};
