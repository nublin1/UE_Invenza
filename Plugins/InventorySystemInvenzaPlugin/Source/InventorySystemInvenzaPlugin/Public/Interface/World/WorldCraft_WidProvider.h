// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WorldCraft_WidProvider.generated.h"

class UCraftingComponent;

UINTERFACE(BlueprintType)
class UWorldCraft_WidProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYSYSTEMINVENZAPLUGIN_API IWorldCraft_WidProvider
{
	GENERATED_BODY()


public:
	virtual UCraftingComponent* GetCraftComponentPtr() = 0;
	virtual void SetCraftComponentPtr(UCraftingComponent* NewCraftingComponent) = 0;
	
};
