//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "LinkedStorageIdentityComponent.generated.h"

/** Attach to PlayerController; assign from authenticated server login before opening a personal chest.
 * No client RPC and no replication: clients cannot choose the account whose storage is opened. */
UCLASS(ClassGroup = (Inventory), Blueprintable, meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API ULinkedStorageIdentityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Functions
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Linked Storage")
	bool SetPersistentPlayerID(const FString& PlayerID);

	FString GetPersistentPlayerID() const { return PersistentPlayerID; }

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Transient)
	FString PersistentPlayerID;
};
