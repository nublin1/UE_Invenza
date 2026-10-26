//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "LinkedStorageState.generated.h"

class UInventoryCollectionComponent;
class ULinkedStorageDefinition;
class APlayerController;
class ULinkedStorageComponent;

/** Persistent physical owner. Spawned in the persistent level, independently of access chests. */
UCLASS(NotPlaceable, Transient)
class INVENTORYSYSTEMINVENZAPLUGIN_API ALinkedStorageState : public AActor
{
	GENERATED_BODY()

public:
	ALinkedStorageState();

protected:
	// Unreal overrides
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget,
		const FVector& SrcLocation) const override;

public:
	// Properties and variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Linked Storage")
	TObjectPtr<UInventoryCollectionComponent> Collection;

	UPROPERTY(Transient)
	TObjectPtr<ULinkedStorageDefinition> Definition;

	// Functions
	void AddViewer(AActor* Player);
	void RemoveViewer(AActor* Player);
	bool HasViewer(const AActor* Player) const;

	/** Exclusive lease belongs to the physical storage, so every access chest shares it. */
	bool TryAcquire(AActor* Player, ULinkedStorageComponent* Chest);
	void Release(AActor* Player, const ULinkedStorageComponent* Chest);
	bool IsLeasedBy(const AActor* Player, const ULinkedStorageComponent* Chest) const;
	bool IsLockedFor(const AActor* Player) const;

protected:
	// Runtime
	// Properties and variables
	TSet<TWeakObjectPtr<AActor>> Viewers;
	TWeakObjectPtr<AActor> ActivePlayer;
	TWeakObjectPtr<APlayerController> ActiveController;
	TWeakObjectPtr<ULinkedStorageComponent> ActiveChest;

	// Functions
	bool HasValidLease() const;
};
