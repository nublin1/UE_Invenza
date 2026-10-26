//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InventoryListEntry.generated.h"

class UListInventoryWidget;
class UObject;
/**
 * 
 */
UCLASS(Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryListEntry : public UObject
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Properties and variables
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	FGuid SlotGuid;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Inventory")
	TObjectPtr<UObject> Item = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	TObjectPtr<UListInventoryWidget> ParentInventoryWidget = nullptr;

	// Functions
	virtual bool IsSupportedForNetworking() const override { return true; }
};
