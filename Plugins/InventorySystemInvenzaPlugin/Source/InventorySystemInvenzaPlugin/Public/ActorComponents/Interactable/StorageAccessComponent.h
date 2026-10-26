//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "ActorComponents/Interactable/InteractableComponent.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "Data/Inventory/StorageAccessTypes.h"
#include "StorageAccessComponent.generated.h"

class UInventoryCollectionComponent;
class UStorageAccessWidget;

/** Server-owned catalog of inventories; never stores or copies their items. */
UCLASS(ClassGroup = (Inventory), Blueprintable, meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UStorageAccessComponent : public UInteractableComponent
{
	GENERATED_BODY()

public:
	UStorageAccessComponent();

protected:
	// Unreal overrides
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStorageListChanged);
#pragma endregion Delegates

	// Properties and variables
	UPROPERTY(BlueprintAssignable, Category = "Storage Access")
	FStorageListChanged OnInventoriesChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Access|Discovery")
	EStorageDiscoveryMode DiscoveryMode = EStorageDiscoveryMode::RadiusAndExplicit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Access|Discovery",
			  meta = (ClampMin = "0", Units = "cm"))
	float SearchRadius = 2000.f;

	/** Optional actor tag shared by the base's containers. Auto discovery only. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Access|Discovery")
	FName RequiredActorTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Access|Discovery")
	FGameplayTag RequiredInventoryTag;

	/** Excludes all sources by ContainerTitle (or inventory ID if empty), regardless of DisplayName overrides.
	 * Full match, case-insensitive, ignoring surrounding whitespace. Empty entries are ignored.
	 * After changing at runtime on the server, call RefreshContainers. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Access|Discovery")
	TArray<FString> ExcludedInventoryNames;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Storage Access|Discovery")
	TArray<TObjectPtr<AActor>> ExplicitContainers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage Access|UI")
	TSubclassOf<UStorageAccessWidget> WindowClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Access|UI")
	FText WindowTitle = GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("StorageAccess.Title"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Access|Interaction",
			  meta = (ClampMin = "1", Units = "cm"))
	float MaxUseDistance = 600.f;

	// Functions
	/** Override for base ownership / team / permissions. Distance is checked separately. */
	UFUNCTION(BlueprintNativeEvent, Category = "Storage Access")
	bool CanPlayerAccess(AActor* Player) const;

	bool IsUsableBy(AActor* Player) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Storage Access")
	virtual void RefreshContainers();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Storage Access")
	void AddContainer(AActor* Container);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Storage Access")
	void RemoveContainer(AActor* Container);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Storage Access")
	void AddInventory(FStorageInventoryEntry Entry);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Storage Access")
	void RemoveInventory(UInventoryBase* Inventory);

	/** Clears runtime removals; does not clear ExcludedInventoryNames. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Storage Access")
	void ClearExclusions();

	UFUNCTION(BlueprintPure, Category = "Storage Access")
	TArray<FStorageInventoryEntry> GetInventories() const { return Inventories; }

	bool ContainsInventory(const UInventoryBase* Inventory) const;

	/** Per-player catalogs allow private linked storages to reuse the storage window. */
	virtual TArray<FStorageInventoryEntry> GetInventoriesForPlayer(AActor* Player);
	virtual bool ContainsInventoryForPlayer(const UInventoryBase* Inventory, AActor* Player) const;

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Transient)
	TArray<FStorageInventoryEntry> Inventories;

	UPROPERTY(Transient)
	TArray<FStorageInventoryEntry> ExplicitInventories;

	TSet<TWeakObjectPtr<AActor>> ExcludedContainers;

	TSet<TWeakObjectPtr<UInventoryBase>> ExcludedInventories;

	TSet<TWeakObjectPtr<AActor>> DiscoveredContainers;

	TSet<TWeakObjectPtr<AActor>> ObservedActors;

	TSet<TWeakObjectPtr<UInventoryCollectionComponent>> ObservedCollections;

	bool bRefreshing = false;

	// Functions
	void RebuildInventories();

	void UnbindSources();

	UFUNCTION()
	void HandleContainerDestroyed(AActor* Actor);

	UFUNCTION()
	void HandleCollectionChanged();
};
