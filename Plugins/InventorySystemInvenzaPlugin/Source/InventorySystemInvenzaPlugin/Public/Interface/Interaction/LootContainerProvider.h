//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "LootContainerProvider.generated.h"

class UInventoryBase;

// This class does not need to be modified.
UINTERFACE()
class ULootContainerProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYSYSTEMINVENZAPLUGIN_API ILootContainerProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual const TObjectPtr<UInventoryBase>& GetMainLootContainer() const = 0;
	virtual const TMap<FString, TObjectPtr<UInventoryBase>>& GetInventoriesToDisplay() const = 0;
};
