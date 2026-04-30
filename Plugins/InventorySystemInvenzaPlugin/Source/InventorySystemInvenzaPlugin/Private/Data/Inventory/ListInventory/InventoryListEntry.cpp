//  Nublin Studio 2026 All Rights Reserved.


#include "Data/Inventory/ListInventory/InventoryListEntry.h"

#include "Net/UnrealNetwork.h"

void UInventoryListEntry::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryListEntry, Item);
}
