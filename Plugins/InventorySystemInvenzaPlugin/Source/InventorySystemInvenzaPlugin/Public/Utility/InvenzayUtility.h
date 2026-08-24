//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Inventory/InventoryTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InvenzayUtility.generated.h"

class UItemCollection;
struct FModalActionConfig;
enum class EObjectInteractionType : uint8;
struct FInitItemsEntry;
class IInventoryInteractionHandler;
class UInvenzaInventorySettingsAsset;
enum class EItemOrientationType : uint8;
class UInventoryBase;
class UObject;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzayUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	// Inventory
	static UInventoryBase* CreateStartupInventory(UObject* WorldContextObject, UItemCollection* ItemCollection,
		const FInventoryStartupData& StartupData,
	TMap<TObjectPtr<UInventoryBase>, FInitItemsList>& StartingItems);

	static void SetupStartingResources(UObject* WorldContextObject, TMap<TObjectPtr<UInventoryBase>, FInitItemsList>& StartingItems);

	UFUNCTION(BlueprintCallable, Category = "InventorySystemInvenza")
	static bool AddItemQuantity(UObject* Outer, UInventoryBase* TargetInventory, FInitItemsEntry InitItemsEntry );
	UFUNCTION(BlueprintCallable, Category = "InventorySystemInvenza")
	static bool AddItemQuantityBySample(UObject* Outer, UInventoryBase* TargetInventory, UObject* ItemSample, int32 TotalQuantity);

	// Items
	UFUNCTION(BlueprintCallable, Category = "InventorySystemInvenza")
	static bool bIsSameItems(UObject* FirstItem, UObject* SecondItem);
	UFUNCTION(BlueprintCallable, Category = "InventorySystemInvenza")
	static bool DoItemsHaveSameFootprint(UObject* FirstItem, UObject* SecondItem, EItemOrientationType OrientationFirstItem,
								  EItemOrientationType OrientationSecondItem, bool bIgnoreSize);
	
	UFUNCTION(BlueprintCallable, Category = "InventorySystemInvenza")
	static void DropItem(UWorld* World, AActor* OwnerActor,
		const FDataTableRowHandle& ItemRow, int32 AmountToDrop,	const FVector& SpawnLocation, const FRotator& SpawnRotation);

	UFUNCTION(BlueprintCallable, Category = "InventorySystemInvenza")
	static FVector2D CalculateItemVisualSize(UObject* Item, EItemOrientationType Orientation, FVector2D SlotSize, FMargin SlotSpacing, bool bIgnoreSize);
	
	//
	UFUNCTION(BlueprintCallable, Category = "InventorySystemInvenza")
	static TMap<EObjectInteractionType, FModalActionConfig> CollectAccessibleObjectActions(UWorld* World, UObject* InItem);

	//
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"),Category = "InventorySystemInvenza")
	static UInvenzaInventorySettingsAsset* GetInvenzaGlobalSettings( const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "InventorySystemInvenza")
	static TScriptInterface<IInventoryInteractionHandler> FindInventoryHandler(AActor* Actor);

protected:
	UFUNCTION(BlueprintPure, Category = "InventorySystemInvenza")
	static bool AddItemQuantityInternal(UInventoryBase* TargetInventory, UObject* ItemForDuplicate, int32 Remaining);
};
