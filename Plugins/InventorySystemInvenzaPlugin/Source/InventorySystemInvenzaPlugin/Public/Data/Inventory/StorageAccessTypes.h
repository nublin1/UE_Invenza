//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StorageAccessTypes.generated.h"

class UInventoryBase;
class UStorageAccessComponent;

/** Original relevance state shared by concurrent storage access sessions. */
struct FStorageRelevanceLease
{
	int32 Count = 0;
	bool bAlwaysRelevant = false;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FStorageInventoryEntry
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage")
	TObjectPtr<UInventoryBase> Inventory = nullptr;
	/** Empty uses ContainerTitle, then the inventory ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage")
	FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage")
	int32 SortOrder = 0;
	FText GetDisplayName() const;
};

USTRUCT()
struct INVENTORYSYSTEMINVENZAPLUGIN_API FStorageAccessSession
{
	GENERATED_BODY()
	UPROPERTY()
	TObjectPtr<UStorageAccessComponent> AccessPoint = nullptr;
	UPROPERTY()
	TArray<FStorageInventoryEntry> Inventories;
	UPROPERTY()
	uint32 Revision = 0;
};

UENUM(BlueprintType)
enum class EStorageDiscoveryMode : uint8
{
	Radius,
	Explicit,
	RadiusAndExplicit
};
