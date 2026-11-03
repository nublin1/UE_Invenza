// Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/ItemData.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interface/Interaction/ObjectDataProvider.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "Subsystems/ModalWindowManager.h"
#include "UI/Inventory/Admission/InventoryAdmissionWidget.h"

bool UIInventoryManager::CanEditAdmissionRules(UInventoryBase* Inventory) const
{
	if (bEndingPlay || !IsInventoryAccessible(Inventory) ||
		!Inventory->GetInventorySettings().bAllowAdmissionRulesEditing ||
		Inventory->GetInventorySettings().bIsReferenceContainer || VendorProviderCurrent.GetObject())
	{
		return false;
	}
	if (GetOwner()->HasAuthority() && Inventory->GetItemCollectionLinked() != ItemCollectionRef &&
		Inventory != MainPawnInventoryRef)
	{
		return IsValid(TrackedInteractionTarget) && TrackedInteractionTarget->IsInteractionOwnedBy(GetOwner()) &&
			UInteractionComponent::IsWithinInteractionDistance(GetOwner(), TrackedInteractionTarget);
	}
	return true;
}

void UIInventoryManager::Server_SetAdmissionRules_Implementation(UInventoryBase* Inventory,
	FInventoryAdmissionRules Rules, FGuid RequestID)
{
	bool bSuccess = RequestID.IsValid() && CanEditAdmissionRules(Inventory) &&
		!bApplyingAmountTransfer && !bApplyingEquipmentTransfer;
	const auto* Settings = GetDefault<UInvenzaInventoryDeveloperSettings>();
	const FGameplayTag Root = Settings->AdmissionCategoryRoot;
	bSuccess &= Root.IsValid() && Rules.AllowedCategories.Num() + Rules.ExcludedCategories.Num() <= 512;
	for (const FGameplayTagContainer* Tags : {&Rules.AllowedCategories, &Rules.ExcludedCategories})
	{
		for (FGameplayTag Tag : *Tags)
		{
			bSuccess &= Tag.IsValid() && Tag.MatchesTag(Root);
		}
	}
	bSuccess &= Rules.AllowedItems.Num() + Rules.ExcludedItems.Num() <= 512;
	for (const auto* Items : {&Rules.AllowedItems, &Rules.ExcludedItems})
	{
		for (const FDataTableRowHandle& Item : *Items)
		{
			const bool bKnownTable = Settings->AdmissionItemTables.ContainsByPredicate([&](const auto& Table)
			{
				return Table.ToSoftObjectPath() == FSoftObjectPath(Item.DataTable);
			});
			const FItemData* Data = bKnownTable && Item.DataTable && Item.DataTable->GetRowStruct() == FItemData::StaticStruct() ?
				Item.DataTable->FindRow<FItemData>(Item.RowName, TEXT("Admission edit"), false) : nullptr;
			bSuccess &= Data && Data->ItemMetaData.ItemCategory.MatchesTag(Root);
		}
	}
	if (const auto* Instance = GetWorld()->GetGameInstance())
	{
		if (const auto* Saves = Instance->GetSubsystem<UInvenzaSaveSubsystem>())
		{
			bSuccess &= !Saves->IsApplyingSnapshot();
		}
	}
	if (bSuccess)
	{
		Inventory->ApplyAdmissionRules(Rules);
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("Admission edit rejected: inaccessible inventory, locked configuration or invalid tags."));
	}
	Client_AdmissionRulesResult(RequestID, bSuccess);
}

void UIInventoryManager::Client_AdmissionRulesResult_Implementation(FGuid RequestID, bool bSuccess)
{
	const auto* Pawn = Cast<APawn>(GetOwner());
	auto* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : Cast<APlayerController>(GetOwner());
	if (PC && PC->GetLocalPlayer())
	{
		if (auto* Modals = PC->GetLocalPlayer()->GetSubsystem<UModalWindowManager>())
		{
			if (auto* Window = Modals->GetAdmissionWindow())
			{
				Window->CompleteSubmission(RequestID, bSuccess);
			}
		}
	}
}

void UIInventoryManager::Client_CloseAdmissionRules_Implementation()
{
	const auto* Pawn = Cast<APawn>(GetOwner());
	auto* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : Cast<APlayerController>(GetOwner());
	if (PC && PC->GetLocalPlayer())
	{
		if (auto* Modals = PC->GetLocalPlayer()->GetSubsystem<UModalWindowManager>())
		{
			Modals->CloseAdmissionRules();
		}
	}
}

void UIInventoryManager::TransferAllowedItems(UInventoryBase* SourceInventory, UInventoryBase* TargetInventory)
{
	if (!CanTransferMatchingItems(SourceInventory, TargetInventory) || !GetOwner())
	{
		return;
	}
	if (!GetOwner()->HasAuthority())
	{
		Server_TransferAllowedItems(SourceInventory, TargetInventory);
		return;
	}
	auto* Collection = SourceInventory->GetItemCollectionLinked();
	const FString SourceID = SourceInventory->GetInventoryContainerID();
	const TArray<UObject*> Items = Collection->GetAllItemsByContainer(SourceID);
	for (UObject* Item : Items)
	{
		if (!TargetInventory->CanReceiveItem(Item) || !Item->Implements<UObjectDataProvider>())
		{
			continue;
		}
		const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(Item);
		if (Quantity <= 0 || IObjectDataProvider::Execute_GetFreeAmount(Item) < Quantity)
		{
			continue;
		}
		const FItemMapping* Mapping = Collection->FindItemMappingByContainerName(Item, SourceID);
		if (!Mapping || Mapping->bIsReferenceContainer)
		{
			continue;
		}
		FItemMoveData Move;
		Move.SourceItem = Item;
		Move.SourceInventory = SourceInventory;
		Move.TargetInventory = TargetInventory;
		Move.SavedOrientation = Mapping->ItemOrientation;
		Move.TargetOrientation = Mapping->ItemOrientation;
		if (!Mapping->OccupiedSlots.IsEmpty() && Mapping->OccupiedSlots[0])
		{
			Move.SourceSlotID = Mapping->OccupiedSlots[0]->InventorySlotInfo.SlotGuid;
		}
		Execute_ItemTransferRequest(this, Move);
	}
}

void UIInventoryManager::Server_TransferAllowedItems_Implementation(UInventoryBase* SourceInventory,
	UInventoryBase* TargetInventory)
{
	TransferAllowedItems(SourceInventory, TargetInventory);
}
