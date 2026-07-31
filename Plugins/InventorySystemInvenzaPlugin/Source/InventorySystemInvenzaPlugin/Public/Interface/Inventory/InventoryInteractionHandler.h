//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Inventory/InventoryTypes.h"
#include "UObject/Interface.h"
#include "InventoryInteractionHandler.generated.h"

struct FItemMoveData;
// This class does not need to be modified.
UINTERFACE()
class UInventoryInteractionHandler : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYSYSTEMINVENZAPLUGIN_API IInventoryInteractionHandler
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
	void OnQuickTransferItem(FItemMoveData InData);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
	void OnQuickTransferAllSameItems(FItemMoveData ItemMoveData);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
	void ItemContextMenuRequest(const FString& FromInventory, FGuid SlotGuid, UItemBase* Item);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
	void ItemTransferRequest(FItemMoveData ItemMoveData);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
	void ItemSplitRequest(UInventoryBase* TargetInventory, UItemBase* ItemToSplit, int32 SplitAmount);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
	void ItemDropRequest(FItemDropData DropData);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
	void RequestUseSlot(const FString& InvID, FGuid SlotID);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
	void RebuildInventoryRequest(const FString& InvID);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
	FInventoryModifierState GetInventoryModifierStates() const;
};
 