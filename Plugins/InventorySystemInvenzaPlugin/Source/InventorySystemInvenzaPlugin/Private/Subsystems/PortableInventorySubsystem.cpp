// Nublin Studio 2026 All Rights Reserved.

#include "Subsystems/PortableInventorySubsystem.h"
#include "ActorComponents/Crafting/CraftingComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "World/PortableInventoryState.h"

bool UPortableInventorySubsystem::RegisterState(const FGuid& ID, APortableInventoryState* State)
{
	if (!ID.IsValid() || !IsValid(State))
	{
		return false;
	}
	if (auto* Existing = FindState(ID); Existing && Existing != State)
	{
		UE_LOG(LogTemp, Error, TEXT("Duplicate portable inventory identity %s."), *ID.ToString());
		return false;
	}
	States.Add(ID, State);
	return true;
}

void UPortableInventorySubsystem::UnregisterState(const FGuid& ID, APortableInventoryState* State)
{
	if (States.FindRef(ID).Get() == State)
	{
		States.Remove(ID);
	}
}

APortableInventoryState* UPortableInventorySubsystem::FindState(const FGuid& ID) const
{
	auto* State = States.FindRef(ID).Get();
	return IsValid(State) && !State->IsActorBeingDestroyed() ? State : nullptr;
}

void UPortableInventorySubsystem::SetCraftObligations(UCraftingComponent* Craft,
	const TSet<TWeakObjectPtr<UInventoryBase>>& Inventories)
{
	if (const auto* Previous = CraftInventories.Find(Craft))
	{
		for (const auto& Inventory : *Previous)
		{
			if (auto* Crafts = InventoryCrafts.Find(Inventory))
			{
				Crafts->Remove(Craft);
				if (Crafts->IsEmpty())
				{
					InventoryCrafts.Remove(Inventory);
				}
			}
		}
	}
	CraftInventories.Remove(Craft);
	if (!Inventories.IsEmpty())
	{
		CraftInventories.Add(Craft, Inventories);
		for (const auto& Inventory : Inventories)
		{
			InventoryCrafts.FindOrAdd(Inventory).Add(Craft);
		}
	}
}

bool UPortableInventorySubsystem::HasCraftObligations(const UInventoryBase* Inventory) const
{
	const auto* Crafts = InventoryCrafts.Find(const_cast<UInventoryBase*>(Inventory));
	if (Crafts)
	{
		for (const auto& Craft : *Crafts)
		{
			if (Craft.IsValid() && Craft->HasInventoryObligations(Inventory))
			{
				return true;
			}
		}
	}
	return false;
}

void UPortableInventorySubsystem::Deinitialize()
{
	States.Empty();
	InventoryCrafts.Empty();
	CraftInventories.Empty();
	Super::Deinitialize();
}
