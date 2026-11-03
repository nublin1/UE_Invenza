// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "PortableInventorySubsystem.generated.h"

class APortableInventoryState;
class UInventoryBase;
class UCraftingComponent;

/** World-scoped portable identity registry and reverse crafting obligation index. */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UPortableInventorySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void Deinitialize() override;

public:
	// Functions
	bool RegisterState(const FGuid& ID, APortableInventoryState* State);
	void UnregisterState(const FGuid& ID, APortableInventoryState* State);
	APortableInventoryState* FindState(const FGuid& ID) const;
	void SetCraftObligations(UCraftingComponent* Craft, const TSet<TWeakObjectPtr<UInventoryBase>>& Inventories);
	bool HasCraftObligations(const UInventoryBase* Inventory) const;

protected:
	// Runtime
	// Properties and variables
	TMap<FGuid, TWeakObjectPtr<APortableInventoryState>> States;
	TMap<TWeakObjectPtr<UInventoryBase>, TSet<TWeakObjectPtr<UCraftingComponent>>> InventoryCrafts;
	TMap<TWeakObjectPtr<UCraftingComponent>, TSet<TWeakObjectPtr<UInventoryBase>>> CraftInventories;
};
