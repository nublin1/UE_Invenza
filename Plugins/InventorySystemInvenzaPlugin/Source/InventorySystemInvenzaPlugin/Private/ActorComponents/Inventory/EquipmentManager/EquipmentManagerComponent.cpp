// Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Inventory/EquipmentManager/EquipmentManagerComponent.h"

#include "GameFramework/Actor.h"
#include "UObject/ObjectPtr.h"
#include "Engine/World.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Data/Items/itemBase.h"
#include "ActorComponents/SaveLoad/InvenzaSaveManager.h"
#include "Data/EquipmentSlotData.h"
#include "Data/EquipmentStructures.h"
#include "Data/Inventory/Equipment/EquipmentSlotDefinition.h"
#include "UI/Inventory/EquipmentInventoryWidget.h"


UEquipmentManagerComponent::UEquipmentManagerComponent(): SlotDefinitionTable(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;

	if (!InventoryContainerID.IsNone() || InventoryContainerID == "")
	{
		FString UniqueString = *FGuid::NewGuid().ToString(EGuidFormats::Digits);
		InventoryContainerID = *UniqueString;
	}
}

void UEquipmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UEquipmentManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UEquipmentManagerComponent::Initialize()
{
	InitializeSlotsFromTable();
	//BindWidgetsToSlots();
}

void UEquipmentManagerComponent::InitializeSlotsFromTable()
{
	if (!SlotDefinitionTable) return;

	EquipmentSlots.Empty();
	for (auto& Row : SlotDefinitionTable->GetRowMap())
	{
		if (const FEquipmentSlotDefinition* SlotData = reinterpret_cast<FEquipmentSlotDefinition*>(Row.Value))
		{
			UEquipmentSlotData* NewSlot = NewObject<UEquipmentSlotData>();
			NewSlot->SlotName = SlotData->SlotName;
			NewSlot->AllowedCategory = SlotData->AllowedCategory;

			EquipmentSlots.Add(NewSlot->SlotName, NewSlot);
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

void UEquipmentManagerComponent::HandleReplaceItem(TArray<UInventorySlotData*> OldItemSlots,
	TArray<UInventorySlotData*> NewItemSlots, UItemBase* Item)
{
	UnequipItemFromSlot(OldItemSlots, Item, Item->GetQuantity());
	EquipItemToSlot(NewItemSlots, Item);
}

void UEquipmentManagerComponent::HandleItemEquippedFromMapping(FItemMapping ItemSlots, UItemBase* Item)
{
	EquipItemToSlot(ItemSlots.OccupatedSlots, Item);
}

void UEquipmentManagerComponent::EquipItemToSlot(FName SlotName, UItemBase* Item)
{
	// Widget name must match slot name
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

	Slot->Get()->ItemEquipped->OnAmountChangedDelegate.AddDynamic(this, &UEquipmentManagerComponent::ResourceAmountChanged);

	// Broadcast
	OnEquippedItem.Broadcast(SlotName, Item);

	// TODO: apply effects
	return;
}

bool UEquipmentManagerComponent::EquipItem(UItemBase* Item)
{
	if (!Item) return false;
	for (auto& [Key, Val] : EquipmentSlots)
	{
		if (Val->ItemEquipped == nullptr && Val->AllowedCategory == Item->GetItemRef().ItemCategory)
		{
			Val->ItemEquipped = Item;
			// TODO: Apply effect / logic

			Val->ItemEquipped->OnAmountChangedDelegate.AddDynamic(this, &UEquipmentManagerComponent::ResourceAmountChanged);

			// Broadcast
			UE_LOG(LogTemp, Log, TEXT("EquipItem: Successfully equipped %s"), *Item->GetName());
			OnEquippedItem.Broadcast(Key, Item);
			return true;
		}
	}

	return false; // No suitable slot found
}

void UEquipmentManagerComponent::UnequipItemFromSlot(FName SlotName, UItemBase* Item)
{
	if (!Item || SlotName.IsNone()) return;

	auto Slot =EquipmentSlots.Find(SlotName);
	if (Slot == nullptr) return;
	
	// TODO: remove effects
	UItemBase* RemovedItem = Slot->Get()->ItemEquipped;
	Slot->Get()->ItemEquipped = nullptr;

	Item->OnAmountChangedDelegate.RemoveDynamic(this, &UEquipmentManagerComponent::ResourceAmountChanged);

	// Broadcast
	OnUnequippedItem.Broadcast(SlotName, RemovedItem);
}


void UEquipmentManagerComponent::ResourceAmountChanged(int32 AmountChanged, UItemBase* Item)
{
	if (!Item )
		return;

	for (auto& [Key, Val] : EquipmentSlots)
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

	UE_LOG(LogTemp, Warning, TEXT("EquipmentInventory res not found"));
}
