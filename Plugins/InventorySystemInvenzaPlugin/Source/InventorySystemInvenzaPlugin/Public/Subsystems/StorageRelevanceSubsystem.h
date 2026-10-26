//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Data/Inventory/StorageAccessTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "StorageRelevanceSubsystem.generated.h"

/** Scoped relevance for distant storages, shared by concurrent access sessions. */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UStorageRelevanceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void Deinitialize() override;

public:
	// Functions
	void Acquire(AActor* Actor);

	void Release(AActor* Actor);

protected:
	// Runtime
	// Properties and variables
	TMap<TWeakObjectPtr<AActor>, FStorageRelevanceLease> Leases;
};
