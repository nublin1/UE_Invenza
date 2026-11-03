// Nublin Studio 2026 All Rights Reserved.

#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"

#include "ActorComponents/InventoryCollectionComponent.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Items/itemBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "World/PortableInventoryState.h"

bool USlotbasedInventory::TryResizeInventory(FIntPoint NewSize)
{
	return ReconfigureGrid(NewSize, false);
}

bool USlotbasedInventory::ReconfigureGrid(FIntPoint NewSize, bool bSortByName)
{
	if (!InventoryOwnerActor || !InventoryOwnerActor->HasAuthority() || !ItemCollectionLinked ||
		InventorySettings.bCollectInvDataFromWidget || !GetSlotsWithLinkedEquipment().IsEmpty() ||
		NewSize.X < 1 || NewSize.Y < 1 || NewSize.X > 256 || NewSize.Y > 256)
	{
		return false;
	}
	if (const auto* State = Cast<APortableInventoryState>(InventoryOwnerActor); State && State->HasCraftObligations())
	{
		return false;
	}
	const auto* Settings = UInvenzaInventorySettingsSubsystem::GetSettingsStatic(this);
	if (!Settings)
	{
		return false;
	}
	TArray<UInventorySlotData*> Slots;
	for (int32 Row = 0; Row < NewSize.X; ++Row)
	{
		for (int32 Column = 0; Column < NewSize.Y; ++Column)
		{
			auto* Slot = GetSlotByPosition(FIntPoint(Row, Column));
			if (!Slot)
			{
				FInventorySlotInfo Info;
				Info.CellPosition = FIntPoint(Row, Column);
				Info.AllowedCategory = Settings->AnyCategoryGameplayTag;
				Slot = UInventorySlotData::CreateWithData(InventoryOwnerActor, Info);
			}
			Slots.Add(Slot);
		}
	}
	TArray<UObject*> Items = ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);
	if (bSortByName)
	{
		Items.StableSort([](const UObject& A, const UObject& B)
		{
			return IObjectDataProvider::Execute_GetItemDisplayText(&A).ToString().Compare(
				IObjectDataProvider::Execute_GetItemDisplayText(&B).ToString(), ESearchCase::IgnoreCase) < 0;
		});
	}
	TSet<UInventorySlotData*> Occupied;
	TMap<UObject*, FItemMapping> Placements;
	for (auto* Item : Items)
	{
		const auto* Old = ItemCollectionLinked->FindItemMappingByContainerName(Item, InventoryContainerID);
		if (!Old || Old->OccupiedSlots.IsEmpty())
		{
			return false;
		}
		FItemMapping Planned = *Old;
		auto Fit = [&](FIntPoint Pivot, EItemOrientationType Orientation)
		{
			const FIntPoint Footprint = InventorySettings.InventorySlotBasedSettings.bIgnoreItemSize ? FIntPoint(1, 1) :
				IObjectDataProvider::Execute_GetItemSize(Item, Orientation);
			if (Footprint.X < 1 || Footprint.Y < 1 || Pivot.X < 0 || Pivot.Y < 0 ||
				Pivot.X + Footprint.Y > NewSize.X || Pivot.Y + Footprint.X > NewSize.Y)
			{
				return false;
			}
			TArray<UInventorySlotData*> Cells;
			for (int32 Row = 0; Row < Footprint.Y; ++Row)
			{
				for (int32 Column = 0; Column < Footprint.X; ++Column)
				{
					auto* Cell = Slots[(Pivot.X + Row) * NewSize.Y + Pivot.Y + Column];
					if (Occupied.Contains(Cell) || !IObjectDataProvider::Execute_GetItemRef(Item).ItemCategory.MatchesTag(
						Cell->InventorySlotInfo.AllowedCategory))
					{
						return false;
					}
					Cells.Add(Cell);
				}
			}
			Planned.OccupiedSlots = Cells;
			Planned.ItemOrientation = Orientation;
			return true;
		};
		bool bPlaced = !bSortByName && Fit(Old->OccupiedSlots[0]->InventorySlotInfo.CellPosition, Old->ItemOrientation);
		for (auto* Slot : Slots)
		{
			if (bPlaced)
			{
				break;
			}
			bPlaced = Fit(Slot->InventorySlotInfo.CellPosition, Old->ItemOrientation);
		}
		if (!bPlaced)
		{
			return false;
		}
		Occupied.Append(Planned.OccupiedSlots);
		Placements.Add(Item, Planned);
	}
	InventorySlotData = Slots;
	InvSize = NewSize;
	for (const auto& Pair : Placements)
	{
		ItemCollectionLinked->UpdateItemMapping(Pair.Key, InventoryContainerID, Pair.Value.OccupiedSlots,
			Pair.Value.ItemOrientation);
	}
	OnRep_InventorySlotData();
	OnRep_InventoryConfiguration();
	NotifyReDrawRequest();
	ItemCollectionLinked->OnInventoryItemsChanged.Broadcast(InventoryContainerID);
	return true;
}
