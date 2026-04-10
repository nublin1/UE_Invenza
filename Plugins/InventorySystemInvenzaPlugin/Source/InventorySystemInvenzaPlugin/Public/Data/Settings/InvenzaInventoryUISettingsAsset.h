//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InvenzaInventoryUISettingsAsset.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaInventoryUISettingsAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> PickupClass;

	UPROPERTY(EditDefaultsOnly)
	FDataTableRowHandle CurrencyItemClass;
};
