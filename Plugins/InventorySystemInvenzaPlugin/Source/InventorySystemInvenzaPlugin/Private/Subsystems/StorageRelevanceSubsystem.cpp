//  Nublin Studio 2026 All Rights Reserved.

#include "Subsystems/StorageRelevanceSubsystem.h"
#include "GameFramework/Actor.h"
void UStorageRelevanceSubsystem::Acquire(AActor* Actor)
{
	if (!IsValid(Actor) || !Actor->HasAuthority())
	{
		return;
	}
	auto& Lease = Leases.FindOrAdd(Actor);
	if (Lease.Count++ == 0)
	{
		Lease.bAlwaysRelevant = Actor->bAlwaysRelevant;
		Actor->bAlwaysRelevant = true;
	}
	Actor->FlushNetDormancy();
	Actor->ForceNetUpdate();
}
void UStorageRelevanceSubsystem::Release(AActor* Actor)
{
	if (auto* Lease = Leases.Find(Actor); Lease && --Lease->Count == 0)
	{
		if (IsValid(Actor))
		{
			Actor->bAlwaysRelevant = Lease->bAlwaysRelevant;
			Actor->ForceNetUpdate();
		}
		Leases.Remove(Actor);
	}
}
void UStorageRelevanceSubsystem::Deinitialize()
{
	for (auto& Pair : Leases)
	{
		if (Pair.Key.IsValid())
		{
			Pair.Key->bAlwaysRelevant = Pair.Value.bAlwaysRelevant;
		}
	}
	Leases.Reset();
	Super::Deinitialize();
}
