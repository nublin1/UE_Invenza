//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveLoad/InventorySaveTypes.h"
#include "InvenzaSaveSubsystem.generated.h"

class UInvenzaSaveGame;
class UInventoryCollectionComponent;
class USaveGame;
class UInvenzaSaveWidget;
class UPickupComponent;

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInvenzaSaveStateChanged);
#pragma endregion Delegates

UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

public:
	// Properties and variables
	UPROPERTY(BlueprintAssignable, Category = "Invenza|Save")
	FInvenzaSaveStateChanged OnStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Invenza|Save")
	FInvenzaSaveStateChanged OnSessionLoaded;

	UPROPERTY(BlueprintReadOnly, Category = "Invenza|Save")
	FText Status;

	UPROPERTY(BlueprintReadOnly, Category = "Invenza|Save")
	bool bBusy = false;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Invenza|Save")
	bool SaveSession(const FString& Slot = TEXT("Quick"), bool bAsync = true);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Save")
	bool LoadSession(const FString& Slot = TEXT("Continue"));

	/** Confirmation belongs to the caller. Host-only; writes an empty new generation before reloading. */
	UFUNCTION(BlueprintCallable, Category = "Invenza|Save")
	bool ResetPlaythrough();

	UFUNCTION(BlueprintCallable, Category = "Invenza|Save")
	bool RetryRecovery();

	UFUNCTION(BlueprintPure, Category = "Invenza|Save")
	TArray<FInventorySavedItem> GetRecoveryItems() const;

	UFUNCTION(BlueprintPure, Category = "Invenza|Save")
	TArray<FInventorySaveSlotInfo> GetSaveSlots() const;

	UFUNCTION(BlueprintPure, Category = "Invenza|Save")
	TArray<FInventorySaveSlotInfo> GetSaveSlotsForWorld(const FString& WorldPackage) const;

	UFUNCTION(BlueprintCallable, Category = "Invenza|Save")
	bool LoadWorldSave(const TSoftObjectPtr<UWorld>& World, const FString& Slot);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Save")
	void ShowSaveMenu(APlayerController* Player);

	UFUNCTION(BlueprintPure, Category = "Invenza|Save")
	bool CanManageSession() const;

	UFUNCTION(BlueprintPure, Category = "Invenza|Save")
	bool IsApplyingSnapshot() const { return bApplyingSnapshot; }

	UFUNCTION(BlueprintPure, Category = "Invenza|Save")
	EInventoryClientTrust GetClientTrust() const;

	UFUNCTION(BlueprintCallable, Category = "Invenza|Save")
	bool SetClientTrust(EInventoryClientTrust Mode);

	bool SaveBeforeExit();

	void RegisterCollection(UInventoryCollectionComponent* Collection);

	void RegisterPickup(UPickupComponent* Pickup);

	void UnregisterPickup(UPickupComponent* Pickup, EEndPlayReason::Type Reason);

	void UnregisterCollection(UInventoryCollectionComponent* Collection, EEndPlayReason::Type Reason);

	void NotifyPlayerReady();

	void MarkSessionChanged();

	UInvenzaSaveGame* CaptureSnapshot(FString& Error);

	UInvenzaSaveGame* GetLoadedSnapshot() const { return LastSnapshot; }

	bool ApplySnapshot(const UInvenzaSaveGame* Snapshot, FString& Error);

	/** A trusted client may replace only its own owner record. No world loads or resets. */
	bool ApplyTrustedOwner(AActor* Requestor, const FInventorySavedOwner& Owner, FString& Error);

	/** Isolated test/server profiles must be selected before any operation starts. */
	bool SetProfile(const FString& Profile);

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Transient)
	TObjectPtr<UInvenzaSaveGame> PendingWrite;

	UPROPERTY(Transient)
	TObjectPtr<UInvenzaSaveGame> PendingTravelSave;

	UPROPERTY(Transient)
	TObjectPtr<UInvenzaSaveGame> LastSnapshot;

	UPROPERTY(Transient)
	TObjectPtr<UInvenzaSaveWidget> Window;

	TSet<TWeakObjectPtr<UInventoryCollectionComponent>> Collections;

	TSet<TWeakObjectPtr<UPickupComponent>> Pickups;

	TMap<FString, FInventorySavedPickup> RetainedPickups;

	TMap<FString, FInventorySavedOwner> RetainedOwners;

	TSet<FString> RestoredOwners;

	TWeakObjectPtr<UWorld> SessionWorld;

	TWeakObjectPtr<UWorld> PendingWriteWorld;

	FTimerHandle AutoSaveTimer;

	FTimerHandle StartupTimer;

	FDelegateHandle WorldTeardownHandle;

	FString ProfileOverride;

	TArray<FString> QueuedSaveSlots;

	FString PendingPhysicalSlot;

	FGuid PlaythroughID;

	int64 Revision = 0;

	uint64 ChangeRevision = 0;

	uint64 SavedChangeRevision = 0;

	uint64 CapturedChangeRevision = 0;

	int32 AutoSaveIndex = 0;

	bool bApplyingSnapshot = false;

	bool bAutoLoadAttempted = false;

	bool bSessionTrustOverride = false;

	bool bSuppressExitSave = false;

	EInventoryClientTrust SessionTrust = EInventoryClientTrust::ServerValidated;

	// Functions
	FString WorldKey() const;

	FString SlotPrefix(const FString& WorldPackage = FString()) const;

	FString PhysicalSlot(const FString& Logical, int32 Generation, const FString& WorldPackage = FString()) const;

	UInvenzaSaveGame* ReadSlot(const FString& Logical, const FString& WorldPackage = FString()) const;

	UInvenzaSaveGame* ReadLatest(const FString& WorldPackage = FString()) const;

	void SetStatus(const FText& Message);

	void TryStartupLoad();

	void WorldTearingDown(UWorld* World);

	void AutoSave();

	void SaveCompleted(const FString& Slot, int32 UserIndex, bool bSuccess);

	bool CapturePickups(UInvenzaSaveGame* Save, FString& Error);

	bool PreparePickups(const UInvenzaSaveGame* Save, TArray<FInventoryPreparedPickup>& Plans, TArray<AActor*>& Spawned,
						TArray<AActor*>& DestroyAfterCommit, FString& Error);

	UFUNCTION()
	void CollectionChanged();

	UFUNCTION()
	void InventoryChanged(const FString& InventoryID);

	UFUNCTION()
	void ItemAmountChanged(int32 Amount, UObject* Item);
};
