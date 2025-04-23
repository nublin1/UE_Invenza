// Nublin Studio 2025 All Rights Reserved.


#include "ActorComponents/EquipmentManager/EquipmentManagerComponent.h"

#include "ActorComponents/Items/itemBase.h"
#include "Data/EquipmentSlotData.h"
#include "Data/EquipmentStructures.h"


UEquipmentManagerComponent::UEquipmentManagerComponent(): SlotDefinitionTable(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEquipmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
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

bool UEquipmentManagerComponent::EquipItemToSlot(FName SlotName, UItemBase* Item)
{
	if (!Item || !EquipmentSlots.Contains(SlotName)) return false;

	FEquipmentSlot& Slot = EquipmentSlots[SlotName];
	if (Slot.AllowedCategory != Item->GetItemRef().ItemCategory)
	{
		return false;
	}

	if (Slot.EquippedItem != nullptr)
	{
		return false;
	}

	Slot.EquippedItem = Item;

	// Broadcast
	OnEquippedItem.Broadcast(SlotName, Item);

	// TODO: Remove from inventory, apply effects
	return true;
}

bool UEquipmentManagerComponent::EquipItem(UItemBase* Item)
{
	if (!Item) return false;
	for (auto& Elem : EquipmentSlots)
	{
		FEquipmentSlot& Slot = Elem.Value;

		if (Slot.EquippedItem == nullptr && Slot.AllowedCategory == Item->GetItemRef().ItemCategory)
		{
			Slot.EquippedItem = Item;
			// TODO: Remove from ItemCollection
			// TODO: Apply effect / logic

			// Broadcast
			OnEquippedItem.Broadcast(Slot.SlotName, Item);
			return true;
		}
	}

	return false; // No suitable slot found
}

bool UEquipmentManagerComponent::UnequipItem(FName SlotName)
{
	if (!EquipmentSlots.Contains(SlotName)) return false;

	FEquipmentSlot& Slot = EquipmentSlots[SlotName];

	if (Slot.EquippedItem == nullptr)
	{
		return false;
	}

	// TODO: Add item back to inventory, remove effects
	UItemBase* RemovedItem = Slot.EquippedItem;
	Slot.EquippedItem = nullptr;

	// Broadcast
	OnUnequippedItem.Broadcast(SlotName, nullptr);

	return true;
}

void UEquipmentManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

