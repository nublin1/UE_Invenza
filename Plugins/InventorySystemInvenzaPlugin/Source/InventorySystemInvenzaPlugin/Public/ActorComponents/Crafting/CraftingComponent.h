//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CraftingTypes.h"
#include "Components/ActorComponent.h"
#include "CraftingComponent.generated.h"


class USlotbasedInventory;
struct FItemIDEntry;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UCraftingComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewCraftStarted, FQueuedRecipe, QueuedRecipe);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftProgressChanged, float, ProgressRatio);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftQueueChanged,TArray<FQueuedRecipe>&,NewQueue);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftFinished, FName, RecipeID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftCanceled, FName, RecipeID);
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
	UPROPERTY(BlueprintAssignable, Category="Crafting|Events")
	FOnNewCraftStarted OnNewCraftStarted;
	UPROPERTY(BlueprintAssignable, Category="Crafting|Events")
	FOnCraftProgressChanged OnCraftProgressChanged;
	UPROPERTY(BlueprintAssignable, Category="Crafting|Events")
	FOnCraftQueueChanged OnCraftQueueChanged;
	UPROPERTY(BlueprintAssignable, Category="Crafting|Events")
	FOnCraftFinished OnCraftFinished;
	UPROPERTY(BlueprintAssignable, Category="Crafting|Events")
	FOnCraftCanceled OnCraftCanceled;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void InitStartingRecipes();
	UFUNCTION()
	FRecipeCheckResult CanCraft(const FItemRecipeRow& RecipeRow, int32 Amount = 1) const;
	UFUNCTION(BlueprintCallable, Category="Crafting")
	static FRecipeCheckResult CanCraftWithItems(const FItemRecipeRow& RecipeRow,
	                                            const TArray<FItemIDEntry>& InventoryItems,
	                                            int32 Amount = 1);
	
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void EnqueueRecipe(FItemRecipeRow ItemRecipeRow, int32 Count = 1);
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void CancelCurrentCraft();

protected:

	// Refs
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting|Ref")
	TObjectPtr<USlotbasedInventory> InputInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting|Ref")
	TObjectPtr<USlotbasedInventory> OutputInventory;
	
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
	TArray<FDataTableRowHandle> StartingRecipes;

	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Crafting")
	TArray<FItemRecipeRow> FoundRecipes;

	// Settings
	UPROPERTY(editAnywhere, BlueprintReadWrite)
	bool bDebugMode = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProcessCraftTickTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
	ECraftingResourceConsumePolicy ConsumePolicy =
		ECraftingResourceConsumePolicy::OnCraftStart;

	// data
	UPROPERTY()
	FTimerHandle CraftTimerHandle;

	// --- Production queue (replicated by everyone) ---
	UPROPERTY(ReplicatedUsing=OnRep_Queue, VisibleAnywhere, BlueprintReadOnly, Category="Crafting")
	TArray<FQueuedRecipe> RecipeQueue;
	
	UPROPERTY(ReplicatedUsing=OnRep_CurrentRecipe, VisibleAnywhere, BlueprintReadOnly, Category="Crafting")
	FQueuedRecipe CurrentCraftingRecipe;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(Server, Reliable)
	void Server_EnqueueRecipe(FItemRecipeRow ItemRecipeRow, int32 Count);
	UFUNCTION(Server, Reliable)
	void Server_CancelCurrentCraft();

	UFUNCTION()
	void ProcessCraftTick();

	//
	void TryStartNext();
	void StartCurrentRecipe(const FQueuedRecipe& Item);
	void FinishCurrentRecipe();
	bool ConsumeResourcesForRecipe(FName RecipeID, int32 Count);
	void RefundResourcesForRecipe(FName RecipeID, int32 Count);

	// --- Notification replication ---
	UFUNCTION()
	void OnRep_Queue();
	UFUNCTION()
	void OnRep_CurrentRecipe();

	//
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnCraftStarted();
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnCraftFinished(FName RecipeID);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnCraftCanceled(FName RecipeID);

	UFUNCTION()
	void CurrentRecipiteProgressChanged();
};
