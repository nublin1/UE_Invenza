//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/Interactable/StorageAccessComponent.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "ActorComponents/Interactable/ContainerComponent.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "EngineUtils.h"
#include "UI/Inventory/StorageAccessWidget.h"

UStorageAccessComponent::UStorageAccessComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	FInteractableData Data;
	Data.DefaultInteractableType = EInteractableType::StorageAccess;
	Data.Action = GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("StorageAccess.Open"));
	Data.ActiveAction = GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("StorageAccess.Close"));
	Data.Quantity = -1;
	InteractableDataMap.Add(EInteractionType::Primary, Data);
}

bool UStorageAccessComponent::CanPlayerAccess_Implementation(AActor* Player) const
{
	return IsValid(Player);
}

bool UStorageAccessComponent::IsUsableBy(AActor* Player) const
{
	return IsValid(Player) && IsValid(GetOwner()) && Player->GetWorld() == GetWorld() &&
		   FVector::DistSquared(Player->GetActorLocation(), GetOwner()->GetActorLocation()) <=
			   FMath::Square(MaxUseDistance) &&
		   CanPlayerAccess(Player);
}

void UStorageAccessComponent::RefreshContainers()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	DiscoveredContainers.Reset();
	if (DiscoveryMode != EStorageDiscoveryMode::Explicit)
	{
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor == GetOwner() || !Actor->FindComponentByClass<UContainerComponent>())
			{
				continue;
			}
			if (!RequiredActorTag.IsNone() && !Actor->ActorHasTag(RequiredActorTag))
			{
				continue;
			}
			if (FVector::DistSquared(Actor->GetActorLocation(), GetOwner()->GetActorLocation()) <=
				FMath::Square(SearchRadius))
			{
				DiscoveredContainers.Add(Actor);
			}
		}
	}
	RebuildInventories();
}

void UStorageAccessComponent::UnbindSources()
{
	for (auto Actor : ObservedActors)
	{
		if (Actor.IsValid())
		{
			Actor->OnDestroyed.RemoveDynamic(this, &UStorageAccessComponent::HandleContainerDestroyed);
		}
	}
	for (auto Collection : ObservedCollections)
	{
		if (Collection.IsValid())
		{
			Collection->OnActorInventoriesChanged.RemoveDynamic(this,
																&UStorageAccessComponent::HandleCollectionChanged);
		}
	}
	ObservedActors.Reset();
	ObservedCollections.Reset();
}

void UStorageAccessComponent::RebuildInventories()
{
	if (bRefreshing || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	TGuardValue<bool> Guard(bRefreshing, true);
	UnbindSources();
	Inventories.Reset();
	TSet<UInventoryBase*> Added;
	auto IsNameExcluded = [&](UInventoryBase* Inventory)
	{
		const FText Title = Inventory->GetInventorySettings().ContainerTitle;
		const FString Name =
			(Title.IsEmpty() ? Inventory->GetInventoryContainerID() : Title.ToString()).TrimStartAndEnd();
		return ExcludedInventoryNames.ContainsByPredicate(
			[&](const FString& ExcludedName)
			{
				const FString TrimmedName = ExcludedName.TrimStartAndEnd();
				return !TrimmedName.IsEmpty() && Name.Equals(TrimmedName, ESearchCase::IgnoreCase);
			});
	};
	auto Observe = [&](AActor* Actor)
	{
		if (!IsValid(Actor) || Actor->IsActorBeingDestroyed())
		{
			return;
		}
		ObservedActors.Add(Actor);
		Actor->OnDestroyed.AddUniqueDynamic(this, &UStorageAccessComponent::HandleContainerDestroyed);
		if (auto* Collection = Actor->FindComponentByClass<UInventoryCollectionComponent>())
		{
			ObservedCollections.Add(Collection);
			Collection->OnActorInventoriesChanged.AddUniqueDynamic(this,
																   &UStorageAccessComponent::HandleCollectionChanged);
		}
	};
	auto Add = [&](FStorageInventoryEntry Entry, bool bExplicit)
	{
		UInventoryBase* Inv = Entry.Inventory;
		if (!IsValid(Inv) || !IsValid(Inv->GetInventoryOwnerActor()) ||
			Inv->GetInventoryOwnerActor()->IsActorBeingDestroyed() || ExcludedInventories.Contains(Inv) ||
			ExcludedContainers.Contains(Inv->GetInventoryOwnerActor()) || Added.Contains(Inv) ||
			Inv->GetInventorySettings().bIsReferenceContainer)
		{
			return;
		}
		if (!bExplicit && RequiredInventoryTag.IsValid() &&
			!Inv->GetInventorySettings().InventoryTag.MatchesTag(RequiredInventoryTag))
		{
			return;
		}
		if (IsNameExcluded(Inv))
		{
			return;
		}
		Added.Add(Inv);
		Inventories.Add(Entry);
		Observe(Inv->GetInventoryOwnerActor());
	};
	if (DiscoveryMode != EStorageDiscoveryMode::Radius)
	{
		for (const auto& Entry : ExplicitInventories)
		{
			Add(Entry, true);
		}
	}
	TSet<TWeakObjectPtr<AActor>> Containers = DiscoveredContainers;
	if (DiscoveryMode != EStorageDiscoveryMode::Radius)
	{
		for (AActor* Actor : ExplicitContainers)
		{
			Containers.Add(Actor);
		}
	}
	for (auto WeakActor : Containers)
	{
		AActor* Actor = WeakActor.Get();
		if (!IsValid(Actor) || Actor->IsActorBeingDestroyed() || ExcludedContainers.Contains(Actor))
		{
			continue;
		}
		Observe(Actor);
		if (auto* Collection = Actor->FindComponentByClass<UInventoryCollectionComponent>())
		{
			for (auto* Inv : Collection->GetActorInventories())
			{
				FStorageInventoryEntry Entry;
				Entry.Inventory = Inv;
				Add(Entry, ExplicitContainers.Contains(Actor));
			}
		}
	}
	Inventories.Sort(
		[](const FStorageInventoryEntry& A, const FStorageInventoryEntry& B)
		{
			if (A.SortOrder != B.SortOrder)
			{
				return A.SortOrder < B.SortOrder;
			}
			const int32 Compare = A.GetDisplayName().CompareTo(B.GetDisplayName());
			return Compare != 0 ? Compare < 0
								: A.Inventory->GetInventoryContainerID() < B.Inventory->GetInventoryContainerID();
		});
	OnInventoriesChanged.Broadcast();
}

void UStorageAccessComponent::AddContainer(AActor* Container)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !IsValid(Container) || Container->GetWorld() != GetWorld())
	{
		return;
	}
	ExcludedContainers.Remove(Container);
	ExplicitContainers.AddUnique(Container);
	RebuildInventories();
}
void UStorageAccessComponent::RemoveContainer(AActor* Container)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !Container)
	{
		return;
	}
	ExplicitContainers.Remove(Container);
	ExcludedContainers.Add(Container);
	RebuildInventories();
}
void UStorageAccessComponent::AddInventory(FStorageInventoryEntry Entry)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !IsValid(Entry.Inventory) ||
		!IsValid(Entry.Inventory->GetInventoryOwnerActor()) ||
		Entry.Inventory->GetInventoryOwnerActor()->GetWorld() != GetWorld())
	{
		return;
	}
	ExcludedInventories.Remove(Entry.Inventory);
	ExplicitInventories.RemoveAll(
		[&](const FStorageInventoryEntry& Old)
		{
			return Old.Inventory == Entry.Inventory;
		});
	ExplicitInventories.Add(Entry);
	RebuildInventories();
}
void UStorageAccessComponent::RemoveInventory(UInventoryBase* Inventory)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !Inventory)
	{
		return;
	}
	ExplicitInventories.RemoveAll(
		[&](const FStorageInventoryEntry& Entry)
		{
			return Entry.Inventory == Inventory;
		});
	ExcludedInventories.Add(Inventory);
	RebuildInventories();
}
void UStorageAccessComponent::ClearExclusions()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	ExcludedContainers.Reset();
	ExcludedInventories.Reset();
	RefreshContainers();
}
bool UStorageAccessComponent::ContainsInventory(const UInventoryBase* Inventory) const
{
	return IsValid(Inventory) && Inventories.ContainsByPredicate(
									 [Inventory](const FStorageInventoryEntry& Entry)
									 {
										 return Entry.Inventory == Inventory;
									 });
}

TArray<FStorageInventoryEntry> UStorageAccessComponent::GetInventoriesForPlayer(AActor* Player)
{
	return GetInventories();
}

bool UStorageAccessComponent::ContainsInventoryForPlayer(const UInventoryBase* Inventory, AActor* Player) const
{
	return ContainsInventory(Inventory);
}
void UStorageAccessComponent::HandleContainerDestroyed(AActor* Actor)
{
	DiscoveredContainers.Remove(Actor);
	ExplicitContainers.Remove(Actor);
	ExplicitInventories.RemoveAll(
		[Actor](const FStorageInventoryEntry& Entry)
		{
			return !IsValid(Entry.Inventory) || Entry.Inventory->GetInventoryOwnerActor() == Actor;
		});
	RebuildInventories();
}
void UStorageAccessComponent::HandleCollectionChanged()
{
	RebuildInventories();
}
void UStorageAccessComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	UnbindSources();
	Inventories.Reset();
	OnInventoriesChanged.Broadcast();
	Super::EndPlay(Reason);
}
