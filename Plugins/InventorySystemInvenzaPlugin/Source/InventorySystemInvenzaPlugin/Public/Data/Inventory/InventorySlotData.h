// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryTypes.h"
#include "Data/ItemDataStructures.h"
#include "UObject/Object.h"
#include "InventorySlotData.generated.h"

enum class EItemCategory : uint8;
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
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	FInventorySlotInfo InventorySlotInfo;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(Blueprintable)
	static UInventorySlotData* Create(UObject* Outer);

	UFUNCTION(BlueprintCallable)
	static UInventorySlotData* CreateWithData(
		UObject* Outer,
		FName Name,
		FIntPoint Position,
		UInputAction* Action,
		EItemCategory Category = EItemCategory::All
	);

	UFUNCTION(BlueprintCallable)
	UInventorySlotData* DuplicateSlotData (UObject* Outer);

	UFUNCTION(BlueprintCallable)
	bool IsEquipmentSlot() const { return InventorySlotInfo.LinkedEquipmentSlot.IsValid(); }
};
