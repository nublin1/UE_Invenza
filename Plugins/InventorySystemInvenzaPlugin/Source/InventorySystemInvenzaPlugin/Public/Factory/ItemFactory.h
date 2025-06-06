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
	static UDataTable* ItemTable;

	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	static void Init(UDataTable* InTable);

	UFUNCTION(BlueprintCallable, Category = "Item Creation")
	static UItemBase* CreateItemByID(UObject* Outer, FName ID, int32 Quantity = 1);
};
