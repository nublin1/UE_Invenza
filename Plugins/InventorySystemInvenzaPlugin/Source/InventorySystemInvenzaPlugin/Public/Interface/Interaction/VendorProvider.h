//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Trade/TradeTypes.h"

#include "VendorProvider.generated.h"

class UItemCollection;
struct FItemMoveData;
class UItemBase;
class UInventoryBase;

// This class does not need to be modified.
UINTERFACE()
class UVendorProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYSYSTEMINVENZAPLUGIN_API IVendorProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual const TObjectPtr<UInventoryBase>& GetVendorLootContainer() const = 0;

	virtual FTradeResult ProcessTradeRequest(const FItemMoveData& TradeData) = 0;

	virtual void SetTradePartnerInventory(UInventoryBase* InInventory) = 0;
	virtual void SetTradePartnerItemCollection(UItemCollection* InCollection) = 0;
};
