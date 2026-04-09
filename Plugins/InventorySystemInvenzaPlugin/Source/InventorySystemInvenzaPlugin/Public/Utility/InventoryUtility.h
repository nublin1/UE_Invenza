//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryUtility.generated.h"

class UInventoryBase;
class UItemBase;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "InventorySystemInvenza")
	static bool AddItemQuantity(UObject* Outer, UInventoryBase* TargetInventory, UItemBase* ItemSample, int32 TotalQuantity);
};
