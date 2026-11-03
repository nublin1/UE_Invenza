// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "InventoryViewEntry.generated.h"

class UInventoryBase;
class UTexture2D;

/** Transient display data for one existing stack. Owned by one view, not by the inventory. */
UCLASS(BlueprintType, Transient)
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryViewEntry : public UObject
{
	GENERATED_BODY()

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEntryUpdated);
#pragma endregion Delegates

	// Properties and variables
	UPROPERTY(BlueprintAssignable, Category = "Inventory View")
	FOnEntryUpdated OnEntryUpdated;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory View")
	FText DisplayName;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory View")
	int32 Quantity = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory View")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory View")
	FGameplayTag Category;

	// Functions
	UFUNCTION(BlueprintPure, Category = "Inventory View")
	UObject* GetItem() const { return Item.Get(); }

	UFUNCTION(BlueprintPure, Category = "Inventory View")
	UInventoryBase* GetSourceInventory() const;

	void Initialize(UInventoryBase* Inventory, UObject* InItem);
	void RefreshDisplayData();

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Transient)
	TWeakObjectPtr<UInventoryBase> SourceInventory;

	UPROPERTY(Transient)
	TWeakObjectPtr<UObject> Item;
};
