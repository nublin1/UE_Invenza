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
#include "Utility/InterfaceUtils.h"


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

bool UEquipmentComponent::HasSlotForCategory(const FGameplayTag& AllowedCategory) const
{
	if (!AllowedCategory.IsValid())
	{
		return false;
	}

	for (const FEquipmentSlotRuntime& Slot : EquipmentSlotsArray)
	{
		if (Slot.AllowedCategory.IsValid()
			&& Slot.AllowedCategory.MatchesTagExact(AllowedCategory))
		{
			return true;
		}
	}

	return false;
}

bool UEquipmentComponent::IsCategoryCompatibleWithSlot(FGameplayTag SlotTag, FGameplayTag ItemCategory) const
{
	const FEquipmentSlotRuntime* Slot = FindSlot(SlotTag);
	return Slot && Slot->AllowedCategory == ItemCategory;
}

bool UEquipmentComponent::CanEquipItemToSlot(const UObject* Item, FGameplayTag SlotTag) const
{
	if (!Item || !SlotTag.IsValid()) return false;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("CanEquipItemToSlot")))
	{
		return false;
	}

	const FEquipmentSlotRuntime* Slot = FindSlot(SlotTag);
	if (!Slot || Slot->EquippedItem) return false;

	const FItemMetaData& ItemData = IObjectDataProvider::Execute_GetItemRef(const_cast<UObject*>(Item));

	return IsCategoryCompatibleWithSlot(SlotTag, ItemData.ItemCategory);
}

bool UEquipmentComponent::IsItemEquippedInSlot(const UObject* Item, FGameplayTag SlotTag) const
{
	if (!Item || !SlotTag.IsValid())
		return false;

	const FEquipmentSlotRuntime* Slot = FindSlot(SlotTag);

	return Slot && Slot->EquippedItem == Item;
}

bool UEquipmentComponent::IsSlotOccupied(FGameplayTag SlotTag) const
{
	if (!SlotTag.IsValid())
		return false;

	const FEquipmentSlotRuntime* Slot = FindSlot(SlotTag);

	return Slot && Slot->EquippedItem != nullptr;
}

void UEquipmentComponent::Server_EquipItem_Implementation(UObject* Item)
{
	if (!Item) return;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("Server_EquipItem")))
	{
		return;
	}

	const FItemMetaData& ItemData = IObjectDataProvider::Execute_GetItemRef(Item);

	for (const FEquipmentSlotRuntime& Slot : EquipmentSlotsArray)
	{
		if (Slot.EquippedItem == nullptr && IsCategoryCompatibleWithSlot(Slot.SlotTag, ItemData.ItemCategory))
		{
			Server_EquipItemToSlot(Slot.SlotTag, Item);
			return;
		}
	}
}

void UEquipmentComponent::Server_EquipItemToSlot_Implementation(FGameplayTag SlotTag, UObject* Item)
{
	if (!CanEquipItemToSlot(Item, SlotTag)) return;

	IObjectDataProvider* Provider = Cast<IObjectDataProvider>(Item);
	FEquipmentSlotRuntime* Slot = FindSlot(SlotTag);

	Slot->EquippedItem = Item;
	Provider->GetOnAmountChangedDelegate().AddDynamic(this, &UEquipmentComponent::ResourceAmountChanged);

	OnEquippedItem.Broadcast(SlotTag, Item);
}

void UEquipmentComponent::Server_UnequipItemFromSlot_Implementation(FGameplayTag SlotTag)
{
	FEquipmentSlotRuntime* Slot = FindSlot(SlotTag);
	if (!Slot || !Slot->EquippedItem) return;

	UObject* RemovedItem = Slot->EquippedItem;
	if (IObjectDataProvider* Provider = Cast<IObjectDataProvider>(RemovedItem))
	{
		Provider->GetOnAmountChangedDelegate().RemoveDynamic(this, &UEquipmentComponent::ResourceAmountChanged);
	}

	Slot->EquippedItem = nullptr;

	OnUnequippedItem.Broadcast(SlotTag, RemovedItem);
}

bool UEquipmentComponent::IsItemEquipped(const UObject* Item) const
{
	if (!Item) return false;

	return EquipmentSlotsArray.ContainsByPredicate([Item](const FEquipmentSlotRuntime& Slot)
	{
		return Slot.EquippedItem == Item;
	});
}

bool UEquipmentComponent::FindSlotByItem(const UObject* Item, FGameplayTag& OutSlotTag) const
{
	if (!Item) return false;

	const FEquipmentSlotRuntime* FoundSlot = EquipmentSlotsArray.FindByPredicate(
		[Item](const FEquipmentSlotRuntime& Slot)
		{
			return Slot.EquippedItem == Item;
		});

	if (FoundSlot)
	{
		OutSlotTag = FoundSlot->SlotTag;
		return true;
	}

	return false;
}

bool UEquipmentComponent::DoesSlotExist(FGameplayTag SlotTag) const
{
	return SlotTag.IsValid() && FindSlot(SlotTag) != nullptr;
}

void UEquipmentComponent::OnRep_EquipmentSlots()
{
}

void UEquipmentComponent::ResourceAmountChanged(int32 AmountChanged, UObject* Item)
{
	if (!GetOwner()->HasAuthority() || !Item) return;

	for (FEquipmentSlotRuntime& Slot : EquipmentSlotsArray)
	{
		if (Slot.EquippedItem == Item && IObjectDataProvider::Execute_GetQuantity(Item) <= 0)
		{
			Server_UnequipItemFromSlot(Slot.SlotTag);
			return;
		}
	}
}

FEquipmentSlotRuntime* UEquipmentComponent::FindSlot(FGameplayTag SlotTag)
{
	return EquipmentSlotsArray.FindByPredicate([SlotTag](const FEquipmentSlotRuntime& Slot)
   {
	   return Slot.SlotTag == SlotTag;
   });
}

const FEquipmentSlotRuntime* UEquipmentComponent::FindSlot(FGameplayTag SlotTag) const
{
	return EquipmentSlotsArray.FindByPredicate([SlotTag](const FEquipmentSlotRuntime& Slot)
   {
	   return Slot.SlotTag == SlotTag;
   });
}
