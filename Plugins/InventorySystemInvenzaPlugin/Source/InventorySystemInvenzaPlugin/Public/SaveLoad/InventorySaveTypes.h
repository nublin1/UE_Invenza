//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/Inventory/InventoryReplicationTypes.h"
#include "Data/CraftSystem/CraftingStructs.h"
#include "UObject/StrongObjectPtr.h"
#include "InventorySaveTypes.generated.h"

class UInventoryBase;
class UInventoryCollectionComponent;
class UCraftingComponent;

UENUM(BlueprintType)
enum class EInvenzaSaveView : uint8
{
	All,
	Save,
	Load
};

UENUM(BlueprintType)
enum class EInventoryClientTrust : uint8
{
	ServerValidated,
	TrustedClient
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventorySavedLocation
{
	GENERATED_BODY()
	UPROPERTY()
	FString InventoryKey;
	UPROPERTY()
	FIntPoint Cell = FIntPoint::ZeroValue;
	UPROPERTY()
	EItemOrientationType Orientation = EItemOrientationType::Horizontal;
	UPROPERTY()
	FGameplayTag EquipmentSlot;
	UPROPERTY()
	int32 ListOrder = 0;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventorySavedProperty
{
	GENERATED_BODY()
	UPROPERTY()
	FName Key;
	UPROPERTY()
	FString Value;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventorySavedItem
{
	GENERATED_BODY()
	UPROPERTY(Category = "Invenza|Save", BlueprintReadOnly)
	FGuid InstanceID;
	UPROPERTY()
	FSoftClassPath InstanceClass;
	UPROPERTY(Category = "Invenza|Save", BlueprintReadOnly)
	FDataTableRowHandle Definition;
	UPROPERTY()
	FSoftObjectPath DefinitionTable;
	UPROPERTY()
	FName DefinitionRow;
	UPROPERTY(Category = "Invenza|Save", BlueprintReadOnly)
	int32 Quantity = 0;
	UPROPERTY()
	TArray<FInventorySavedProperty> CustomState;
	UPROPERTY()
	TArray<FInventorySavedLocation> Locations;
	void SetCustomState(const TMap<FName, FString>& State)
	{
		CustomState.Reset();
		for (const auto& Pair : State)
		{
			auto& Value = CustomState.AddDefaulted_GetRef();
			Value.Key = Pair.Key;
			Value.Value = Pair.Value;
		}
	}
	TMap<FName, FString> GetCustomState() const
	{
		TMap<FName, FString> Result;
		for (const auto& Pair : CustomState)
		{
			Result.Add(Pair.Key, Pair.Value);
		}
		return Result;
	}
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventorySavedContainer
{
	GENERATED_BODY()
	UPROPERTY()
	FString Key;
	UPROPERTY()
	FIntPoint Size = FIntPoint(1, 1);
	UPROPERTY()
	FText Title;
	UPROPERTY()
	FInventorySettings StartupSettings;
};

USTRUCT()
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventorySavedCraftBatch
{
	GENERATED_BODY()
	UPROPERTY()
	FString OwnerKey;
	UPROPERTY()
	FString InventoryKey;
	UPROPERTY()
	FInventorySavedItem Sample;
	UPROPERTY()
	int32 Amount = 0;
};

USTRUCT()
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventorySavedCraftEntry
{
	GENERATED_BODY()
	UPROPERTY()
	FQueuedRecipe Recipe;
	UPROPERTY()
	TArray<int32> SelectedOptions;
	UPROPERTY()
	int32 PaidIterations = 0;
	UPROPERTY()
	TArray<FInitItemsEntry> UnitCost;
	UPROPERTY()
	TArray<FInventorySavedCraftBatch> Resources;
};

USTRUCT()
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventorySavedCraft
{
	GENERATED_BODY()
	UPROPERTY()
	FName ComponentName;
	UPROPERTY()
	TArray<FInventorySavedCraftEntry> Queue;
	UPROPERTY()
	FGuid CurrentEntry;
	UPROPERTY()
	TArray<FBlockReasonData> Blocks;
	UPROPERTY()
	TArray<FInventorySavedCraftBatch> PendingDeliveries;
	UPROPERTY()
	float FuelTime = 0.f;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventorySavedOwner
{
	GENERATED_BODY()
	UPROPERTY(Category = "Invenza|Save", BlueprintReadOnly)
	FString Key;
	/** Optional group configuration for linked storage owners; old saves leave it empty. */
	UPROPERTY()
	FSoftObjectPath LinkedStorageDefinition;
	UPROPERTY()
	FSoftClassPath ActorClass;
	UPROPERTY()
	FTransform Transform;
	UPROPERTY()
	FString LevelPackage;
	UPROPERTY()
	bool bRuntimeActor = false;
	UPROPERTY()
	bool bDestroyed = false;
	UPROPERTY()
	TArray<FInventorySavedContainer> Inventories;
	UPROPERTY()
	TArray<FInventorySavedItem> Items;
	/** Preserved verbatim when definitions or space are unavailable. Not additional live items. */
	UPROPERTY()
	TArray<FInventorySavedItem> RecoveryItems;
	UPROPERTY()
	TArray<FInventorySavedCraft> Crafting;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventorySaveSlotInfo
{
	GENERATED_BODY()
	UPROPERTY(Category = "Invenza|Save", BlueprintReadOnly)
	FString Slot;
	UPROPERTY(Category = "Invenza|Save", BlueprintReadOnly)
	FDateTime SavedAt;
	UPROPERTY(Category = "Invenza|Save", BlueprintReadOnly)
	int64 Revision = 0;
	UPROPERTY(Category = "Invenza|Save", BlueprintReadOnly)
	bool bAvailable = false;
};

USTRUCT()
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventorySavedPickup
{
	GENERATED_BODY()
	UPROPERTY()
	FString Key;
	UPROPERTY()
	FSoftClassPath ActorClass;
	UPROPERTY()
	FTransform Transform;
	UPROPERTY()
	bool bRuntimeActor = false;
	UPROPERTY()
	bool bDestroyed = false;
	UPROPERTY()
	FInventorySavedItem Item;
};

// Prepared data holds newly created UObjects alive until the complete world transaction commits.
struct FInventoryPreparedContainer
{
	UInventoryBase* Inventory = nullptr;
	FIntPoint Size = FIntPoint(1, 1);
	FText Title;
	TArray<UInventorySlotData*> Slots;
};

struct FInventoryPreparedCraft
{
	UCraftingComponent* Component = nullptr;
	TArray<FQueuedRecipe> Queue;
	TArray<FCraftAdditionalData> AdditionalData;
	TMap<FGuid, FCraftReservation> Reservations;
	TArray<FCraftItemBatch> Deliveries;
	TArray<FBlockReasonData> Blocks;
	FQueuedRecipe Current;
	float FuelTime = 0.f;
	TArray<TStrongObjectPtr<UObject>> CreatedObjects;
};

struct FInventoryPreparedOwner
{
	UInventoryCollectionComponent* Collection = nullptr;
	TArray<FInventoryEntry> Entries;
	TArray<FInventoryPreparedContainer> Containers;
	TArray<TStrongObjectPtr<UObject>> CreatedObjects;
	TArray<FInventorySavedItem> RecoveryItems;
	TArray<FInventoryPreparedCraft> Crafting;
};

struct FInventoryPreparedPickup
{
	class UPickupComponent* Component = nullptr;
	TStrongObjectPtr<UObject> Item;
};
