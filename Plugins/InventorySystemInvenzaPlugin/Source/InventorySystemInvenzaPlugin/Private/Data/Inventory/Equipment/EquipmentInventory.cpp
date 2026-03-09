// Nublin Studio 2025 All Rights Reserved.

#include "Data/Inventory/Equipment/EquipmentInventory.h"

#include "IDetailTreeNode.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectPtr.h"
#include "Engine/World.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Data/Items/itemBase.h"
#include "ActorComponents/SaveLoad/InvenzaSaveManager.h"
#include "Data/EquipmentStructures.h"
#include "Data/Inventory/Equipment/EquipmentSlotDefinition.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "UI/Inventory/EquipmentInventoryWidget.h"


UEquipmentInventory::UEquipmentInventory(): SlotDefinitionTable(nullptr)
{
	
}

void UEquipmentInventory::InitInventory(UItemCollection* ItemCollectionRef, FVector2D NewSize)
{
	InitializeSlotsFromTable();
	//BindWidgetsToSlots();
}

void UEquipmentInventory::InitializeSlotsFromTable()
{
	if (!SlotDefinitionTable) return;

	/*EquipmentSlots.Empty();
	for (auto& Row : SlotDefinitionTable->GetRowMap())
	{
		if (const FEquipmentSlotDefinition* SlotData = reinterpret_cast<FEquipmentSlotDefinition*>(Row.Value))
		{
			UEquipmentSlotData* NewSlot = NewObject<UEquipmentSlotData>();
			NewSlot->SlotName = SlotData->SlotName;
			NewSlot->AllowedCategory = SlotData->AllowedCategory;

			EquipmentSlots.Add(NewSlot->SlotName, NewSlot);
		}
	}*/
}

void UEquipmentInventory::BindWidgetsToSlots()
{
	/*auto InventoryManager = GetOwner()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager)
	{
		return;
	}

	auto SaveManager = GetOwner()->FindComponentByClass<UInvenzaSaveManager>();
	if (SaveManager)
	{
		SaveManager->OnGameLoaded.AddDynamic(
			this, &UEquipmentInventory::ValidateEquippedItems);
	}

	CharacterEquipmentWidget = InventoryManager->GetCoreHUDWidget()->GetEquipmentInvWidget();
	if (!CharacterEquipmentWidget)
	{
		return;
	}
	
	CharacterEquipmentWidget->GetInventoryFromContainerSlot()->OnItemReplaceDelegate.AddDynamic(
		this, &UEquipmentInventory::HandleReplaceItem);
	CharacterEquipmentWidget->GetInventoryFromContainerSlot()->OnAddItemDelegate.AddDynamic(
		this, &UEquipmentInventory::HandleItemEquippedFromMapping);
	CharacterEquipmentWidget->GetInventoryFromContainerSlot()->OnPreRemoveItemDelegate.AddDynamic(
		this, &UEquipmentInventory::HandleItemUnequippedFromMapping);*/
	
}

void UEquipmentInventory::HandleReplaceItem(TArray<UInventorySlotData*> OldItemSlots,
	TArray<UInventorySlotData*> NewItemSlots, UItemBase* Item)
{
	/*UnequipItemFromSlot(OldItemSlots, Item, Item->GetQuantity());
	EquipItemToSlot(NewItemSlots, Item);*/
}

void UEquipmentInventory::HandleItemEquippedFromMapping(FItemMapping ItemSlots, UItemBase* Item)
{
//	EquipItemToSlot(ItemSlots.OccupiedSlots, Item);
}

void UEquipmentInventory::EquipItemToSlot(FName SlotName, UItemBase* Item)
{
	/*// Widget name must match slot name
	if (!Item || SlotName.IsNone()) return;

	auto Slot =EquipmentSlots.Find(SlotName);
	if (Slot == nullptr) return;
	
	if (Slot->Get()->AllowedCategory != Item->GetItemRef().ItemCategory)
	{
		return;
	}

	if (Slot->Get()->ItemEquipped != nullptr)
	{
		return ;
	}

	Slot->Get()->ItemEquipped = Item;

	Slot->Get()->ItemEquipped->OnAmountChangedDelegate.AddDynamic(this, &UEquipmentInventory::ResourceAmountChanged);

	// Broadcast
	OnEquippedItem.Broadcast(SlotName, Item);

	// TODO: apply effects
	return;*/
}

FItemAddResult UEquipmentInventory::HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	if (!ItemMoveData.SourceItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item is null. Nothing to add"));
		return FItemAddResult::AddedNone(FText::FromString("Item is null. Nothing to add"));
	}

	if (ItemMoveData.SourceInventory && ItemMoveData.SourceItem->GetQuantity() <= 0)
		UE_LOG(LogTemp, Warning, TEXT("item Quantity is %i"), ItemMoveData.SourceItem->GetQuantity());

	if(ItemMoveData.SourceInventory
		&& ItemMoveData.SourceInventory != ItemMoveData.TargetInventory
		&& !ItemMoveData.SourceInventory->GetInventorySettings().bAllowItemReferencing 
		&& InventorySettings.bIsReferenceContainer )
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   0, ItemMoveData.SourceItem->GetItemRef().ItemTextData.NameID));
	}

	if (InventorySettings.bIsReferenceContainer )
		return HandleAddReferenceItem(ItemMoveData, bOnlyCheck);
	
	/*for (auto& [Key, Val] : EquipmentSlots)
	{
		if (Val->ItemEquipped == nullptr && Val->AllowedCategory == EDetailNodeType::Item->GetItemRef().ItemCategory)
		{
			Val->ItemEquipped = EDetailNodeType::Item;
			// TODO: Apply effect / logic

			Val->ItemEquipped->OnAmountChangedDelegate.AddDynamic(this, &UEquipmentInventory::ResourceAmountChanged);

			// Broadcast
			UE_LOG(LogTemp, Log, TEXT("EquipItem: Successfully equipped %s"), *Item->GetName());
			OnEquippedItem.Broadcast(Key, Item);
			return true;
		}
	}*/

	return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   0, ItemMoveData.SourceItem->GetItemRef().ItemTextData.NameID));
}

FItemAddResult UEquipmentInventory::HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	/*if (ItemMoveData.TargetSlot == nullptr)
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.NameID));

	if (bIsSlotEmpty(ItemMoveData.TargetSlot->GetSlotData()))
	{
		if (ItemMoveData.SourceInventory == this && ItemCollectionLinked->ItemHasInventory(ItemMoveData.SourceItem, InventoryContainerID))
		{
			if (!bOnlyCheck)
				ReplaceItem(ItemMoveData.SourceItem, ItemMoveData.TargetSlot->GetSlotData());
			return FItemAddResult::Swapped(0, true, FText::FromString("Item successfully moved to an empty slot."));
		}
		
		FItemMapping Slots;
		Slots.InventoryID = InventoryContainerID;
		Slots.OccupiedSlots.Add(ItemMoveData.TargetSlot->GetSlotData());
		if (!bOnlyCheck)
			AddNewItem(ItemMoveData, Slots, ItemMoveData.SourceItem->GetQuantity());

		TMap<UInventorySlotData*, FItemPlacementData> AffectedSlots;
		AffectedSlots.Add(Slots.OccupiedSlots[0], {1, ItemMoveData.SavedOrientation});

		return FItemAddResult::AddedAll(1, true, FText::Format(
			FText::FromString("Successfully added {0} to inventory as a reference"),
			ItemMoveData.SourceItem->GetItemRef().ItemTextData.NameID), AffectedSlots);
	}

	if (ItemMoveData.SourceInventory == this)
	{
		auto TarItem = ItemCollectionLinked->GetItemFromSlot(ItemMoveData.TargetSlot->GetSlotData(), InventoryContainerID);
		if (bOnlyCheck)
			return FItemAddResult::Swapped(0, true, FText::FromString("Items successfully swapped."));
		
		ReplaceItem(ItemMoveData.SourceItem, ItemMoveData.TargetSlot->GetSlotData());
		ReplaceItem(TarItem, ItemMoveData.SourceItemPivotSlot->GetSlotData());
		return FItemAddResult::Swapped(0, true, FText::FromString("Items successfully swapped."));
	}

	/*if (!bOnlyCheck)
	{
		HandleRemoveItemFromContainer(InventoryData.ItemCollectionLink->GetItemFromSlot(ItemMoveData.TargetSlot->GetSlotData(), GetAsContainerWidget()));

		FItemMapping Slots;
		Slots.OccupiedSlots.Add(ItemMoveData.TargetSlot->GetSlotData());
		AddNewItem(ItemMoveData, Slots, ItemMoveData.SourceItem->GetQuantity());

		return FItemAddResult::AddedAll(1, true, FText::Format(
			FText::FromString("Successfully added {0} to inventory as a reference"),
			ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}#1#

	TMap<UInventorySlotData*, FItemPlacementData> AffectedSlots;
	AffectedSlots.Add(ItemMoveData.TargetSlot->GetSlotData(), {1, ItemMoveData.SavedOrientation});*/
	
	/*return FItemAddResult::AddedAll(1, true, FText::Format(
			FText::FromString("Successfully added {0} to inventory as a reference"),
			ItemMoveData.SourceItem->GetItemRef().ItemTextData.NameID ), AffectedSlots);*/

	return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   0, ItemMoveData.SourceItem->GetItemRef().ItemTextData.NameID));
}


void UEquipmentInventory::UnequipItemFromSlot(FName SlotName, UItemBase* Item)
{
	/*if (!Item || SlotName.IsNone()) return;

	auto Slot =EquipmentSlots.Find(SlotName);
	if (Slot == nullptr) return;
	
	// TODO: remove effects
	UItemBase* RemovedItem = Slot->Get()->ItemEquipped;
	Slot->Get()->ItemEquipped = nullptr;

	Item->OnAmountChangedDelegate.RemoveDynamic(this, &UEquipmentInventory::ResourceAmountChanged);

	// Broadcast
	OnUnequippedItem.Broadcast(SlotName, RemovedItem);*/
}


void UEquipmentInventory::ResourceAmountChanged(int32 AmountChanged, UItemBase* Item)
{
	if (!Item )
		return;

	/*for (auto& [Key, Val] : EquipmentSlots)
	{
		if (Val->ItemEquipped == Item)
		{
			if (Val->ItemEquipped->GetQuantity() <= 0)
			{
				UnequipItemFromSlot(Key, Item);
				return;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("EquipmentInventory res not found"));*/
}
