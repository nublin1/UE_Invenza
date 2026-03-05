//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemFactory.generated.h"

class UItemBase;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UItemFactory : public UObject
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Item Creation")
	static UItemBase* CreateItemByID(UObject* Outer, UDataTable* InTable, FName ID, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Item Creation")
	static UItemBase* CreateItemByHandle(UObject* Outer, FDataTableRowHandle Handle, int32 Quantity = 1);
};
