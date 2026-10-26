// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryTypes.h"
#include "Data/ItemDataStructures.h"
#include "UObject/Object.h"
#include "InventorySlotData.generated.h"

class UItemBase;
class UInputAction;
/**
 *
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventorySlotData : public UObject
{
	GENERATED_BODY()

public:
	UInventorySlotData();

protected:
	// Unreal overrides
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Properties and variables
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_InventorySlotInfo,
			  Category = "Inventory")
	FInventorySlotInfo InventorySlotInfo;

	// Functions
	virtual bool IsSupportedForNetworking() const override { return true; }

	UFUNCTION()
	void OnRep_InventorySlotInfo();

	UFUNCTION(Blueprintable)
	static UInventorySlotData* Create(UObject* Outer);

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	static UInventorySlotData* CreateWithData(UObject* Outer, FInventorySlotInfo SlotData);

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	UInventorySlotData* DuplicateSlotData(UObject* Outer);

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	bool IsEquipmentSlot() const { return InventorySlotInfo.LinkedEquipmentSlot.IsValid(); }
};
