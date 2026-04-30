//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InventoryListEntry.generated.h"

class UListInventoryWidget;
class UItemBase;
/**
 * 
 */
UCLASS(Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryListEntry : public UObject
{
	GENERATED_BODY()
	
public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(BlueprintReadWrite, Replicated, Category="Inventory")
	TObjectPtr<UItemBase> Item = nullptr;

	UPROPERTY(BlueprintReadWrite, Category="Inventory")
	TObjectPtr<UListInventoryWidget> ParentInventoryWidget = nullptr;
};
