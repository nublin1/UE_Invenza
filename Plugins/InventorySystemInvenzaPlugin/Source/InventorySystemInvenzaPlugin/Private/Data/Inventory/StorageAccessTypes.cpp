//  Nublin Studio 2026 All Rights Reserved.

#include "Data/Inventory/StorageAccessTypes.h"
#include "Data/Inventory/InventoryBase.h"

FText FStorageInventoryEntry::GetDisplayName() const
{
	if (!DisplayName.IsEmpty())
	{
		return DisplayName;
	}
	if (!IsValid(Inventory))
	{
		return FText::GetEmpty();
	}
	const FText Title = Inventory->GetInventorySettings().ContainerTitle;
	return Title.IsEmpty() ? FText::FromString(Inventory->GetInventoryContainerID()) : Title;
}
