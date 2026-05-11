// Nublin Studio 2026 All Rights Reserved.

#include "Data/Inventory/Equipment/EquipmentComponent.h"

#include "GameFramework/Actor.h"
#include "UObject/ObjectPtr.h"
#include "Engine/World.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Data/Items/itemBase.h"
#include "Data/Inventory/Equipment/EquipmentSlotDefinition.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"


UEquipmentComponent::UEquipmentComponent()
{
	SetIsReplicatedByDefault(true);
}

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEquipmentComponent, EquipmentSlotsArray);
}

bool UEquipmentComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
	struct FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	for (const FEquipmentSlotRuntime& Slot : EquipmentSlotsArray)
	{
		if (IsValid(Slot.EquippedItem))
		{
			bWroteSomething |= Channel->ReplicateSubobject(Slot.EquippedItem, *Bunch, *RepFlags);
		}
	}

	return bWroteSomething;
}

void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GetOwnerRole() == ROLE_Authority)
	{
		InitializeSlotsFromTable();
	}
}

void UEquipmentComponent::InitializeSlotsFromTable()
{
	for (const FDataTableRowHandle& RowHandle : SlotRows)
	{
		const FEquipmentSlotDefinition* Row = RowHandle.GetRow<FEquipmentSlotDefinition>(TEXT(""));
		if (!Row) continue;

		FEquipmentSlotRuntime SlotRuntime;
		SlotRuntime.SlotTag = Row->SlotTag;
		SlotRuntime.AllowedCategory = Row->AllowedCategory;
        
		EquipmentSlotsArray.Add(SlotRuntime);
	}
}

void UEquipmentComponent::Server_EquipItem_Implementation(UItemBase* Item)
{
	if (!Item) return;

	for (FEquipmentSlotRuntime& Slot : EquipmentSlotsArray)
	{
		if (Slot.EquippedItem == nullptr && Slot.AllowedCategory == Item->GetItemRef().ItemCategory)
		{
			Server_EquipItemToSlot(Slot.SlotTag, Item);
			return;
		}
	}
}

void UEquipmentComponent::Server_EquipItemToSlot_Implementation(FGameplayTag SlotTag, UItemBase* Item)
{
	if (!Item || !SlotTag.IsValid()) return;

	FEquipmentSlotRuntime* Slot = FindSlot(SlotTag);
	if (!Slot || Slot->EquippedItem || Slot->AllowedCategory != Item->GetItemRef().ItemCategory) return;

	Slot->EquippedItem = Item;
	Slot->EquippedItem->OnAmountChangedDelegate.AddDynamic(this, &UEquipmentComponent::ResourceAmountChanged);
	
	OnEquippedItem.Broadcast(SlotTag, Item);
	
	/*if (GetNetMode() != NM_DedicatedServer)
	{
		
	}*/
}

void UEquipmentComponent::Server_UnequipItemFromSlot_Implementation(FGameplayTag SlotTag)
{
	FEquipmentSlotRuntime* Slot = FindSlot(SlotTag);
	if (!Slot || !Slot->EquippedItem) return;

	UItemBase* RemovedItem = Slot->EquippedItem;
	RemovedItem->OnAmountChangedDelegate.RemoveDynamic(this, &UEquipmentComponent::ResourceAmountChanged);
    
	Slot->EquippedItem = nullptr;
    
	OnUnequippedItem.Broadcast(SlotTag, RemovedItem);
}

void UEquipmentComponent::OnRep_EquipmentSlots()
{
}

void UEquipmentComponent::ResourceAmountChanged(int32 AmountChanged, UItemBase* Item)
{
	if (!GetOwner()->HasAuthority() || !Item) return;

	for (FEquipmentSlotRuntime& Slot : EquipmentSlotsArray)
	{
		if (Slot.EquippedItem == Item && Item->GetQuantity() <= 0)
		{
			Server_UnequipItemFromSlot(Slot.SlotTag);
			return;
		}
	}
}

FEquipmentSlotRuntime* UEquipmentComponent::FindSlot(FGameplayTag SlotTag)
{
	return EquipmentSlotsArray.FindByPredicate([&](const FEquipmentSlotRuntime& Slot) {
		return Slot.SlotTag == SlotTag;
	});
}
