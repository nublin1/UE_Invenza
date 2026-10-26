//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "Data/Inventory/LinkedStorageTypes.h"
#include "LinkedStorageSubsystem.generated.h"

class ALinkedStorageState;
class ULinkedStorageDefinition;

UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API ULinkedStorageSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Functions
	ALinkedStorageState* FindOrCreate(ULinkedStorageDefinition* Definition, ELinkedStorageAccessMode Mode,
		AActor* Player);
	ALinkedStorageState* FindStorage(const FString& Key) const;
	FString MakeStorageKey(const ULinkedStorageDefinition* Definition, ELinkedStorageAccessMode Mode,
		AActor* Player) const;
	FString ResolvePersonalPlayerID(AActor* Player) const;

protected:
	// Runtime
	// Properties and variables
	mutable TMap<FString, TWeakObjectPtr<ALinkedStorageState>> Storages;
};
