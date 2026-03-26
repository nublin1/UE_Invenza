// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
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
	FName SlotName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory")
	FIntPoint CellPosition{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TObjectPtr<UInputAction> UseAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	EItemCategory AllowedCategory = EItemCategory::All;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FGameplayTag LinkedEquipmentSlot;

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
	bool IsEquipmentSlot() const { return LinkedEquipmentSlot.IsValid(); }
};
