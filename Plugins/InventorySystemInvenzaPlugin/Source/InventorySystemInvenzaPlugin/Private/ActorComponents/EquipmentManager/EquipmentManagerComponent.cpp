// Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/EquipmentManager/EquipmentManagerComponent.h"

#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/Items/itemBase.h"
#include "Data/EquipmentSlotData.h"
#include "Data/EquipmentStructures.h"
#include "UI/Inventory/EquipmentInventoryWidget.h"
#include "UI/Inventory/EquipmentSlotWidget.h"

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

void UEquipmentManagerComponent::InitializeSlotsFromTable()
{
	if (!SlotDefinitionTable) return;

	EquipmentSlots.Empty();

	static const FString Context = TEXT("EquipmentSlotInit");
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

	auto EquipmentInvWidget = InventoryManager->GetCoreHUDWidget()->GetEquipmentInvWidget();
	if (!EquipmentInvWidget)
	{
		return;
	}

	EquipmentInvWidget->GetInventoryFromContainerSlot()->OnAddItemDelegate.AddDynamic(
		this, &UEquipmentManagerComponent::EquipItemToSlot);
	EquipmentInvWidget->GetInventoryFromContainerSlot()->OnPreRemoveItemDelegate.AddDynamic(
		this, &UEquipmentManagerComponent::UnequipItemFromSlot);
	
}

void UEquipmentManagerComponent::EquipItemToSlot(FItemMapping ItemSlots, UItemBase* Item)
{
	// Widget name must match slot name
	auto SlotName = ItemSlots.ItemSlotDatas[0].SlotName;
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

void UEquipmentManagerComponent::UnequipItemFromSlot(FItemMapping ItemSlots, UItemBase* Item, int32 RemoveQuantity)
{
	auto SlotName = ItemSlots.ItemSlotDatas[0].SlotName;
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

