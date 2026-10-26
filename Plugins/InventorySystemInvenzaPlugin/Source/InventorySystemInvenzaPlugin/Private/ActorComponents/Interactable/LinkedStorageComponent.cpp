//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/Interactable/LinkedStorageComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Data/Inventory/InventoryBase.h"
#include "Engine/World.h"
#include "Subsystems/LinkedStorageSubsystem.h"
#include "World/LinkedStorageState.h"

void ULinkedStorageComponent::RefreshContainers()
{
	// No radius discovery: each player receives exactly the authorized physical inventory.
	OnInventoriesChanged.Broadcast();
}

TArray<FStorageInventoryEntry> ULinkedStorageComponent::GetInventoriesForPlayer(AActor* Player)
{
	TArray<FStorageInventoryEntry> Result;
	if (!IsUsableBy(Player) || !IsInteractionOwnedBy(Player))
	{
		return Result;
	}
	auto* Registry = GetWorld()->GetSubsystem<ULinkedStorageSubsystem>();
	auto* Storage = Registry->FindOrCreate(StorageDefinition, StorageAccessMode, Player);
	if (Storage && Storage->IsLeasedBy(Player, this) && Storage->Collection->GetActorInventories().Num() == 1)
	{
		FStorageInventoryEntry Entry;
		Entry.Inventory = Storage->Collection->GetActorInventories()[0];
		Result.Add(Entry);
	}
	return Result;
}

bool ULinkedStorageComponent::ContainsInventoryForPlayer(const UInventoryBase* Inventory, AActor* Player) const
{
	if (!IsValid(Inventory) || !IsUsableBy(Player) || !IsInteractionOwnedBy(Player))
	{
		return false;
	}
	const auto* Registry = GetWorld()->GetSubsystem<ULinkedStorageSubsystem>();
	const auto* Storage = Registry->FindStorage(Registry->MakeStorageKey(StorageDefinition, StorageAccessMode, Player));
	return Storage && Storage->IsLeasedBy(Player, this) && Storage->Definition == StorageDefinition &&
		Storage->Collection->GetActorInventories().Contains(Inventory);
}

bool ULinkedStorageComponent::TryAcquireInteraction(AActor* Interactor)
{
	if (!IsValid(GetOwner()) || !GetOwner()->HasAuthority() || GetOwner()->IsActorBeingDestroyed() ||
		!IsValid(Interactor) || Interactor->IsActorBeingDestroyed() || !IsUsableBy(Interactor))
	{
		return false;
	}
	auto* Registry = GetWorld()->GetSubsystem<ULinkedStorageSubsystem>();
	auto* Storage = Registry->FindOrCreate(StorageDefinition, StorageAccessMode, Interactor);
	if (!Storage || !Storage->TryAcquire(Interactor, this))
	{
		return false;
	}
	if (const auto* Previous = LeasedStorages.Find(Interactor); Previous && Previous->IsValid() && Previous->Get() != Storage)
	{
		Previous->Get()->Release(Interactor, this);
	}
	LeasedStorages.Add(Interactor, Storage);
	Interactors.Add(Interactor);
	Interactor->OnEndPlay.AddUniqueDynamic(this, &ThisClass::HandleInteractorEndPlay);
	return true;
}

void ULinkedStorageComponent::ReleaseInteraction(AActor* Interactor)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	Interactors.Remove(Interactor);
	if (const auto* Storage = LeasedStorages.Find(Interactor); Storage && Storage->IsValid())
	{
		Storage->Get()->Release(Interactor, this);
	}
	LeasedStorages.Remove(Interactor);
	if (IsValid(Interactor))
	{
		Interactor->OnEndPlay.RemoveDynamic(this, &ThisClass::HandleInteractorEndPlay);
	}
	bIsInteracting = !Interactors.IsEmpty();
}

bool ULinkedStorageComponent::IsInteractionLockedByOther(const AActor* Interactor) const
{
	if (!IsValid(Interactor) || !GetWorld())
	{
		return true;
	}
	const auto* Registry = GetWorld()->GetSubsystem<ULinkedStorageSubsystem>();
	const auto* Storage = Registry->FindStorage(Registry->MakeStorageKey(StorageDefinition, StorageAccessMode,
		const_cast<AActor*>(Interactor)));
	return Storage && Storage->IsLockedFor(Interactor);
}

bool ULinkedStorageComponent::IsInteractionOwnedBy(const AActor* Interactor) const
{
	const auto* Storage = LeasedStorages.Find(Interactor);
	return IsValid(Interactor) && Storage && Storage->IsValid() && Storage->Get()->IsLeasedBy(Interactor, this);
}

void ULinkedStorageComponent::SetInteracting(bool NewState)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		bIsInteracting = !Interactors.IsEmpty();
	}
}

void ULinkedStorageComponent::HandleInteractorEndPlay(AActor* Actor, EEndPlayReason::Type Reason)
{
	ReleaseInteraction(Actor);
}

void ULinkedStorageComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	const auto Previous = Interactors.Array();
	for (auto Weak : Previous)
	{
		if (auto* Player = Weak.Get())
		{
			if (auto* Interaction = Player->FindComponentByClass<UInteractionComponent>();
				Interaction && Interaction->GetActiveInteractable() == this)
			{
				Interaction->StopInteract();
			}
			ReleaseInteraction(Player);
		}
	}
	Super::EndPlay(Reason);
}
