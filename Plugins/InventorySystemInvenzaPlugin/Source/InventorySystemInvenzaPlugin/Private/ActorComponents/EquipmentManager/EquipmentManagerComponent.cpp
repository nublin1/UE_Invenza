// Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/EquipmentManager/EquipmentManagerComponent.h"

#include "ActorComponents/ItemCollection.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectPtr.h"
#include "Engine/World.h"
#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/Items/itemBase.h"
#include "ActorComponents/SaveLoad/InvenzaSaveManager.h"
#include "Data/EquipmentSlotData.h"
#include "Data/EquipmentStructures.h"
#include "UI/Inventory/EquipmentInventoryWidget.h"


UEquipmentManagerComponent::UEquipmentManagerComponent(): SlotDefinitionTable(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEquipmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UEquipmentManagerComponent::Initialize()
{
	InitializeSlotsFromTable();
	BindWidgetsToSlots();
}

void UEquipmentManagerComponent::ValidateEquippedItems()
{
	if(!CharacterEquipmentWidget)
	{
		return;
	}

	auto ItemCollection = GetOwner()->FindComponentByClass<UItemCollection>();
	if(!ItemCollection)
		return;

	auto ItemsInCollection = ItemCollection->GetAllItemsByContainer(CharacterEquipmentWidget);

	if (EquipmentSlots.IsEmpty())
		return;
		
	for (auto& Pair : EquipmentSlots)
	{
		Pair.Value.EquippedItem = nullptr;
	}

	for (auto& Item : ItemsInCollection)
	{
		auto Mapping = ItemCollection->FindItemMappingForItemInContainer(Item, CharacterEquipmentWidget);
		if (!Mapping)
			continue;

		EquipItemToSlot(Mapping->ItemSlotDatas, Item);
	}
}

void UEquipmentManagerComponent::InitializeSlotsFromTable()
{
	if (!SlotDefinitionTable) return;

	EquipmentSlots.Empty();
	for (auto& Row : SlotDefinitionTable->GetRowMap())
	{
		if (const FEquipmentSlotData* SlotData = reinterpret_cast<FEquipmentSlotData*>(Row.Value))
		{
			FEquipmentSlot NewSlot;
			NewSlot.SlotName = SlotData->SlotName;
			NewSlot.AllowedCategory = SlotData->AllowedCategory;

			EquipmentSlots.Add(SlotData->SlotName, NewSlot);
		}
	}
}

void UEquipmentManagerComponent::BindWidgetsToSlots()
{
	auto InventoryManager = GetOwner()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager)
	{
		return;
	}

	auto SaveManager = GetOwner()->FindComponentByClass<UInvenzaSaveManager>();
	if (SaveManager)
	{
		SaveManager->OnGameLoaded.AddDynamic(
			this, &UEquipmentManagerComponent::ValidateEquippedItems);
	}

	CharacterEquipmentWidget = InventoryManager->GetCoreHUDWidget()->GetEquipmentInvWidget();
	if (!CharacterEquipmentWidget)
	{
		return;
	}
	
	CharacterEquipmentWidget->GetInventoryFromContainerSlot()->OnItemReplaceDelegate.AddDynamic(
		this, &UEquipmentManagerComponent::HandleReplaceItem);
	CharacterEquipmentWidget->GetInventoryFromContainerSlot()->OnAddItemDelegate.AddDynamic(
		this, &UEquipmentManagerComponent::HandleItemEquippedFromMapping);
	CharacterEquipmentWidget->GetInventoryFromContainerSlot()->OnPreRemoveItemDelegate.AddDynamic(
		this, &UEquipmentManagerComponent::HandleItemUnequippedFromMapping);
	
}

void UEquipmentManagerComponent::HandleReplaceItem(TArray<FInventorySlotData> OldItemSlots,
	TArray<FInventorySlotData> NewItemSlots, UItemBase* Item)
{
	UnequipItemFromSlot(OldItemSlots, Item, Item->GetQuantity());
	EquipItemToSlot(NewItemSlots, Item);
}

void UEquipmentManagerComponent::HandleItemEquippedFromMapping(FItemMapping ItemSlots, UItemBase* Item)
{
	EquipItemToSlot(ItemSlots.ItemSlotDatas, Item);
}

void UEquipmentManagerComponent::EquipItemToSlot(TArray<FInventorySlotData>& ItemSlotsData, UItemBase* Item)
{
	// Widget name must match slot name
	auto SlotName = ItemSlotsData[0].SlotName;
	if (!Item || SlotName.IsNone() || !EquipmentSlots.Contains(SlotName)) return;

	FEquipmentSlot& EquipmentSlot = EquipmentSlots[SlotName];
	if (EquipmentSlot.AllowedCategory != Item->GetItemRef().ItemCategory)
	{
		return;
	}

	if (EquipmentSlot.EquippedItem != nullptr)
	{
		return ;
	}

	EquipmentSlot.EquippedItem = Item;

	// Broadcast
	OnEquippedItem.Broadcast(SlotName, Item);

	// TODO: apply effects
	return;
}

void UEquipmentManagerComponent::EquipItem(UItemBase* Item)
{
	if (!Item) return;
	for (auto& Elem : EquipmentSlots)
	{
		FEquipmentSlot& Slot = Elem.Value;

		if (Slot.EquippedItem == nullptr && Slot.AllowedCategory == Item->GetItemRef().ItemCategory)
		{
			Slot.EquippedItem = Item;
			// TODO: Apply effect / logic

			// Broadcast
			OnEquippedItem.Broadcast(Slot.SlotName, Item);
			return;
		}
	}

	return; // No suitable slot found
}

void UEquipmentManagerComponent::HandleItemUnequippedFromMapping(FItemMapping ItemSlots, UItemBase* Item, int32 RemoveQuantity)
{
	UnequipItemFromSlot(ItemSlots.ItemSlotDatas, Item, RemoveQuantity);
}

void UEquipmentManagerComponent::UnequipItemFromSlot(TArray<FInventorySlotData>& ItemSlotsData, UItemBase* Item, int32 RemoveQuantity)
{
	auto SlotName = ItemSlotsData[0].SlotName;
	if (SlotName.IsNone() || !EquipmentSlots.Contains(SlotName)) return;

	FEquipmentSlot& Slot = EquipmentSlots[SlotName];
	if (Slot.EquippedItem == nullptr)
	{
		return;
	}

	if (Item->GetQuantity() >= RemoveQuantity)
	{
		// TODO: remove effects
		UItemBase* RemovedItem = Slot.EquippedItem;
		Slot.EquippedItem = nullptr;

		// Broadcast
		OnUnequippedItem.Broadcast(SlotName, nullptr);
		return;
	}
}

TMap<UItemBase*, FEquipmentSlot> UEquipmentManagerComponent::GetEquippedItemsData()
{
	TMap<UItemBase*, FEquipmentSlot> EquipmentItems;
	if (EquipmentSlots.Num() == 0)
	{
		return EquipmentItems;
	}

	for (const auto& EquipmentSlot : EquipmentSlots)
	{
		if (EquipmentSlot.Value.EquippedItem)
		{
			EquipmentItems.Add(EquipmentSlot.Value.EquippedItem, EquipmentSlot.Value);
		}
	}

	return EquipmentItems;
}

void UEquipmentManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

