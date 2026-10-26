//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Inventory/InventoryTypes.h"
#include "UObject/Object.h"
#include "InventorySimulator.generated.h"

class UObject;
class UInventoryCollectionComponent;
class UInventoryBase;

/**
 * Primarily intended for simulating complex operations such as trading
 * before applying them to the real inventory.
 *
 * The simulator duplicates the inventory and performs operations on the copy,
 * allowing validation of the entire sequence of actions.
 *
 * NOTE:
 * If you only need to check a single add operation, use:
 * UInventoryBase::HandleAddItem(..., bOnlyCheck = true)
 * instead of using the simulator.
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventorySimulator : public UObject
{
	GENERATED_BODY()

public:
	UInventorySimulator();

public:
	// Functions
	virtual bool IsSupportedForNetworking() const override { return true; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Simulation|Setup")
	void DuplicateInventoryForSimulation(UInventoryBase* InInventory);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Simulation")
	void TransferRequestSimulateQuantity(UObject* ItemSample, int32 ToatalQuantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Simulation")
	void TransferRequestSimulate(FItemMoveData ItemMoveData);

	UFUNCTION(BlueprintPure, Category = "Inventory|Simulation")
	UInventoryBase* GetSimulationInventory() { return SimulationInventory; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Simulation|History")
	const TArray<FInventorySimulationOperation>& GetOperationHistory() const { return OperationHistory; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Simulation|History")
	bool AreAllOperationsSuccessful() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Simulation|History")
	bool WasLastOperationSuccessful();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Simulation|History")
	void ClearHistory() { OperationHistory.Reset(); }

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Simulation")
	TObjectPtr<UInventoryBase> SourceInventory;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Simulation")
	TObjectPtr<UInventoryBase> SimulationInventory;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Simulation")
	TObjectPtr<UInventoryCollectionComponent> SimulationCollection;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Simulation|History")
	TArray<FInventorySimulationOperation> OperationHistory;
};
