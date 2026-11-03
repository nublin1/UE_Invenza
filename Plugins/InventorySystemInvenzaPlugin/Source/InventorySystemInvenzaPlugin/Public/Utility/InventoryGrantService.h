// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Data/Inventory/InventoryGrantTypes.h"

class AActor;
class UItemBase;
class UInventoryBase;

/** Server-only placement and overflow accounting. Does not equip or remove source mappings. */
class INVENTORYSYSTEMINVENZAPLUGIN_API FInventoryGrantService
{
public:
	// Functions
	static FInventoryGrantResult Grant(UItemBase* Item, const TArray<UInventoryBase*>& Destinations,
		AActor* Recipient, const FInventoryGrantPolicy& Policy);
};
