//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/Interactable/CharacterInventoryAccessComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetTree.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Data/Inventory/Equipment/EquipmentComponent.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Net/UnrealNetwork.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"
#include "UI/Inventory/SlotbasedInventorySlot.h"
#include "UI/Core/OperationsPanel/OperationPanelWidget.h"
#include "Utility/InvenzayUtility.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"

UCharacterInventoryAccessComponent::UCharacterInventoryAccessComponent()
{
	CharacterDisplayName =
		GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("Invenza.InventoryMannequin"));
}

void UCharacterInventoryAccessComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCharacterInventoryAccessComponent, EquipmentInventory);
}

bool UCharacterInventoryAccessComponent::ContainsInventory(UInventoryBase* Inventory) const
{
	if (!IsValid(Inventory) || (Inventory != MainLootInventory && Inventory != EquipmentInventory))
	{
		return false;
	}
	auto* Collection = Inventory->GetItemCollectionLinked();
	return IsValid(GetOwner()) && !GetOwner()->IsActorBeingDestroyed() && Collection &&
		   Inventory->GetInventoryOwnerActor() == GetOwner() && Collection->GetOwner() == GetOwner() &&
		   Collection->GetActorInventories().Contains(Inventory);
}

bool UCharacterInventoryAccessComponent::TryBeginAccess(AActor* Interactor)
{
	if (!GetOwner()->HasAuthority() || !IsValid(Interactor) ||
		!UInteractionComponent::IsWithinInteractionDistance(Interactor, this) || !TryAcquireInteraction(Interactor))
	{
		return false;
	}
	if (CurrentAccessor.IsValid() && CurrentAccessor != Interactor &&
		UInteractionComponent::IsWithinInteractionDistance(CurrentAccessor.Get(), this))
	{
		return false;
	}
	CurrentAccessor = Interactor;
	return true;
}

void UCharacterInventoryAccessComponent::EndAccess(AActor* Interactor)
{
	if (GetOwner()->HasAuthority() && CurrentAccessor == Interactor)
	{
		CurrentAccessor = nullptr;
	}
}

void UCharacterInventoryAccessComponent::InitializeInteractionComponent()
{
	if (GetOwner()->HasAuthority() && bCreateDefaultInventories && StartupInventories.IsEmpty())
	{
		const auto* Settings = UInvenzayUtility::GetInvenzaGlobalSettings(this);
		auto* Equipment = GetOwner()->FindComponentByClass<UEquipmentComponent>();
		if (!Settings || !Equipment)
		{
			return;
		}
		Equipment->InitializeSlotsFromTable();
		MainLootContainerInvTag = Settings->MainInvTag;
		EquipmentInventoryTag = Settings->EquipmentInvTag;
		const auto* Defaults = GetDefault<UInvenzaInventoryDeveloperSettings>();
		FInventoryStartupData Bag;
		Bag.Settings = Defaults->CharacterBagTemplate.ResolveSettings();
		Bag.Settings.InventoryTag = MainLootContainerInvTag;
		Bag.StartItems = BagStartingItems;
		Bag.Settings.ContainerTitle = Defaults->GetText(TEXT("Invenza.CharacterBag"));
		StartupInventories.Add(Bag);
		FInventoryStartupData Gear;
		Gear.Settings = Defaults->CharacterEquipmentTemplate.ResolveSettings();
		Gear.Settings.InventoryTag = EquipmentInventoryTag;
		Gear.Settings.ContainerTitle = Defaults->GetText(TEXT("Invenza.CharacterEquipment"));
		Gear.Settings.InventorySlotBasedSettings.InitNumColumns = FMath::Max(
			1, FMath::DivideAndRoundUp(Equipment->GetEquipmentSlotsArray().Num(),
									   FMath::Max(1, Gear.Settings.InventorySlotBasedSettings.InitNumberRows)));
		StartupInventories.Add(Gear);
	}
	Super::InitializeInteractionComponent();
	if (!GetOwner()->HasAuthority() || !ItemCollectionRef)
	{
		return;
	}
	EquipmentInventory = ItemCollectionRef->GetInventoryByTag(EquipmentInventoryTag);
	if (bCreateDefaultInventories && EquipmentInventory)
	{
		auto* Grid = Cast<USlotbasedInventory>(EquipmentInventory);
		auto* Equipment = GetOwner()->FindComponentByClass<UEquipmentComponent>();
		if (!Grid || !Equipment)
		{
			return;
		}
		const auto Slots = Grid->GetInventorySlots();
		const auto Definitions = Equipment->GetEquipmentSlotsArray();
		// Use the same authored placement around the portrait as the player's equipment screen.
		FSlotBasedInventoryWidgetInitData Layout;
		Layout.InventorySize = FIntPoint::ZeroValue;
		Layout.InvCellSize = Grid->GetInventorySettings().InventorySlotBasedSettings.InvCellSize;
		if (auto* WidgetClass =
				Cast<UWidgetBlueprintGeneratedClass>(Grid->GetInventorySettings().InventoryWidgetClass.Get()))
		{
			if (auto* Tree = WidgetClass->GetWidgetTreeArchetype())
			{
				Tree->ForEachWidget(
					[&](UWidget* Widget)
					{
						auto* SlotWidget = Cast<USlotbasedInventorySlot>(Widget);
						auto* LayoutSlot = SlotWidget ? Cast<UUniformGridSlot>(SlotWidget->Slot) : nullptr;
						if (!LayoutSlot || !Definitions.ContainsByPredicate(
											   [&](const auto& Definition)
											   {
												   return Definition.SlotTag == SlotWidget->LinkedEquipmentSlotTag;
											   }))
						{
							return;
						}
						FInventorySlotInfo Info;
						Info.CellPosition = FIntPoint(LayoutSlot->GetRow(), LayoutSlot->GetColumn());
						Info.LinkedEquipmentSlot = SlotWidget->LinkedEquipmentSlotTag;
						Info.AllowedCategory = SlotWidget->AllowedSlotCategory;
						Layout.SlotLayout.Add(Info);
						Layout.InventorySize.X = FMath::Max(Layout.InventorySize.X, Info.CellPosition.X + 1);
						Layout.InventorySize.Y = FMath::Max(Layout.InventorySize.Y, Info.CellPosition.Y + 1);
						if (auto* Panel = Cast<UUniformGridPanel>(SlotWidget->GetParent()))
						{
							Layout.SlotSpacing = Panel->GetSlotPadding();
						}
					});
			}
		}
		if (Layout.SlotLayout.Num() == Definitions.Num() && !Definitions.IsEmpty())
		{
			Grid->SetWidgetInitData(Layout);
			Grid->RebuildInventory();
			return;
		}
		for (int32 Index = 0; Index < Slots.Num(); ++Index)
		{
			if (Definitions.IsValidIndex(Index))
			{
				Slots[Index]->InventorySlotInfo.LinkedEquipmentSlot = Definitions[Index].SlotTag;
				Slots[Index]->InventorySlotInfo.AllowedCategory = Definitions[Index].AllowedCategory;
				FString SlotName = Definitions[Index].SlotTag.ToString();
				int32 LastSeparator = INDEX_NONE;
				if (SlotName.FindLastChar(TEXT('.'), LastSeparator))
				{
					SlotName.RightChopInline(LastSeparator + 1);
				}
				Slots[Index]->InventorySlotInfo.SlotName = FName(*SlotName);
			}
		}
		if (Slots.Num() > Definitions.Num())
		{
			auto ValidSlots = Slots;
			ValidSlots.SetNum(Definitions.Num());
			Grid->SetInventorySlots(ValidSlots);
		}
	}
}
