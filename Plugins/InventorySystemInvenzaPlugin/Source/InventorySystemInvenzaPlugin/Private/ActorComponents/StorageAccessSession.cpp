//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/UIInventoryManager.h"
#include "TimerManager.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/Interactable/StorageAccessComponent.h"
#include "ActorComponents/Interactable/LinkedStorageComponent.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "ActorComponents/Interactable/CharacterInventoryAccessComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Interface/Inventory/InvUIProvider.h"
#include "Subsystems/StorageRelevanceSubsystem.h"
#include "World/LinkedStorageState.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Inventory/StorageAccessWidget.h"
#include "UI/Inventory/InventoryWorkspaceWidget.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"
#include "Utility/InputUtility.h"

bool UIInventoryManager::IsStorageInventoryAccessible(const UInventoryBase* Inventory) const
{
	if (!IsValid(Inventory) || !IsValid(StorageSession.AccessPoint))
	{
		return false;
	}

	if (GetOwner()->HasAuthority())
	{
		return StorageSession.AccessPoint->IsUsableBy(GetOwner()) &&
			   UInteractionComponent::IsWithinInteractionDistance(GetOwner(), StorageSession.AccessPoint) &&
			   StorageSession.AccessPoint->ContainsInventoryForPlayer(Inventory, GetOwner());
	}

	return StorageSession.Revision != ClosedStorageRevision && StorageSession.Inventories.ContainsByPredicate(
																   [Inventory](const FStorageInventoryEntry& Entry)
																   {
																	   return Entry.Inventory == Inventory;
																   });
}

bool UIInventoryManager::IsInventoryAccessible(UInventoryBase* Inventory) const
{
	if (!IsValid(Inventory) || !ItemCollectionRef)
	{
		return false;
	}

	if (ItemCollectionRef->GetActorInventories().Contains(Inventory))
	{
		return true;
	}

	const auto Linked = ItemCollectionRef->GetLinkedInventories();
	const auto Craft = ItemCollectionRef->GetLinkedCraftInventories();
	return Inventory == Linked.ExternalInventory || Inventory == Linked.VendorInventory ||
		   Inventory == Craft.InputInventory || Inventory == Craft.FuelInventory ||
		   Inventory == Craft.OutputInventory || IsStorageInventoryAccessible(Inventory) ||
		   IsCharacterInventoryAccessible(Inventory);
}

UInventoryBase* UIInventoryManager::FindStorageInventory(const FString& ID) const
{
	for (const auto& Entry : StorageSession.Inventories)
	{
		if (IsValid(Entry.Inventory) && Entry.Inventory->GetInventoryContainerID() == ID &&
			IsStorageInventoryAccessible(Entry.Inventory))
		{
			return Entry.Inventory;
		}
	}

	if (CharacterInventoryAccess)
	{
		for (auto* Inventory : {CharacterInventoryAccess->GetMainLootContainer().Get(),
								CharacterInventoryAccess->GetEquipmentInventory()})
		{
			if (IsValid(Inventory) && Inventory->GetInventoryContainerID() == ID &&
				IsCharacterInventoryAccessible(Inventory))
			{
				return Inventory;
			}
		}
	}
	return nullptr;
}

void UIInventoryManager::OpenStorageAccess(UStorageAccessComponent* AccessPoint)
{
	if (!IsValid(AccessPoint) || !AccessPoint->IsUsableBy(GetOwner()) || !ItemCollectionRef ||
		!UInteractionComponent::IsWithinInteractionDistance(GetOwner(), AccessPoint) ||
		!AccessPoint->TryAcquireInteraction(GetOwner()))
	{
		return;
	}
	if (StorageSession.AccessPoint == AccessPoint)
	{
		EndStorageAccess();
		return;
	}

	EndStorageAccess();
	HandleClearInteraction(nullptr, EInteractionType::Primary);
	TrackInventoryInteraction(AccessPoint, EInteractionType::Primary);
	AccessPoint->RefreshContainers();
	StorageSession.AccessPoint = AccessPoint;
	++StorageSession.Revision;
	AccessPoint->OnInventoriesChanged.AddUniqueDynamic(this, &ThisClass::HandleStorageListChanged);
	HandleStorageListChanged();
}

void UIInventoryManager::HandleStorageListChanged()
{
	auto* Access = StorageSession.AccessPoint.Get();
	if (!IsValid(Access) || !IsValid(Access->GetOwner()) || Access->GetOwner()->IsActorBeingDestroyed())
	{
		EndStorageAccess();
		return;
	}

	const auto Inventories = Access->GetInventoriesForPlayer(GetOwner());
	if (Cast<ULinkedStorageComponent>(Access) && Inventories.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("Linked storage access denied: verify group definition and authenticated player identity."));
		EndStorageAccess();
		return;
	}
	TArray<TWeakObjectPtr<UInteractableComponent>> NewLocks;
	for (const auto& Entry : Inventories)
	{
		AActor* InventoryOwner = IsValid(Entry.Inventory) ? Entry.Inventory->GetInventoryOwnerActor() : nullptr;
		UInteractableComponent* Container = IsValid(InventoryOwner)
			? InventoryOwner->FindComponentByClass<UInteractableComponent>() : nullptr;
		if (!Container || InventoryOwner == Access->GetOwner() || NewLocks.Contains(Container))
		{
			continue;
		}
		if (!Container->TryAcquireInteraction(GetOwner()))
		{
			for (auto Lock : NewLocks)
			{
				if (Lock.IsValid() && !StorageInteractionLocks.Contains(Lock))
				{
					Lock->ReleaseInteraction(GetOwner());
				}
			}
			EndStorageAccess();
			return;
		}
		NewLocks.Add(Container);
	}
	for (auto Lock : StorageInteractionLocks)
	{
		if (Lock.IsValid() && !NewLocks.Contains(Lock))
		{
			Lock->ReleaseInteraction(GetOwner());
		}
	}
	StorageInteractionLocks = MoveTemp(NewLocks);
	StorageSession.Inventories = Inventories;
	UpdateStorageRelevance();
	GetOwner()->ForceNetUpdate();
	OnRep_StorageSession();
}

void UIInventoryManager::UpdateStorageRelevance()
{
	auto* Relevance = GetWorld() ? GetWorld()->GetSubsystem<UStorageRelevanceSubsystem>() : nullptr;
	if (!Relevance)
	{
		StorageRelevantActors.Reset();
		return;
	}
	TSet<TWeakObjectPtr<AActor>> Current;
	for (const auto& Entry : StorageSession.Inventories)
	{
		if (IsValid(Entry.Inventory))
		{
			Current.Add(Entry.Inventory->GetInventoryOwnerActor());
			if (auto* Collection = Entry.Inventory->GetItemCollectionLinked())
			{
				Current.Add(Collection->GetOwner());
			}
		}
	}

	if (StorageSession.AccessPoint)
	{
		Current.Add(StorageSession.AccessPoint->GetOwner());
	}

	for (auto Actor : Current)
	{
		if (Actor.IsValid() && !StorageRelevantActors.Contains(Actor))
		{
			if (auto* LinkedStorage = Cast<ALinkedStorageState>(Actor.Get()))
			{
				LinkedStorage->AddViewer(GetOwner());
			}
			else
			{
				Relevance->Acquire(Actor.Get());
			}
		}
	}
	for (auto Actor : StorageRelevantActors)
	{
		if (!Current.Contains(Actor))
		{
			if (auto* LinkedStorage = Cast<ALinkedStorageState>(Actor.Get()))
			{
				LinkedStorage->RemoveViewer(GetOwner());
			}
			else
			{
				Relevance->Release(Actor.Get());
			}
		}
	}

	StorageRelevantActors = MoveTemp(Current);
}

void UIInventoryManager::ReleaseStorageInteractionLocks()
{
	for (auto Lock : StorageInteractionLocks)
	{
		if (Lock.IsValid())
		{
			Lock->ReleaseInteraction(GetOwner());
		}
	}
	StorageInteractionLocks.Reset();
}

void UIInventoryManager::EndStorageAccess()
{
	if (StorageSession.AccessPoint && TrackedInteractionTarget == StorageSession.AccessPoint)
	{
		HandleClearInteraction(StorageSession.AccessPoint, TrackedInteractionType);
		return;
	}

	if (StorageSession.AccessPoint)
	{
		StorageSession.AccessPoint->OnInventoriesChanged.RemoveDynamic(this, &ThisClass::HandleStorageListChanged);
	}

	ReleaseStorageInteractionLocks();
	StorageSession.AccessPoint = nullptr;
	StorageSession.Inventories.Reset();
	++StorageSession.Revision;

	UpdateStorageRelevance();
	GetOwner()->ForceNetUpdate();
	OnRep_StorageSession();
}

void UIInventoryManager::CloseStorageAccess()
{
	const uint32 Revision = StorageSession.Revision;
	if (InteractionComponent && InteractionComponent->GetActiveInteractable() == StorageSession.AccessPoint &&
		StorageSession.AccessPoint)
	{
		InteractionComponent->StopInteract();
	}

	ClosedStorageRevision = StorageSession.Revision;
	ClearStorageUI();
	if (GetOwner()->HasAuthority())
	{
		if (StorageSession.AccessPoint)
		{
			EndStorageAccess();
		}
	}
	else
	{
		Server_CloseStorageAccess(Revision);
	}
}

void UIInventoryManager::Server_CloseStorageAccess_Implementation(uint32 Revision)
{
	if (StorageSession.Revision == Revision)
	{
		EndStorageAccess();
	}
}

void UIInventoryManager::RefreshStorageAccess()
{
	Server_RefreshStorageAccess(StorageSession.Revision);
}

void UIInventoryManager::Server_RefreshStorageAccess_Implementation(uint32 Revision)
{
	if (Revision != StorageSession.Revision)
	{
		return;
	}
	if (StorageSession.AccessPoint && StorageSession.AccessPoint->IsUsableBy(GetOwner()))
	{
		StorageSession.AccessPoint->RefreshContainers();
	}
	else
	{
		EndStorageAccess();
	}
}

void UIInventoryManager::ClearStorageUI()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(StorageUIRefreshTimer);
	}
	for (auto Inv : ObservedStorageInventories)
	{
		if (Inv.IsValid())
		{
			Inv->OnConfigurationChanged.RemoveDynamic(this, &ThisClass::OnRep_StorageSession);
		}
	}

	ObservedStorageInventories.Reset();
	if (IsValid(StorageWindow))
	{
		auto* Window = StorageWindow.Get();
		StorageWindow = nullptr;
		Window->SetManager(nullptr);
		auto* PlayerWidget = IsValid(Window->Workspace) ? Window->Workspace->ReleasePlayerWidget() : nullptr;
		Window->RemoveFromParent();
		const bool bRestorePlayerWidget = !bEndingPlay && World && !World->bIsTearingDown && IsValid(GetOwner()) &&
										  !GetOwner()->IsActorBeingDestroyed();
		if (IsValid(PlayerWidget) && bRestorePlayerWidget && IsValid(UIInvProvider.GetObject()))
		{
			UIInvProvider->AddPawnInvContainerWidget(PlayerWidget);
		}
		else if (IsValid(PlayerWidget))
		{
			PlayerWidget->ReleaseInventory();
		}
	}

	for (auto& Pair : StorageWidgets)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->ReleaseInventory();
		}

		if (IsValid(ItemCollectionRef))
		{
			ItemCollectionRef->UnregisterContainerWidget(Pair.Key);
		}
	}
	StorageWidgets.Reset();
}

void UIInventoryManager::OnRep_StorageSession()
{
	if (bEndingPlay || !GetWorld() || GetWorld()->bIsTearingDown)
	{
		return;
	}
	if (!OwnerPawnRef || !OwnerPawnRef->IsLocallyControlled())
	{
		return;
	}

	if (!StorageSession.AccessPoint || StorageSession.Revision == ClosedStorageRevision)
	{
		ClearStorageUI();
		return;
	}
	// Coalesce notifications; never rebuild UMG from inside an inventory mutation.
	if (!GetWorld()->GetTimerManager().IsTimerActive(StorageUIRefreshTimer))
	{
		StorageUIRefreshTimer = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::RefreshStorageUI);
	}
}

void UIInventoryManager::RefreshStorageUI()
{
	if (bEndingPlay || !GetWorld() || GetWorld()->bIsTearingDown || bUpdatingStorageUI || !IsValid(OwnerPawnRef) ||
		!OwnerPawnRef->IsLocallyControlled())
	{
		return;
	}

	TGuardValue<bool> Guard(bUpdatingStorageUI, true);
	if (!StorageSession.AccessPoint || StorageSession.Revision == ClosedStorageRevision)
	{
		ClearStorageUI();
		return;
	}
	if (!ItemCollectionRef || !UIInvProvider)
	{
		return;
	}

	if (!StorageWindow)
	{
		if (!StorageSession.AccessPoint->WindowClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("Storage access point requires a WindowClass."));
			CloseStorageAccess();
			return;
		}

		UIInvProvider->CloseCraftMenu();
		UIInvProvider->CloseDualInventoryView();
		StorageWindow = CreateWidget<UStorageAccessWidget>(Cast<APlayerController>(OwnerPawnRef->GetController()),
														   StorageSession.AccessPoint->WindowClass);
		if (!StorageWindow)
		{
			return;
		}

		StorageWindow->SetManager(this);
		StorageWindow->TitleLabel->UpdateText(StorageSession.AccessPoint->WindowTitle);
		auto* PlayerWidget = ItemCollectionRef->GetContainerWidget(MainPawnInventoryRef);
		if (!PlayerWidget)
		{
			PlayerWidget = CreateInventoryWidget(MainPawnInventoryRef);
		}

		StorageWindow->Workspace->SetPlayerWidget(PlayerWidget);
		if (!UIInvProvider->AttachStorageWindow(StorageWindow))
		{
			StorageWindow->AddToViewport(20);
		}
	}

	TSet<UInventoryBase*> Current;
	TArray<UInventoryContainerWidget*> Widgets;
	TArray<FText> Titles;
	const TArray<FStorageInventoryEntry> Entries = StorageSession.Inventories;
	for (const auto& Entry : Entries)
	{
		UInventoryBase* Inv = Entry.Inventory;
		if (!IsValid(Inv) || Inv == MainPawnInventoryRef || Current.Contains(Inv))
		{
			continue;
		}

		Current.Add(Inv);
		Inv->OnConfigurationChanged.AddUniqueDynamic(this, &ThisClass::OnRep_StorageSession);
		ObservedStorageInventories.Add(Inv);
		if (!Inv->GetItemCollectionLinked() || Inv->GetInventoryContainerID().IsEmpty())
		{
			continue;
		}

		auto& Widget = StorageWidgets.FindOrAdd(Inv);
		if (!Widget)
		{
			Widget = CreateInventoryWidget(Inv);
		}

		if (Widget)
		{
			Widgets.Add(Widget);
			Titles.Add(Entry.GetDisplayName());
		}
	}

	for (auto It = StorageWidgets.CreateIterator(); It; ++It)
	{
		if (!Current.Contains(It.Key()))
		{
			if (IsValid(It.Value()))
			{
				It.Value()->ReleaseInventory();
			}
			ItemCollectionRef->UnregisterContainerWidget(It.Key());
			It.RemoveCurrent();
		}
	}
	for (auto It = ObservedStorageInventories.CreateIterator(); It; ++It)
	{
		if (!Current.Contains(It->Get()))
		{
			if (It->IsValid())
			{
				It->Get()->OnConfigurationChanged.RemoveDynamic(this, &ThisClass::OnRep_StorageSession);
			}
			It.RemoveCurrent();
		}
	}
	StorageWindow->Workspace->SetInventoryWidgets(Widgets, Titles);
}

void UIInventoryManager::EndPlay(const EEndPlayReason::Type Reason)
{
	bEndingPlay = true;
	if (ItemCollectionRef)
	{
		ItemCollectionRef->OnActorInventoriesChanged.RemoveDynamic(this, &ThisClass::QueueInventoryUIRefresh);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StorageUIRefreshTimer);
		World->GetTimerManager().ClearTimer(InventoryInitTimer);
		World->GetTimerManager().ClearTimer(ReplicatedInventoryUITimer);
		World->GetTimerManager().ClearTimer(FallbackInteractionRangeTimer);
	}
	if (ControlsHelpInput.IsValid())
	{
		UInputUtility::RemoveBinding(ControlsHelpInput.Get(), ControlsHelpBindingHandle);
	}
	if (GetOwner()->HasAuthority())
	{
		HandleClearInteraction(TrackedInteractionTarget.Get(), TrackedInteractionType);
	}
	if (GetOwner()->HasAuthority())
	{
		EndStorageAccess();
	}
	ClearStorageUI();
	ClearCharacterInventoryUI();
	Super::EndPlay(Reason);
}
