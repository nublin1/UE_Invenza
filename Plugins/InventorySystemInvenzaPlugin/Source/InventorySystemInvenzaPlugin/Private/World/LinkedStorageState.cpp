//  Nublin Studio 2026 All Rights Reserved.

#include "World/LinkedStorageState.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/Interactable/LinkedStorageComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

ALinkedStorageState::ALinkedStorageState()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	Collection = CreateDefaultSubobject<UInventoryCollectionComponent>(TEXT("Collection"));
}

void ALinkedStorageState::AddViewer(AActor* Player)
{
	if (HasAuthority() && IsValid(Player) && Player->GetWorld() == GetWorld())
	{
		Viewers.Add(Player);
		FlushNetDormancy();
		ForceNetUpdate();
	}
}

void ALinkedStorageState::RemoveViewer(AActor* Player)
{
	Viewers.Remove(Player);
	ForceNetUpdate();
}

bool ALinkedStorageState::HasViewer(const AActor* Player) const
{
	return Viewers.Contains(Player);
}

bool ALinkedStorageState::HasValidLease() const
{
	const auto* Pawn = Cast<APawn>(ActivePlayer.Get());
	const auto* Controller = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	return Pawn && IsValid(Controller) && Controller == ActiveController.Get() &&
		!Controller->IsActorBeingDestroyed() && Controller->GetPawn() == Pawn &&
		!ActivePlayer->IsActorBeingDestroyed() &&
		ActiveChest.IsValid() && IsValid(ActiveChest->GetOwner()) &&
		!ActiveChest->GetOwner()->IsActorBeingDestroyed() && ActiveChest->IsUsableBy(ActivePlayer.Get()) &&
		UInteractionComponent::IsWithinInteractionDistance(ActivePlayer.Get(), ActiveChest.Get());
}

bool ALinkedStorageState::TryAcquire(AActor* Player, ULinkedStorageComponent* Chest)
{
	const auto* Pawn = Cast<APawn>(Player);
	auto* Controller = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!HasAuthority() || !IsValid(Player) || !IsValid(Chest) || !Chest->IsUsableBy(Player) ||
		!IsValid(Controller) || !UInteractionComponent::IsWithinInteractionDistance(Player, Chest) || IsLockedFor(Player))
	{
		return false;
	}
	if (!HasValidLease())
	{
		Viewers.Reset();
	}
	ActivePlayer = Player;
	ActiveController = Controller;
	ActiveChest = Chest;
	return true;
}

void ALinkedStorageState::Release(AActor* Player, const ULinkedStorageComponent* Chest)
{
	if (HasAuthority() && ActivePlayer.Get() == Player && ActiveChest.Get() == Chest)
	{
		RemoveViewer(Player);
		ActivePlayer.Reset();
		ActiveController.Reset();
		ActiveChest.Reset();
	}
}

bool ALinkedStorageState::IsLeasedBy(const AActor* Player, const ULinkedStorageComponent* Chest) const
{
	return HasValidLease() && ActivePlayer.Get() == Player && ActiveChest.Get() == Chest;
}

bool ALinkedStorageState::IsLockedFor(const AActor* Player) const
{
	return HasValidLease() && ActivePlayer.Get() != Player;
}

bool ALinkedStorageState::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget,
	const FVector& SrcLocation) const
{
	for (const auto& Viewer : Viewers)
	{
		const auto* Pawn = Cast<APawn>(Viewer.Get());
		if (Pawn && HasValidLease() && ActivePlayer.Get() == Pawn && Pawn->GetController() == RealViewer)
		{
			return true;
		}
	}
	return false;
}
