//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "ActorComponents/Interactable/StorageAccessComponent.h"
#include "Data/Inventory/LinkedStorageTypes.h"
#include "LinkedStorageComponent.generated.h"

class ULinkedStorageDefinition;
class ALinkedStorageState;

UCLASS(ClassGroup = (Inventory), Blueprintable, meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API ULinkedStorageComponent : public UStorageAccessComponent
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

public:
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Linked Storage")
	TObjectPtr<ULinkedStorageDefinition> StorageDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Linked Storage")
	ELinkedStorageAccessMode StorageAccessMode = ELinkedStorageAccessMode::Shared;

	// Functions
	virtual void RefreshContainers() override;
	virtual TArray<FStorageInventoryEntry> GetInventoriesForPlayer(AActor* Player) override;
	virtual bool ContainsInventoryForPlayer(const UInventoryBase* Inventory, AActor* Player) const override;
	virtual bool TryAcquireInteraction(AActor* Interactor) override;
	virtual void ReleaseInteraction(AActor* Interactor) override;
	virtual bool IsInteractionLockedByOther(const AActor* Interactor) const override;
	virtual bool IsInteractionOwnedBy(const AActor* Interactor) const override;
	virtual void SetInteracting(bool NewState) override;

protected:
	// Runtime
	// Properties and variables
	TSet<TWeakObjectPtr<AActor>> Interactors;
	TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<ALinkedStorageState>> LeasedStorages;

	// Functions
	UFUNCTION()
	void HandleInteractorEndPlay(AActor* Actor, EEndPlayReason::Type Reason);
};
