// Nublin Studio 2026 All Rights Reserved.

#include "Data/Inventory/Equipment/EquipmentComponent.h"

#include "GameFramework/Actor.h"
#include "UObject/ObjectPtr.h"
#include "Engine/World.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Data/Items/itemBase.h"
#include "Data/Inventory/Equipment/EquipmentSlotDefinition.h"


UEquipmentComponent::UEquipmentComponent(): SlotDefinitionTable(nullptr)
{
	
}

void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeSlotsFromTable();
}

void UEquipmentComponent::InitializeSlotsFromTable()
{
	if (!SlotDefinitionTable) return;

	for (auto& Row : SlotDefinitionTable->GetRowMap())
	{
		if (const FEquipmentSlotDefinition* SlotData = reinterpret_cast<FEquipmentSlotDefinition*>(Row.Value))
		{
			FEquipmentSlotData EquipmentSlotData;
			EquipmentSlotData.EquipmentSlotTag = SlotData->EquipmentSlotTag;
			EquipmentSlotData.AllowedCategory = SlotData->AllowedCategory;

			EquipmentSlots.Add(SlotData->EquipmentSlotTag, EquipmentSlotData);
		}
	}
}


bool UEquipmentComponent::EquipItem(UItemBase* Item)
{
	if (!Item) return false;

	for (auto& [Key, Val] : EquipmentSlots)
	{
		if (Val.EquippedItem == nullptr && Val.AllowedCategory == Item->GetItemRef().ItemCategory)
		{
			return EquipItemToSlot(Key, Item);
		}
	}

	return false;
}

bool UEquipmentComponent::EquipItemToSlot(FGameplayTag SlotTag, UItemBase* Item)
{	
	if (!Item || !SlotTag.IsValid()) return false;

	auto Slot = EquipmentSlots.Find(SlotTag);
	if (!Slot) return false;

	if (Slot->AllowedCategory != Item->GetItemRef().ItemCategory)
	{
		return false;
	}

	if (Slot->EquippedItem != nullptr)
	{
		return false;
	}

	Slot->EquippedItem = Item;

	Slot->EquippedItem->OnAmountChangedDelegate.AddDynamic(this, &UEquipmentComponent::ResourceAmountChanged);

	UE_LOG(LogTemp, Log, TEXT("EquipItem: Successfully equipped %s to %s"), *Item->GetName(), *SlotTag.ToString());

	OnEquippedItem.Broadcast(SlotTag, Item);

	return true;
}

void UEquipmentComponent::UnequipItemFromSlot(FGameplayTag SlotTag)
{
	if (!SlotTag.IsValid()) return;

	auto Slot = EquipmentSlots.Find(SlotTag);
	if (Slot == nullptr) return;

	UItemBase* RemovedItem = Slot->EquippedItem;
	if (!RemovedItem) return;
	
	RemovedItem->OnAmountChangedDelegate.RemoveDynamic(this, &UEquipmentComponent::ResourceAmountChanged);
	Slot->EquippedItem = nullptr;
	
	OnUnequippedItem.Broadcast(SlotTag, RemovedItem);
}

void UEquipmentComponent::ResourceAmountChanged(int32 AmountChanged, UItemBase* Item)
{
	if (!Item )
		return;

	for (auto& [Key, Val] : EquipmentSlots)
	{
		if (Val.EquippedItem == Item)
		{
			if (Val.EquippedItem->GetQuantity() <= 0)
			{
				UnequipItemFromSlot(Key);
				return;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("EquipmentInventory res not found"));
}
