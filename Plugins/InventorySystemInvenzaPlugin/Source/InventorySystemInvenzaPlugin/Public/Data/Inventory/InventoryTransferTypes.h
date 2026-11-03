// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Inventory/InventoryTypes.h"
#include "InventoryTransferTypes.generated.h"

UENUM(BlueprintType)
enum class EInventoryTakeAmountResult : uint8
{
	Success,
	InvalidRequest,
	AccessDenied,
	SourceChanged,
	AmountReserved,
	NoCapacity,
	UnsupportedItem,
	Busy
};

/** The target inventory is resolved by the server from the requesting manager's owner. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventoryTakeAmountRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Take Amount")
	FGuid RequestID;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Take Amount")
	FItemMoveData Move;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Take Amount")
	int32 ExpectedSourceQuantity = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Take Amount")
	int32 ExpectedAvailableAmount = 0;
};
