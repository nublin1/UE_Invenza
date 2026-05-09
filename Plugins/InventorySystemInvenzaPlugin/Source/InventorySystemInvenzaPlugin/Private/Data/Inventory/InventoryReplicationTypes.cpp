//  Nublin Studio 2026 All Rights Reserved.


#include "Data/Inventory/InventoryReplicationTypes.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/Inventory/InventoryBase.h"


void FInventoryEntry::PostReplicatedAdd(const FInventoryArray& InArraySerializer)
{
	if (InArraySerializer.OwningCollection)
	{
		for (const FItemMapping& Mapping : Locations.Mappings)
		{
			InArraySerializer.OwningCollection->NotifyUI_ItemChanged(Item, Mapping.InventoryID,
			                                                         EInventoryActionType::Added);
		}
	}
}

void FInventoryEntry::PostReplicatedChange(const FInventoryArray& InArraySerializer)
{
	if (InArraySerializer.OwningCollection)
	{
		for (const FItemMapping& Mapping : Locations.Mappings)
		{
			InArraySerializer.OwningCollection->NotifyUI_ItemChanged(Item, Mapping.InventoryID,
			                                                         EInventoryActionType::Updated);
		}
	}
}

void FInventoryEntry::PreReplicatedRemove(const FInventoryArray& InArraySerializer)
{
	if (InArraySerializer.OwningCollection)
	{
		for (const FItemMapping& Mapping : Locations.Mappings)
		{
			InArraySerializer.OwningCollection->NotifyUI_ItemChanged(Item, Mapping.InventoryID,
																	 EInventoryActionType::Removed);
		}
	}
}

void FInventoryArray::PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& Parameters)
{
	if (OwningCollection)
	{
		TSet<FString> ContainersToRedraw;

		for (UInventoryBase* Inv : OwningCollection->GetActorInventories())
		{
			if (Inv)
			{
				ContainersToRedraw.Add(Inv->GetInventoryContainerID());
			}
		}

		if (OwningCollection->GetLinkedInventories().ExternalInventory)
		{
			ContainersToRedraw.Add(OwningCollection->GetLinkedInventories().ExternalInventory->GetInventoryContainerID());
		}

		if (OwningCollection->GetLinkedInventories().VendorInventory)
		{
			ContainersToRedraw.Add(OwningCollection->GetLinkedInventories().VendorInventory->GetInventoryContainerID());
		}
		
		for (const FString& InvID : ContainersToRedraw)
		{
			OwningCollection->OnInventoryItemsChanged.Broadcast(InvID);
			OwningCollection->NotifyUI_ReDraw(InvID);
		}
	}
}
