//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Pickupableass.generated.h"

class UObject;
// This class does not need to be modified.
UINTERFACE()
class UPickupableass : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYSYSTEMINVENZAPLUGIN_API IPickupableass
{
	GENERATED_BODY()

public:
	virtual UObject* GetItemData() = 0;
	
	virtual void OnPickedUp() = 0;
};
