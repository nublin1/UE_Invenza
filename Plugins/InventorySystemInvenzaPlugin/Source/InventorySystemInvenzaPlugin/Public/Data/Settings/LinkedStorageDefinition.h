//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "Data/Inventory/InventoryTypes.h"
#include "LinkedStorageDefinition.generated.h"

/** One definition asset per group, shared by every chest in that group. */
UCLASS(BlueprintType)
class INVENTORYSYSTEMINVENZAPLUGIN_API ULinkedStorageDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Linked Storage")
	FName StorageGroupID;

	/** Authoritative capacity, restrictions, widgets and one-time initial contents. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Linked Storage")
	FInventoryStartupData Inventory;
};
