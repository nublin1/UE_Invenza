//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemFactory.generated.h"

class UItemBase;
struct FInventorySavedItem;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UItemFactory : public UObject
{
	GENERATED_BODY()

public:
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Item Creation")
	static UItemBase* CreateItemByHandle(UObject* Outer, FDataTableRowHandle Handle, int32 Quantity = 1);

	/** Native persistence path preserves subclasses and refuses to silently clamp a saved live stack. */
	static UItemBase* CreateItemFromSave(UObject* Outer, const FInventorySavedItem& Saved, bool bBatchSample = false);
};
