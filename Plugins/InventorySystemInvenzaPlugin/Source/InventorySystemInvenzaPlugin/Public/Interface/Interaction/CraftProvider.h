//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CraftProvider.generated.h"

class UCraftingComponent;
// This class does not need to be modified.
UINTERFACE()
class UCraftProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYSYSTEMINVENZAPLUGIN_API ICraftProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UCraftingComponent* GetCraftingComponent() const = 0;
};
