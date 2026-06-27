//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CraftingTypes.h"
#include "Components/ActorComponent.h"
#include "CraftingComponent.generated.h"


class UInventoryBase;
class USlotbasedInventory;
struct FItemIDEntry;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UCraftingComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAvailableRecipesChanged);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftStateChanged, bool, bIsPaused);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftDataChanged, FQueuedRecipe&, Recipe);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftQueueChanged,TArray<FQueuedRecipe>&,NewQueue);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlocksUpdated, TArray<FBlockReasonData>, Blocks);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewCraftStarted, FQueuedRecipe, QueuedRecipe);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftFinished, FName, RecipeID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCraftCanceled);
#pragma endregion

public:
	UCraftingComponent();

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
	FOnAvailableRecipesChanged OnAvailableRecipesChanged;

	UPROPERTY(BlueprintAssignable, Category="Crafting|Delegates")
	FOnCraftStateChanged OnCraftStateChanged;
	UPROPERTY(BlueprintAssignable, Category="Crafting|Delegates")
	FOnBlocksUpdated OnBlocksUpdated;
	
	UPROPERTY(BlueprintAssignable, Category="Crafting|Events")
	FOnNewCraftStarted OnNewCraftStarted;
	UPROPERTY(BlueprintAssignable, Category="Crafting|Events")
	FOnCraftDataChanged OnCraftDataChanged;
	UPROPERTY(BlueprintAssignable, Category="Crafting|Events")
	FOnCraftQueueChanged OnCraftQueueChanged;
	UPROPERTY(BlueprintAssignable, Category="Crafting|Events")
	FOnCraftFinished OnCraftFinished;
	UPROPERTY(BlueprintAssignable, Category="Crafting|Events")
	FOnCraftCanceled OnCraftCanceled;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void RequestInitCraftingComponent();
	UFUNCTION(Server, Reliable,  Category = "Crafting")
	void Server_InitCraftingComponent();
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void HandleInitCraftingComponent();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Crafting")
	void SetInputInventory(UInventoryBase* NewInputInventory);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Crafting")
	void SetOutputInventory(UInventoryBase* NewOutputInventory);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void RequestRecalculateAvailableRecipes();
	UFUNCTION(Server, Reliable)
	void Server_RecalculateAvailableRecipes();
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void HandleRecalculateAvailableRecipes();

	UFUNCTION(BlueprintCallable, Category="Crafting")
	void SetManualPauseRequest(bool bNewPaused);
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void SetNoResourcesRequest(bool bNewValue);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
	FCraftingComponentConfig GetConfig() const { return Config; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
	TArray<FItemRecipeRow> GetAvailableRecipes() const { return AvailableRecipes; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
	TArray<FCachedRecipeResult> GetCachedRecipeResults() const { return CachedRecipeResults; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
	bool GetCachedResultForRecipe(FName RecipeID, FRecipeCheckResult& OutResult) const;
	
	UFUNCTION(BlueprintCallable)
	TArray<FBlockReasonData> GetBlocksReasons() {return BlocksReasons; }
	
	UFUNCTION(BlueprintCallable)
	bool GetIsManualPaused() { return IsManualPaused; } 
	
	UFUNCTION()
	FRecipeCheckResult CanCraft(const FItemRecipeRow& RecipeRow, const TArray<int32>& SelectedOptions, int32 Amount = 1) const;
	UFUNCTION(BlueprintCallable, Category="Crafting")
	static FRecipeCheckResult CanCraftWithItems(const FItemRecipeRow& RecipeRow,
	                                            const TArray<FItemIDEntry>& InventoryItems,
	                                            int32 Amount = 1);
	
	UFUNCTION(BlueprintCallable, Category="Crafting")
	static FRecipeCheckResult CanCraftWithItemsOptions(const FItemRecipeRow& RecipeRow,
												const TArray<FItemIDEntry>& InventoryItems,
												const TArray<int32>& SelectedOptions,
												int32 Amount = 1);
	
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void EnqueueRecipeRequest(FItemRecipeRow ItemRecipeRow, const TArray<int32>& SelectedOptions, int32 Count = 1);
	
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void CancelRecipeRequest(int32 QueueIndex);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void RequestMoveQueueItem(FName RecipeID, int32 QueueIndex, bool bMoveUp);

protected:

	// Refs
	UPROPERTY(ReplicatedUsing=OnRep_InventoryUpdated, VisibleInstanceOnly, BlueprintReadWrite, Category="Crafting|Ref")
	TObjectPtr<UInventoryBase> InputInventory;

	UPROPERTY(ReplicatedUsing=OnRep_InventoryUpdated, VisibleInstanceOnly, BlueprintReadWrite, Category="Crafting|Ref")
	TObjectPtr<UInventoryBase> OutputInventory;
	
	// Config
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crafting|Config")
	FCraftingComponentConfig Config;

	// Runtime
	UPROPERTY(ReplicatedUsing=OnRep_AvailableRecipes, VisibleInstanceOnly, BlueprintReadOnly, Category="Crafting|Runtime")
	TArray<FItemRecipeRow> AvailableRecipes;
	
	UPROPERTY(ReplicatedUsing=OnRep_CachedRecipes, VisibleInstanceOnly, BlueprintReadOnly, Category="Crafting")
	TArray<FCachedRecipeResult> CachedRecipeResults;
	
	UPROPERTY(ReplicatedUsing=OnRep_Blocks, VisibleInstanceOnly, BlueprintReadWrite, Category="Crafting|Runtime")
	TArray<FBlockReasonData> BlocksReasons;
	
	UPROPERTY(ReplicatedUsing=OnRep_Blocks, VisibleInstanceOnly, BlueprintReadWrite, Category="Crafting|Runtime")
	bool IsManualPaused = false;

	// Settings
	UPROPERTY(editAnywhere, BlueprintReadWrite)
	bool bDebugMode = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CraftingSpeed = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProcessCraftTickTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
	ECraftingResourceConsumePolicy ConsumePolicy =
		ECraftingResourceConsumePolicy::OnCraftStart;

	UPROPERTY(EditDefaultsOnly, Category="Crafting|Blocks")
	FBlockReasonData Block_NoResources;

	UPROPERTY(EditDefaultsOnly, Category="Crafting|Blocks")
	FBlockReasonData Block_ManualPause;

	// Data
	UPROPERTY()
	FTimerHandle CraftTimerHandle;

	UPROPERTY(ReplicatedUsing=OnRep_Queue, VisibleInstanceOnly, BlueprintReadOnly, Category="Crafting")
	TArray<FQueuedRecipe> RecipeQueue;
	
	UPROPERTY(ReplicatedUsing=OnRep_CurrentRecipe, VisibleInstanceOnly, BlueprintReadOnly, Category="Crafting")
	FQueuedRecipe CurrentCraftingRecipe;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void OnRep_InventoryUpdated();

	UFUNCTION()
	void OnRep_CachedRecipes();
	
	UFUNCTION()
	void OnRep_AvailableRecipes();

	UFUNCTION()
	void OnRep_Blocks();
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_EnqueueRecipe(FItemRecipeRow ItemRecipeRow, const TArray<int32>& SelectedOptions, int32 Count);
	UFUNCTION(BlueprintCallable)
	void HandleEnqueueRecipe(FItemRecipeRow ItemRecipeRow,const TArray<int32>& SelectedOptions, int32 Count);
	UFUNCTION(Server, Reliable)
	void Server_CancelRecipe(int32 QueueIndex);
	UFUNCTION(BlueprintCallable)
	void HandleCancelRecipe(int32 QueueIndex);

	UFUNCTION()
	void ProcessCraftTick();

	//
	void TryStartNext();
	void StartCurrentRecipe(FQueuedRecipe& Item);
	void FinishCurrentRecipe();
	bool ConsumeResourcesForRecipe(FQueuedRecipe& Item, int32 Count);
	void RefundResourcesForRecipe(const FQueuedRecipe& Item, int32 Count);
	void GiveCraftedItemToInventory(FItemRecipeRow CraftedRow);

	// --- Notification replication ---
	UFUNCTION()
	void OnRep_Queue();
	UFUNCTION()
	void OnRep_CurrentRecipe();

	//
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnBlocksUpdated (const TArray<FBlockReasonData>& BlocksActive);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnCraftStarted();
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnCraftFinished(FName RecipeID);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnCraftCanceled();

	//
	UFUNCTION(Server, Reliable)
	void Server_MoveQueueItem(FName RecipeID, int32 QueueIndex, bool bMoveUp);
	UFUNCTION()
	void Handle_MoveQueueItem(FName RecipeID, int32 QueueIndex, bool bMoveUp);

	UFUNCTION()
	void CurrentRecipeProgressChanged();

	UFUNCTION(BlueprintCallable)
	void SaveCurrentProgressToQueue();
	
};
