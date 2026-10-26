//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/CraftSystem/CraftingEnums.h"
#include "Data/CraftSystem/CraftingStructs.h"
#include "Components/ActorComponent.h"
#include "CraftingComponent.generated.h"

class UInventoryBase;
class USlotbasedInventory;
class UInventorySlotData;
struct FItemMapping;
struct FItemIDEntry;
struct FInventorySavedCraft;
struct FInventoryPreparedCraft;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UCraftingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCraftingComponent();

protected:
	// Unreal overrides
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAvailableRecipesChanged);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftStateChanged, bool, bIsPaused);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAutomaticCraftingChanged, bool, bAutomatic);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentCraftDataChanged, const FQueuedRecipe&, Recipe);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftQueueChanged, const TArray<FQueuedRecipe>&, NewQueue);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlocksUpdated, TArray<FBlockReasonData>, Blocks);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewCraftStarted, FQueuedRecipe, QueuedRecipe);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftFinished, FName, RecipeID);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCraftCanceled);
#pragma endregion Delegates

	// Properties and variables
	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
	FOnAvailableRecipesChanged OnAvailableRecipesChanged;

	UPROPERTY(BlueprintAssignable, Category = "Crafting|Delegates")
	FOnCraftStateChanged OnCraftStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
	FOnAutomaticCraftingChanged OnAutomaticCraftingChanged;

	UPROPERTY(BlueprintAssignable, Category = "Crafting|Delegates")
	FOnBlocksUpdated OnBlocksUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
	FOnNewCraftStarted OnNewCraftStarted;

	UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
	FOnCurrentCraftDataChanged OnCurrentCraftDataChanged;

	UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
	FOnCraftQueueChanged OnCraftQueueChanged;

	UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
	FOnCraftFinished OnCraftFinished;

	UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
	FOnCraftCanceled OnCraftCanceled;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void RequestInitCraftingComponent();

	bool CapturePersistence(FInventorySavedCraft& Data, FString& Error) const;

	bool PreparePersistence(const FInventorySavedCraft& Data, FInventoryPreparedCraft& Plan, FString& Error);

	void CommitPersistence(FInventoryPreparedCraft& Plan);

	void ResumeAfterPersistence();

	UFUNCTION(Server, Reliable, Category = "Crafting")
	void Server_InitCraftingComponent();

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void HandleInitCraftingComponent();

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void AddOperator();

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void RemoveOperator();

	UFUNCTION(BlueprintPure, Category = "Crafting|Config")
	bool IsAutomaticCraftingAllowed() const { return bAllowAutomaticCrafting; }

	/** Changes the mode on the authority; player requests go through their inventory manager. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Crafting|Config")
	void SetAllowAutomaticCrafting(bool bAutomatic);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool HasFuelAvailable() const;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Crafting")
	void SetInventories(UInventoryBase* NewInputInventory, UInventoryBase* NewOutputInventory,
						UInventoryBase* NewFuelInventory);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Crafting")
	void SetInputInventory(UInventoryBase* NewInputInventory);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Crafting")
	void SetOutputInventory(UInventoryBase* NewOutputInventory);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Crafting")
	void SetFuelInventory(UInventoryBase* NewFuelInventory);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Crafting")
	void SetInteractorInventory(UInventoryBase* NewInteractorInventory);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void RequestRecalculateAvailableRecipes();

	UFUNCTION(Server, Reliable)
	void Server_RecalculateAvailableRecipes();

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void HandleRecalculateAvailableRecipes();

	UFUNCTION(BlueprintCallable, BlueprintCallable, Category = "Crafting")
	void SetBlockStateRequest(const FBlockReasonData& BlockReason, bool bBlocked);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Crafting")
	void Server_SetBlockState(const FBlockReasonData& BlockReason, bool bBlocked);

	UFUNCTION(BlueprintCallable, BlueprintCallable, Category = "Crafting")
	void HandleSetBlockState(const FBlockReasonData& BlockReason, bool bBlocked);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void SetNoResourcesRequest(bool bNewValue);

	UFUNCTION(Category = "Crafting")
	UInventoryBase* GetInputInventory() const { return InputInventory; };

	UFUNCTION(Category = "Crafting")
	UInventoryBase* GetOutputInventory() const { return OutputInventory; };

	UFUNCTION(Category = "Crafting")
	UInventoryBase* GetFuelInventory() const { return FuelInventory; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
	FCraftingInventoryOverrides GetConfig() const { return Config; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
	TArray<FItemRecipeRow> GetAvailableRecipes() const { return AvailableRecipes; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
	TArray<FQueuedRecipe> GetQueueItems() const { return RecipeQueue.Items; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
	FQueuedRecipe GetCurrentCraftingRecipe() const { return CurrentCraftingRecipe; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
	TArray<FCachedRecipeResult> GetCachedRecipeResults() const { return CachedRecipeResults; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
	bool GetCachedResultForRecipe(FName RecipeID, FRecipeCheckResult& OutResult) const;

	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	TArray<FBlockReasonData> GetBlocksReasons() { return ActiveBlocksReasons; }

	UFUNCTION()
	FRecipeCheckResult CanCraft(const FItemRecipeRow& RecipeRow, const TArray<int32>& SelectedOptions,
								int32 Amount = 1) const;

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	static FRecipeCheckResult CanCraftWithItems(const FItemRecipeRow& RecipeRow,
												const TArray<FItemIDEntry>& InventoryItems, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	static FRecipeCheckResult CanCraftWithItemsOptions(const FItemRecipeRow& RecipeRow,
													   const TArray<FItemIDEntry>& InventoryItems,
													   const TArray<int32>& SelectedOptions, int32 Amount = 1);

	// Zero means no finite, currently craftable resource maximum (including resource-free recipes).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
	int32 GetMaxCraftAmount(const FItemRecipeRow& RecipeRow, const TArray<int32>& SelectedOptions) const;

	static int32 GetMaxCraftAmountWithItems(const FItemRecipeRow& RecipeRow, const TArray<FItemIDEntry>& Items,
											const TArray<int32>& SelectedOptions);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void EnqueueRecipeRequest(FItemRecipeRow ItemRecipeRow, const TArray<int32>& SelectedOptions, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void CancelRecipeRequest(int32 QueueIndex);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void RequestMoveQueueItem(FName RecipeID, int32 QueueIndex, bool bMoveUp);

protected:
	// Runtime
	// Properties and variables
	// Refs
	UPROPERTY(ReplicatedUsing = OnRep_InventoryUpdated, VisibleInstanceOnly, BlueprintReadWrite,
			  Category = "Crafting|Ref")
	TObjectPtr<UInventoryBase> InputInventory;

	UPROPERTY(ReplicatedUsing = OnRep_InventoryUpdated, VisibleInstanceOnly, BlueprintReadWrite,
			  Category = "Crafting|Ref")
	TObjectPtr<UInventoryBase> OutputInventory;

	UPROPERTY(ReplicatedUsing = OnRep_InventoryUpdated, VisibleInstanceOnly, BlueprintReadWrite,
			  Category = "Crafting|Ref")
	TObjectPtr<UInventoryBase> FuelInventory;

	UPROPERTY(ReplicatedUsing = OnRep_InventoryUpdated, VisibleInstanceOnly, BlueprintReadWrite,
			  Category = "Crafting|Ref")
	TObjectPtr<UInventoryBase> InteractorInventory;

	// Config
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting|Config")
	FCraftingInventoryOverrides Config;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_AutomaticCrafting, Category = "Crafting|Config")
	bool bAllowAutomaticCrafting = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting|Fuel",
		meta = (DisplayName = "Requires Fuel for Crafting"))
	bool bRequiresFuel = false;

	/** Items in the Fuel inventory must have this trait (child tags also match). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting|Fuel", meta = (EditCondition = "bRequiresFuel"))
	FGameplayTag FuelItemTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Config")
	TArray<FDataTableRowHandle> StartingRecipes;

	UPROPERTY(ReplicatedUsing = OnRep_AvailableRecipes, VisibleInstanceOnly, BlueprintReadOnly,
			  Category = "Crafting|Runtime")
	TArray<FItemRecipeRow> AvailableRecipes;

	UPROPERTY(ReplicatedUsing = OnRep_CachedRecipes, VisibleInstanceOnly, BlueprintReadOnly,
			  Category = "Crafting|Runtime")
	TArray<FCachedRecipeResult> CachedRecipeResults;

	UPROPERTY(ReplicatedUsing = OnRep_Blocks, VisibleInstanceOnly, BlueprintReadWrite, Category = "Crafting|Runtime")
	TArray<FBlockReasonData> ActiveBlocksReasons;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Crafting|Runtime")
	int32 OperatorCount = 0;

	UPROPERTY(Transient)
	TMap<FGuid, FCraftReservation> CraftReservations;

	UPROPERTY(Transient)
	TArray<FCraftItemBatch> PendingDeliveries;

	bool bCraftMutation = false;

	float DeliveryRetryElapsed = 0.f;

	TWeakObjectPtr<UInventoryBase> ObservedFuelInventory;

	FTimerHandle FuelStateRefreshTimer;

	bool bFuelStateRefreshScheduled = false;

	// Settings
	UPROPERTY(Category = "Invenza|Crafting", editAnywhere, BlueprintReadWrite)
	bool bDebugMode = true;

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	float CraftingSpeed = 1.0f;

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	float ProcessCraftTickTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	ECraftingResourceConsumePolicy ConsumePolicy = ECraftingResourceConsumePolicy::OnCraftStart;

	UPROPERTY()
	FTimerHandle CraftTimerHandle;

	UPROPERTY(ReplicatedUsing = OnRep_Queue, VisibleInstanceOnly, BlueprintReadOnly, Category = "Crafting")
	FRecipeQueueContainer RecipeQueue;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Crafting")
	TArray<FCraftAdditionalData> QueueAdditionalData;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentRecipe, VisibleInstanceOnly, BlueprintReadOnly, Category = "Crafting")
	FQueuedRecipe CurrentCraftingRecipe;

	UPROPERTY(Transient)
	float FuelAccumulatedTime = 0.f;

	UPROPERTY(Category = "Crafting|Fuel", EditAnywhere, BlueprintReadOnly,
		meta = (EditCondition = "bRequiresFuel", ClampMin = "0.001"))
	float FuelUnitsPerMinute = 1.f;

	UPROPERTY(Category = "Invenza|Crafting", VisibleInstanceOnly, BlueprintReadOnly)
	float SecondsPerFuelUnit = 60.f / FuelUnitsPerMinute;

	// Functions
	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable, Server, Reliable)
	void Server_EnqueueRecipe(FItemRecipeRow ItemRecipeRow, const TArray<int32>& SelectedOptions, int32 Count);

	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	void HandleEnqueueRecipe(FItemRecipeRow ItemRecipeRow, const TArray<int32>& SelectedOptions, int32 Count);

	UFUNCTION(Server, Reliable)
	void Server_CancelRecipe(int32 QueueIndex);

	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	void HandleCancelRecipe(int32 QueueIndex);

	UFUNCTION()
	void ProcessCraftTick();

	//
	void TryStartNext();

	void StartCurrentRecipe(FQueuedRecipe& Item);

	void FinishCurrentRecipe();

	bool ConsumeFuelUnit();

	bool IsFuelItem(UObject* Item) const;

	void BindFuelInventoryEvents();

	void UnbindFuelInventoryEvents();

	UFUNCTION()
	void RequestFuelStateRefresh();

	void RefreshFuelStateAfterInventoryChange();

	UFUNCTION()
	void HandleFuelItemAdded(FItemMapping& Mapping, UObject* Item);

	UFUNCTION()
	void HandleFuelItemRemoved(FItemMapping Mapping, UObject* Item);

	UFUNCTION()
	void HandleFuelQuantityChanged(UObject* Item);

	UFUNCTION()
	void HandleFuelItemReplaced(TArray<UInventorySlotData*> OldSlots, FItemMapping& Mapping, UObject* Item);

	bool ConsumeResourcesForRecipe(FQueuedRecipe& Item, int32 Count, FCraftAdditionalData& AddData);

	void RefundResourcesForRecipe(const FQueuedRecipe& Item, int32 Count, FCraftAdditionalData& AddData);

	TArray<UInventoryBase*> GetResourceInventories() const;

	bool PrepareCraftOutput(const FItemRecipeRow& Recipe, TArray<FCraftItemBatch>& OutBatches);

	void FlushPendingDeliveries();

	void CommitReservedIteration(FQueuedRecipe& Item);

	// --- Notification replication ---
	UFUNCTION()
	void OnRep_Queue();

	UFUNCTION()
	void OnRep_CurrentRecipe();

	UFUNCTION()
	void OnRep_InventoryUpdated();

	UFUNCTION()
	void OnRep_CachedRecipes();

	UFUNCTION()
	void OnRep_AvailableRecipes();

	UFUNCTION()
	void OnRep_Blocks();

	UFUNCTION()
	void OnRep_AutomaticCrafting();

	//
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnBlocksUpdated(const TArray<FBlockReasonData>& BlocksActive);

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

	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	void SaveCurrentProgressToQueue();

	void LogQueueState(const FString& Context) const;

	void RecalculateSortOrders();

	void UpdateOperatorBlockState();

	void UpdateFuelBlockState();

	UFUNCTION(Blueprintable)
	static bool IsResourceInventory(UInventoryBase* Inventory);

	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	static int32 CountItem(UInventoryBase* Inventory, FName ItemID);

	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	static FRecipeCheckResult CheckRecipe(const FItemRecipeRow& Recipe, const TArray<FItemIDEntry>& Items,
										  const TArray<int32>& SelectedOptions, int32 Amount);
};
