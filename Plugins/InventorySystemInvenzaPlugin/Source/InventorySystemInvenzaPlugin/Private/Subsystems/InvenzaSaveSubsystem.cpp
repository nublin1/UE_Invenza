//  Nublin Studio 2026 All Rights Reserved.

#include "Subsystems/InvenzaSaveSubsystem.h"
#include "UObject/Package.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "Data/Settings/LinkedStorageDefinition.h"
#include "World/LinkedStorageState.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/Items/itemBase.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "SaveLoad/InvenzaSaveGame.h"
#include "SaveLoad/InvenzaSaveSettings.h"
#include "UI/SaveLoad/InvenzaSaveWidget.h"
#include "Misc/PackageName.h"
#include "TimerManager.h"
#include "Misc/App.h"

static bool IsInvenzaSaveNameValid(const FString& Name)
{
	if (Name.IsEmpty() || Name.Len() > 64)
	{
		return false;
	}
	for (TCHAR C : Name)
	{
		if (!FChar::IsAlnum(C) && C != TEXT('_') && C != TEXT('-'))
		{
			return false;
		}
	}
	return true;
}

static UInvenzaSaveGame* ReadInvenzaSaveFile(const FString& Slot, const FString& World)
{
	if (!UGameplayStatics::DoesSaveGameExist(Slot, 0))
	{
		return nullptr;
	}
	TArray<uint8> Bytes;
	if (!UGameplayStatics::LoadDataFromSlot(Bytes, Slot, 0) || Bytes.Num() < 32)
	{
		return nullptr;
	}
	uint32 Tag = 0;
	FMemory::Memcpy(&Tag, Bytes.GetData(), sizeof(Tag));
	if (Tag != 0x53415647)
	{
		return nullptr; // Engine SaveGame signature; reject truncated/foreign files before parsing.
	}
	auto* Save = Cast<UInvenzaSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
	return Save && Save->FormatVersion == 1 && Save->WorldPackage == World && Save->PlaythroughID.IsValid() ? Save
																											: nullptr;
}

void UInvenzaSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	PlaythroughID = FGuid::NewGuid();
	WorldTeardownHandle = FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &ThisClass::WorldTearingDown);
}

void UInvenzaSaveSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldBeginTearDown.Remove(WorldTeardownHandle);
	if (SessionWorld.IsValid())
	{
		SessionWorld->GetTimerManager().ClearTimer(AutoSaveTimer);
		SessionWorld->GetTimerManager().ClearTimer(StartupTimer);
	}
	Collections.Empty();
	Window = nullptr;
	Super::Deinitialize();
}

FString UInvenzaSaveSubsystem::WorldKey() const
{
	return GetWorld() ? UWorld::RemovePIEPrefix(GetWorld()->GetOutermost()->GetName()) : FString();
}

FString UInvenzaSaveSubsystem::SlotPrefix(const FString& WorldPackage) const
{
	const auto* Settings = GetDefault<UInvenzaSaveSettings>();
	const FString RequestedProfile = ProfileOverride.IsEmpty() ? Settings->Profile : ProfileOverride;
	const FString Profile = IsInvenzaSaveNameValid(RequestedProfile) ? RequestedProfile : TEXT("Default");
	const FString Prefix = GetWorld() && GetWorld()->WorldType == EWorldType::PIE ? TEXT("PIE_") : TEXT("");
	return FString::Printf(TEXT("Invenza_%s%s_%08X_"), *Prefix, *Profile,
						   FCrc::StrCrc32(*(WorldPackage.IsEmpty() ? WorldKey() : WorldPackage)));
}

FString UInvenzaSaveSubsystem::PhysicalSlot(const FString& Logical, int32 Generation, const FString& WorldPackage) const
{
	return SlotPrefix(WorldPackage) + Logical + FString::Printf(TEXT("_%d"), Generation);
}

UInvenzaSaveGame* UInvenzaSaveSubsystem::ReadSlot(const FString& Logical, const FString& WorldPackage) const
{
	const FString World = WorldPackage.IsEmpty() ? WorldKey() : WorldPackage;
	TStrongObjectPtr<UInvenzaSaveGame> A(ReadInvenzaSaveFile(PhysicalSlot(Logical, 0, World), World));
	auto* B = ReadInvenzaSaveFile(PhysicalSlot(Logical, 1, World), World);
	auto* Result = !A.IsValid() ? B : !B || A->Revision >= B->Revision ? A.Get() : B;
	if (Logical != TEXT("Reset") && Result)
	{
		TStrongObjectPtr<UInvenzaSaveGame> Reset(ReadInvenzaSaveFile(PhysicalSlot(TEXT("Reset"), 0, World), World));
		if (Reset.IsValid() && Reset->Revision > Result->Revision && Reset->PlaythroughID != Result->PlaythroughID)
		{
			return nullptr;
		}
	}
	return Result;
}

TArray<FInventorySaveSlotInfo> UInvenzaSaveSubsystem::GetSaveSlots() const
{
	return GetSaveSlotsForWorld(WorldKey());
}

TArray<FInventorySaveSlotInfo> UInvenzaSaveSubsystem::GetSaveSlotsForWorld(const FString& WorldPackage) const
{
	TArray<FInventorySaveSlotInfo> Result;
	TArray<FString> Names{TEXT("Quick"),   TEXT("Manual1"), TEXT("Manual2"),
						  TEXT("Manual3"), TEXT("Exit"),	TEXT("Shutdown")};
	for (int32 Index = 0; Index < FMath::Clamp(GetDefault<UInvenzaSaveSettings>()->AutoSaveCount, 1, 10); ++Index)
	{
		Names.Add(FString::Printf(TEXT("Auto%d"), Index));
	}
	for (const auto& Name : Names)
	{
		FInventorySaveSlotInfo Info;
		Info.Slot = Name;
		if (auto* Saved = ReadSlot(Name, WorldPackage))
		{
			Info.bAvailable = true;
			Info.Revision = Saved->Revision;
			Info.SavedAt = Saved->SavedAt;
		}
		Result.Add(Info);
	}
	return Result;
}

UInvenzaSaveGame* UInvenzaSaveSubsystem::ReadLatest(const FString& WorldPackage) const
{
	TStrongObjectPtr<UInvenzaSaveGame> Best(ReadSlot(TEXT("Reset"), WorldPackage));
	for (const auto& Slot : GetSaveSlotsForWorld(WorldPackage))
	{
		if (Slot.bAvailable && (!Best.IsValid() || Slot.Revision > Best->Revision))
		{
			Best.Reset(ReadSlot(Slot.Slot, WorldPackage));
		}
	}
	return Best.Get();
}

bool UInvenzaSaveSubsystem::LoadWorldSave(const TSoftObjectPtr<UWorld>& World, const FString& Slot)
{
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client || bBusy || World.IsNull())
	{
		return false;
	}
	const FString Target = World.ToSoftObjectPath().GetLongPackageName();
	if (Target == WorldKey() && CanManageSession())
	{
		return LoadSession(Slot);
	}
	if (CanManageSession() || !FPackageName::DoesPackageExist(Target))
	{
		return false;
	}
	TStrongObjectPtr<UInvenzaSaveGame> Selected(Slot == TEXT("Continue") ? ReadLatest(Target) : ReadSlot(Slot, Target));
	if (!Selected.IsValid() || Selected->bFreshStart)
	{
		SetStatus(GetDefault<UInvenzaSaveSettings>()->GetSaveText(TEXT("NoSave")));
		return false;
	}
	PendingTravelSave = Selected.Get();
	UGameplayStatics::OpenLevel(this, FName(Target));
	return true;
}

bool UInvenzaSaveSubsystem::CanManageSession() const
{
	return GetWorld() && GetWorld()->IsGameWorld() && !Collections.IsEmpty() && GetWorld()->GetNetMode() != NM_Client &&
		   (GetWorld()->WorldType != EWorldType::PIE || GetDefault<UInvenzaSaveSettings>()->bEnableInPIE);
}

bool UInvenzaSaveSubsystem::SetProfile(const FString& Profile)
{
	if (bBusy || !IsInvenzaSaveNameValid(Profile))
	{
		return false;
	}
	ProfileOverride = Profile;
	Revision = 0;
	PlaythroughID = FGuid::NewGuid();
	return true;
}

void UInvenzaSaveSubsystem::SetStatus(const FText& Message)
{
	Status = Message;
	OnStateChanged.Broadcast();
}

EInventoryClientTrust UInvenzaSaveSubsystem::GetClientTrust() const
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_Client)
	{
		if (auto* Player = GetGameInstance()->GetFirstLocalPlayerController())
		{
			if (APawn* Pawn = Player->GetPawn())
			{
				if (auto* Manager = Pawn->FindComponentByClass<UIInventoryManager>())
				{
					return Manager->GetSessionClientTrust();
				}
			}
		}
	}
	return bSessionTrustOverride ? SessionTrust : GetDefault<UInvenzaSaveSettings>()->ClientTrust;
}

bool UInvenzaSaveSubsystem::SetClientTrust(EInventoryClientTrust Mode)
{
	if (!CanManageSession() || bBusy ||
		(Mode != EInventoryClientTrust::ServerValidated && Mode != EInventoryClientTrust::TrustedClient))
	{
		return false;
	}
	SessionTrust = Mode;
	bSessionTrustOverride = true;
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		if (auto* Manager = It->FindComponentByClass<UIInventoryManager>())
		{
			Manager->SetSessionClientTrust(Mode);
			It->ForceNetUpdate();
		}
	}
	OnStateChanged.Broadcast();
	return true;
}

void UInvenzaSaveSubsystem::RegisterCollection(UInventoryCollectionComponent* Collection)
{
	if (!Collection || !Collection->GetOwner()->HasAuthority())
	{
		return;
	}
	if (SessionWorld.Get() != Collection->GetWorld())
	{
		Collections.Empty();
		RetainedOwners.Empty();
		RestoredOwners.Empty();
		Pickups.Empty();
		RetainedPickups.Empty();
		SessionWorld = Collection->GetWorld();
		bAutoLoadAttempted = false;
		bSuppressExitSave = false;
		LastSnapshot = nullptr;
		ChangeRevision = 0;
		SavedChangeRevision = 0;
		QueuedSaveSlots.Empty();
	}
	if (!Collection->IsRuntimeSaveActor() && !Collection->PersistentOwnerID.IsValid() && !Collection->GetOwner()->HasAnyFlags(RF_WasLoaded) &&
		!Collection->GetOwner()->IsNetStartupActor() &&
		!Collection->GetOwner()->FindComponentByClass<UIInventoryManager>())
	{
		Collection->SetRuntimeSaveIdentity(TEXT("Runtime:") + FGuid::NewGuid().ToString());
	}
	Collections.Add(Collection);
	Collection->OnActorInventoriesChanged.AddUniqueDynamic(this, &ThisClass::CollectionChanged);
	Collection->OnInventoryItemsChanged.AddUniqueDynamic(this, &ThisClass::InventoryChanged);
	MarkSessionChanged();
	NotifyPlayerReady();
}

void UInvenzaSaveSubsystem::UnregisterCollection(UInventoryCollectionComponent* Collection, EEndPlayReason::Type Reason)
{
	if (!Collection || Collection->GetWorld() != SessionWorld.Get())
	{
		return;
	}
	Collection->OnActorInventoriesChanged.RemoveDynamic(this, &ThisClass::CollectionChanged);
	Collection->OnInventoryItemsChanged.RemoveDynamic(this, &ThisClass::InventoryChanged);
	if (!bApplyingSnapshot && (Reason == EEndPlayReason::Destroyed || Reason == EEndPlayReason::RemovedFromWorld))
	{
		FInventorySavedOwner Saved;
		FString Error;
		if (Collection->CapturePersistence(Saved, Error))
		{
			Saved.bDestroyed = Reason == EEndPlayReason::Destroyed && !Saved.Key.StartsWith(TEXT("Player:"));
			if (Saved.bDestroyed)
			{
				Saved.Items.Empty();
				Saved.RecoveryItems.Empty();
			}
			RetainedOwners.Add(Saved.Key, MoveTemp(Saved));
		}
		MarkSessionChanged();
	}
	RestoredOwners.Remove(Collection->GetPersistentOwnerKey());
	Collections.Remove(Collection);
}

void UInvenzaSaveSubsystem::NotifyPlayerReady()
{
	for (const auto& Weak : Collections)
	{
		if (auto* Collection = Weak.Get())
		{
			Collection->ResolvePlayerSaveIdentity();
		}
	}
	if (GetWorld() && !GetWorld()->GetTimerManager().IsTimerActive(StartupTimer))
	{
		StartupTimer = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::TryStartupLoad);
	}
}

void UInvenzaSaveSubsystem::TryStartupLoad()
{
	if (!CanManageSession() || bBusy || bApplyingSnapshot)
	{
		return;
	}
	bool bPlayerReady = false;
	for (const auto& Weak : Collections)
	{
		auto* Collection = Weak.Get();
		if (!Collection)
		{
			continue;
		}
		if (const auto* Pawn = Cast<APawn>(Collection->GetOwner()); Pawn && Pawn->IsPlayerControlled())
		{
			bPlayerReady |= !Collection->GetActorInventories().IsEmpty();
		}
		for (auto* Inventory : Collection->GetActorInventories())
		{
			if (auto* Grid = Cast<USlotbasedInventory>(Inventory); Grid && Grid->GetInventorySlots().IsEmpty())
			{
				return;
			}
		}
	}
	if (!bPlayerReady)
	{
		return; // Player initialization will schedule another event, never a polling tick.
	}
	const auto* Settings = GetDefault<UInvenzaSaveSettings>();
	if (!bAutoLoadAttempted)
	{
		bAutoLoadAttempted = true;
		TStrongObjectPtr<UInvenzaSaveGame> Latest(ReadLatest());
		if (PendingTravelSave && PendingTravelSave->WorldPackage == WorldKey())
		{
			TStrongObjectPtr<UInvenzaSaveGame> Selected(PendingTravelSave);
			PendingTravelSave = nullptr;
			FString Error;
			if (ApplySnapshot(Selected.Get(), Error))
			{
				SetStatus(Settings->GetSaveText(TEXT("Loaded")));
				OnSessionLoaded.Broadcast();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Menu load failed: %s"), *Error);
				SetStatus(Settings->GetSaveText(TEXT("LoadFailed")));
			}
		}
		else if (Latest.IsValid() && Latest->bFreshStart)
		{
			PlaythroughID = Latest->PlaythroughID;
			Revision = Latest->Revision;
			SetStatus(GetDefault<UInvenzaSaveSettings>()->GetSaveText(TEXT("FreshStart")));
		}
		else if (Settings->bAutoLoad && Latest.IsValid())
		{
			LoadSession(TEXT("Continue"));
		}
		if (Settings->bAutoSave)
		{
			GetWorld()->GetTimerManager().SetTimer(AutoSaveTimer, this, &ThisClass::AutoSave,
												   FMath::Max(10.f, Settings->AutoSaveSeconds), true);
		}
	}
	else if (!RetainedOwners.IsEmpty())
	{
		// A streamed owner or reconnecting player may become ready after the initial restore.
		const auto ReadyCollections = Collections.Array();
		for (const auto& Weak : ReadyCollections)
		{
			auto* Collection = Weak.Get();
			if (!Collection || Collection->GetActorInventories().IsEmpty() ||
				RestoredOwners.Contains(Collection->GetPersistentOwnerKey()))
			{
				continue;
			}
			const auto* Saved = RetainedOwners.Find(Collection->GetPersistentOwnerKey());
			if (!Saved)
			{
				continue;
			}
			if (Saved->bDestroyed)
			{
				Collection->GetOwner()->Destroy();
				continue;
			}
			FInventoryPreparedOwner Plan;
			FString Error;
			if (Collection->PreparePersistence(*Saved, Plan, Error, true))
			{
				TGuardValue<bool> Guard(bApplyingSnapshot, true);
				Collection->CommitPersistence(Plan);
				RestoredOwners.Add(Saved->Key);
				Collection->NotifyPersistenceRestored();
			}
		}
	}
}

void UInvenzaSaveSubsystem::MarkSessionChanged()
{
	if (!bApplyingSnapshot)
	{
		++ChangeRevision;
	}
}

void UInvenzaSaveSubsystem::CollectionChanged()
{
	MarkSessionChanged();
	for (const auto& Weak : Collections)
	{
		if (auto* Collection = Weak.Get())
		{
			for (const auto& Entry : Collection->GetItemLocations().Items)
			{
				if (auto* Item = Cast<UItemBase>(Entry.Item))
				{
					Item->OnAmountChangedDelegate.AddUniqueDynamic(this, &ThisClass::ItemAmountChanged);
				}
			}
		}
	}
	NotifyPlayerReady();
}

void UInvenzaSaveSubsystem::InventoryChanged(const FString& InventoryID)
{
	CollectionChanged();
}
void UInvenzaSaveSubsystem::ItemAmountChanged(int32 Amount, UObject* Item)
{
	MarkSessionChanged();
}

UInvenzaSaveGame* UInvenzaSaveSubsystem::CaptureSnapshot(FString& Error)
{
	Error.Reset();
	if (!CanManageSession())
	{
		Error = TEXT("Only the host can save this session");
		return nullptr;
	}
	auto* Save = NewObject<UInvenzaSaveGame>(this);
	Save->WorldPackage = WorldKey();
	Save->PlaythroughID = PlaythroughID;
	Save->Revision = Revision + 1;
	Save->SavedAt = FDateTime::UtcNow();
	TMap<FString, FInventorySavedOwner> Owners = RetainedOwners;
	TSet<FString> LiveKeys;
	for (const auto& Weak : Collections)
	{
		auto* Collection = Weak.Get();
		if (!Collection || Collection->GetOwner()->IsActorBeingDestroyed() ||
			Collection->GetActorInventories().IsEmpty())
		{
			continue;
		}
		FInventorySavedOwner Owner;
		if (!Collection->CapturePersistence(Owner, Error))
		{
			return nullptr;
		}
		if (LiveKeys.Contains(Owner.Key))
		{
			Error = TEXT("Duplicate live owner identity");
			return nullptr;
		}
		LiveKeys.Add(Owner.Key);
		Owners.Add(Owner.Key, MoveTemp(Owner));
	}
	TSet<FGuid> Instances;
	for (const auto& Pair : Owners)
	{
		for (const auto* Items : {&Pair.Value.Items, &Pair.Value.RecoveryItems})
		{
			for (const auto& Item : *Items)
			{
				if (Instances.Contains(Item.InstanceID))
				{
					Error = TEXT("Duplicate item identity across owners");
					return nullptr;
				}
				Instances.Add(Item.InstanceID);
			}
		}
		Save->Owners.Add(Pair.Value);
	}
	if (!CapturePickups(Save, Error))
	{
		return nullptr;
	}
	for (const auto& Pickup : Save->Pickups)
	{
		if (!Pickup.bDestroyed)
		{
			if (Instances.Contains(Pickup.Item.InstanceID))
			{
				Error = TEXT("Item is present in both a pickup and an inventory");
				return nullptr;
			}
			Instances.Add(Pickup.Item.InstanceID);
		}
	}
	return Save;
}

bool UInvenzaSaveSubsystem::SaveSession(const FString& Slot, bool bAsync)
{
	const auto Available = GetSaveSlots();
	if (Slot == TEXT("Shutdown") || !CanManageSession() ||
		!Available.ContainsByPredicate(
			[&](const auto& Info)
			{
				return Info.Slot == Slot;
			}))
	{
		return false;
	}
	if (bBusy)
	{
		QueuedSaveSlots.AddUnique(Slot);
		return true;
	}
	FString Error;
	if (auto* Latest = ReadLatest())
	{
		Revision = FMath::Max(Revision, Latest->Revision);
	}
	PendingWrite = CaptureSnapshot(Error);
	if (!PendingWrite)
	{
		UE_LOG(LogTemp, Warning, TEXT("Save capture failed: %s"), *Error);
		SetStatus(GetDefault<UInvenzaSaveSettings>()->GetSaveText(TEXT("SaveFailed")));
		return false;
	}
	TStrongObjectPtr<UInvenzaSaveGame> A(ReadInvenzaSaveFile(PhysicalSlot(Slot, 0), WorldKey()));
	const auto* B = ReadInvenzaSaveFile(PhysicalSlot(Slot, 1), WorldKey());
	const int32 Generation = !A.IsValid() || (B && A->Revision <= B->Revision) ? 0 : 1;
	PendingPhysicalSlot = PhysicalSlot(Slot, Generation);
	CapturedChangeRevision = ChangeRevision;
	PendingWriteWorld = GetWorld();
	bBusy = true;
	SetStatus(GetDefault<UInvenzaSaveSettings>()->GetSaveText(TEXT("Saving")));
	if (bAsync)
	{
		UGameplayStatics::AsyncSaveGameToSlot(
			PendingWrite, PendingPhysicalSlot, 0,
			FAsyncSaveGameToSlotDelegate::CreateUObject(this, &ThisClass::SaveCompleted));
	}
	else
	{
		const bool bSuccess = UGameplayStatics::SaveGameToSlot(PendingWrite, PendingPhysicalSlot, 0);
		SaveCompleted(PendingPhysicalSlot, 0, bSuccess);
		return bSuccess;
	}
	return true;
}

void UInvenzaSaveSubsystem::SaveCompleted(const FString& Slot, int32 UserIndex, bool bSuccess)
{
	if (Slot != PendingPhysicalSlot || !PendingWrite)
	{
		return;
	}
	if (PendingWriteWorld.Get() != GetWorld())
	{
		PendingWrite = nullptr;
		bBusy = false;
		QueuedSaveSlots.Empty();
		NotifyPlayerReady();
		return;
	}
	if (bSuccess)
	{
		Revision = PendingWrite->Revision;
		SavedChangeRevision = CapturedChangeRevision;
	}
	PendingWrite = nullptr;
	bBusy = false;
	SetStatus(bSuccess ? GetDefault<UInvenzaSaveSettings>()->GetSaveText(TEXT("Saved"))
					   : GetDefault<UInvenzaSaveSettings>()->GetSaveText(TEXT("SaveFailed")));
	if (!QueuedSaveSlots.IsEmpty())
	{
		const FString Next = QueuedSaveSlots[0];
		QueuedSaveSlots.RemoveAt(0);
		SaveSession(Next, true);
	}
}

void UInvenzaSaveSubsystem::AutoSave()
{
	if (ChangeRevision == SavedChangeRevision || !CanManageSession())
	{
		return;
	}
	const int32 Count = FMath::Clamp(GetDefault<UInvenzaSaveSettings>()->AutoSaveCount, 1, 10);
	SaveSession(FString::Printf(TEXT("Auto%d"), AutoSaveIndex++ % Count), true);
}

bool UInvenzaSaveSubsystem::SaveBeforeExit()
{
	if (!CanManageSession() || !GetDefault<UInvenzaSaveSettings>()->bSaveOnExit)
	{
		return true;
	}
	if (bBusy)
	{
		SetStatus(GetDefault<UInvenzaSaveSettings>()->GetSaveText(TEXT("WaitBeforeExit")));
		return false;
	}
	const bool bSuccess = SaveSession(TEXT("Exit"), false);
	if (bSuccess)
	{
		bSuppressExitSave = true;
	}
	return bSuccess;
}

bool UInvenzaSaveSubsystem::LoadSession(const FString& Slot)
{
	if (!CanManageSession() || bBusy)
	{
		return false;
	}
	TStrongObjectPtr<UInvenzaSaveGame> Save(Slot == TEXT("Continue") ? ReadLatest() : ReadSlot(Slot));
	if (!Save.IsValid() || Save->bFreshStart)
	{
		SetStatus(GetDefault<UInvenzaSaveSettings>()->GetSaveText(TEXT("NoSave")));
		return false;
	}
	FString Error;
	const bool bSuccess = ApplySnapshot(Save.Get(), Error);
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session load rejected: %s"), *Error);
	}
	if (bSuccess && !GetRecoveryItems().IsEmpty())
	{
		SetStatus(FText::Format(GetDefault<UInvenzaSaveSettings>()->GetSaveText(TEXT("LoadedRecovery")),
								GetRecoveryItems().Num()));
	}
	else
	{
		SetStatus(bSuccess ? GetDefault<UInvenzaSaveSettings>()->GetSaveText(TEXT("Loaded"))
						   : GetDefault<UInvenzaSaveSettings>()->GetSaveText(TEXT("LoadFailed")));
	}
	if (bSuccess)
	{
		OnSessionLoaded.Broadcast();
	}
	return bSuccess;
}

bool UInvenzaSaveSubsystem::ApplySnapshot(const UInvenzaSaveGame* Snapshot, FString& Error)
{
	Error.Reset();
	if (!CanManageSession() || bApplyingSnapshot || !Snapshot || Snapshot->FormatVersion != 1 ||
		Snapshot->WorldPackage != WorldKey() || !Snapshot->PlaythroughID.IsValid())
	{
		Error = TEXT("Incompatible snapshot or unauthorized load");
		return false;
	}
	TSet<FString> OwnerKeys;
	TSet<FGuid> InstanceIDs;
	for (const auto& Saved : Snapshot->Owners)
	{
		if (Saved.Key.IsEmpty() || OwnerKeys.Contains(Saved.Key))
		{
			Error = TEXT("Duplicate saved owner");
			return false;
		}
		OwnerKeys.Add(Saved.Key);
		for (const auto* Items : {&Saved.Items, &Saved.RecoveryItems})
		{
			for (const auto& Item : *Items)
			{
				if (!Item.InstanceID.IsValid() || InstanceIDs.Contains(Item.InstanceID) || Item.Quantity <= 0)
				{
					Error = TEXT("Invalid or duplicate saved item");
					return false;
				}
				InstanceIDs.Add(Item.InstanceID);
			}
		}
	}
	TGuardValue<bool> Guard(bApplyingSnapshot, true);
	TMap<FString, UInventoryCollectionComponent*> Current;
	for (const auto& Weak : Collections)
	{
		if (auto* Collection = Weak.Get())
		{
			const FString Key = Collection->GetPersistentOwnerKey();
			if (Current.Contains(Key))
			{
				Error = TEXT("Duplicate live owner identity: ") + Key;
				return false;
			}
			Current.Add(Key, Collection);
		}
	}
	TArray<AActor*> Spawned;
	TArray<AActor*> DestroyAfterCommit;
	TArray<FInventoryPreparedOwner> Plans;
	auto RollbackSpawns = [&]()
	{
		for (auto* Actor : Spawned)
		{
			Actor->Destroy();
		}
	};
	for (const auto& Saved : Snapshot->Owners)
	{
		auto* Collection = Current.FindRef(Saved.Key);
		if (Saved.bDestroyed)
		{
			if (Collection)
			{
				DestroyAfterCommit.Add(Collection->GetOwner());
			}
			continue;
		}
		bool bLevelLoaded = Saved.LevelPackage.IsEmpty();
		for (auto* Level : GetWorld()->GetLevels())
		{
			if (UWorld::RemovePIEPrefix(Level->GetOutermost()->GetName()) == Saved.LevelPackage)
			{
				bLevelLoaded = true;
			}
		}
		if (!Collection && !Saved.Key.StartsWith(TEXT("Player:")) && (Saved.bRuntimeActor || bLevelLoaded))
		{
			UClass* Class = Saved.ActorClass.TryLoadClass<AActor>();
			if (!Class || Class->HasAnyClassFlags(CLASS_Abstract))
			{
				Error = TEXT("Saved actor class is unavailable");
				RollbackSpawns();
				return false;
			}
			FActorSpawnParameters Parameters;
			Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			auto* Actor = GetWorld()->SpawnActor<AActor>(Class, Saved.Transform, Parameters);
			Collection = Actor ? Actor->FindComponentByClass<UInventoryCollectionComponent>() : nullptr;
			if (Actor)
			{
				Spawned.Add(Actor);
			}
			if (!Collection)
			{
				Error = TEXT("Saved actor has no inventory collection");
				RollbackSpawns();
				return false;
			}
			Collection->SetRuntimeSaveIdentity(Saved.Key);
			if (auto* Storage = Cast<ALinkedStorageState>(Actor))
			{
				Storage->Definition = Cast<ULinkedStorageDefinition>(Saved.LinkedStorageDefinition.TryLoad());
				if (!Storage->Definition)
				{
					Error = TEXT("Linked storage definition is unavailable");
					RollbackSpawns();
					return false;
				}
			}
			for (const auto& SavedInventory : Saved.Inventories)
			{
				bool bExists = false;
				for (auto* Existing : Collection->GetActorInventories())
				{
					if (Collection->GetPersistentInventoryKey(Existing) == SavedInventory.Key)
					{
						bExists = true;
					}
				}
				if (bExists)
				{
					continue;
				}
				FInventoryStartupData Startup;
				Startup.Settings = SavedInventory.StartupSettings;
				auto* Inventory = UInventoryBase::CreateInventoryAdvanced(Actor, Startup, Actor, Collection);
				if (Inventory)
				{
					Inventory->SetInventorySize(SavedInventory.Size);
					Collection->AddPawnInventory_Internal(Inventory);
					if (auto* Grid = Cast<USlotbasedInventory>(Inventory))
					{
						Grid->RebuildInventory();
					}
				}
			}
		}
		if (!Collection || Collection->GetActorInventories().IsEmpty())
		{
			continue;
		}
		// Retained for streaming or player reconnect.
		auto& Plan = Plans.AddDefaulted_GetRef();
		if (!Collection->PreparePersistence(Saved, Plan, Error, true))
		{
			RollbackSpawns();
			return false;
		}
	}
	for (const auto& Pair : Current)
	{
		if (Pair.Value->IsRuntimeSaveActor() && !OwnerKeys.Contains(Pair.Key))
		{
			DestroyAfterCommit.Add(Pair.Value->GetOwner());
		}
	}
	TArray<FInventoryPreparedPickup> PickupPlans;
	if (!PreparePickups(Snapshot, PickupPlans, Spawned, DestroyAfterCommit, Error))
	{
		RollbackSpawns();
		return false;
	}
	// All data is prepared. Close interactions before replacing references held by their widgets.
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		if (auto* Interaction = It->FindComponentByClass<UInteractionComponent>())
		{
			Interaction->StopInteract();
		}
	}
	for (auto& Plan : Plans)
	{
		if (!IsValid(Plan.Collection) || Plan.Collection->GetOwner()->IsActorBeingDestroyed())
		{
			Error = TEXT("Inventory owner was destroyed while closing an interaction");
			RollbackSpawns();
			return false;
		}
	}
	for (auto& Plan : Plans)
	{
		Plan.Collection->CommitPersistence(Plan);
	}
	for (auto& Plan : PickupPlans)
	{
		Plan.Component->RestorePersistentItem(Plan.Item.Get());
	}
	RetainedPickups.Empty();
	for (const auto& Saved : Snapshot->Pickups)
	{
		RetainedPickups.Add(Saved.Key, Saved);
	}
	RetainedOwners.Empty();
	RestoredOwners.Empty();
	for (const auto& Saved : Snapshot->Owners)
	{
		RetainedOwners.Add(Saved.Key, Saved);
	}
	for (auto& Plan : Plans)
	{
		RestoredOwners.Add(Plan.Collection->GetPersistentOwnerKey());
	}
	for (auto* Actor : DestroyAfterCommit)
	{
		Actor->Destroy();
	}
	for (auto& Plan : Plans)
	{
		Plan.Collection->NotifyPersistenceRestored();
	}
	LastSnapshot = DuplicateObject<UInvenzaSaveGame>(Snapshot, this);
	PlaythroughID = Snapshot->PlaythroughID;
	Revision = FMath::Max(Revision, Snapshot->Revision);
	SavedChangeRevision = ChangeRevision;
	return true;
}

bool UInvenzaSaveSubsystem::ApplyTrustedOwner(AActor* Requestor, const FInventorySavedOwner& Owner, FString& Error)
{
	Error.Reset();
	if (!CanManageSession() || bBusy || bApplyingSnapshot || GetClientTrust() != EInventoryClientTrust::TrustedClient ||
		!IsValid(Requestor))
	{
		Error = TEXT("Client state submission is disabled");
		return false;
	}
	auto* Collection = Requestor->FindComponentByClass<UInventoryCollectionComponent>();
	if (!Collection || Owner.Key != Collection->GetPersistentOwnerKey() || Owner.bDestroyed ||
		Owner.Items.Num() + Owner.RecoveryItems.Num() > 1024 ||
		Owner.Inventories.Num() != Collection->GetActorInventories().Num())
	{
		Error = TEXT("Client can replace only its own registered inventory state");
		return false;
	}
	for (const auto& Saved : Owner.Inventories)
	{
		bool bMatches = false;
		for (auto* Inventory : Collection->GetActorInventories())
		{
			if (Collection->GetPersistentInventoryKey(Inventory) == Saved.Key &&
				Inventory->GetInventorySize() == Saved.Size)
			{
				bMatches = true;
			}
		}
		if (!bMatches)
		{
			Error = TEXT("Client cannot change inventory definitions or dimensions");
			return false;
		}
	}
	TSet<FGuid> OtherIDs;
	for (const auto& Weak : Collections)
	{
		if (auto* Other = Weak.Get(); Other && Other != Collection)
		{
			for (const auto& Entry : Other->GetItemLocations().Items)
			{
				if (auto* Item = Cast<UItemBase>(Entry.Item))
				{
					OtherIDs.Add(Item->GetPersistentInstanceID());
				}
			}
		}
	}
	for (const auto& Pair : RetainedOwners)
	{
		if (Pair.Key != Owner.Key)
		{
			for (const auto* Items : {&Pair.Value.Items, &Pair.Value.RecoveryItems})
			{
				for (const auto& Item : *Items)
				{
					OtherIDs.Add(Item.InstanceID);
				}
			}
		}
	}
	for (const auto& Weak : Pickups)
	{
		if (auto* Pickup = Weak.Get())
		{
			if (auto* Item = Cast<UItemBase>(Pickup->PeekItemData()))
			{
				OtherIDs.Add(Item->GetPersistentInstanceID());
			}
		}
	}
	for (const auto& Pair : RetainedPickups)
	{
		if (!Pair.Value.bDestroyed)
		{
			OtherIDs.Add(Pair.Value.Item.InstanceID);
		}
	}
	for (const auto* Items : {&Owner.Items, &Owner.RecoveryItems})
	{
		for (const auto& Item : *Items)
		{
			if (OtherIDs.Contains(Item.InstanceID) || Item.CustomState.Num() > 64 || Item.Locations.Num() > 32)
			{
				Error = TEXT("Client submitted foreign item identity or excessive data");
				return false;
			}
			for (const auto& Pair : Item.CustomState)
			{
				if (Pair.Value.Len() > 4096)
				{
					Error = TEXT("Client custom state is too large");
					return false;
				}
			}
		}
	}
	FInventorySavedOwner DataToApply = Owner;
	FInventorySavedOwner CurrentState;
	if (!Collection->CapturePersistence(CurrentState, Error))
	{
		return false;
	}
	DataToApply.Crafting = CurrentState.Crafting;
	FInventoryPreparedOwner Plan;
	if (!Collection->PreparePersistence(DataToApply, Plan, Error, true))
	{
		return false;
	}
	// Trusted state must be complete; never silently quarantine an invalid network submission.
	if (!Plan.RecoveryItems.IsEmpty())
	{
		Error = TEXT("Client state contains unavailable definitions or invalid placements");
		return false;
	}
	{
		TGuardValue<bool> Guard(bApplyingSnapshot, true);
		Collection->CommitPersistence(Plan);
		Collection->NotifyPersistenceRestored();
	}
	MarkSessionChanged();
	return true;
}

bool UInvenzaSaveSubsystem::ResetPlaythrough()
{
	if (!CanManageSession() || bBusy)
	{
		return false;
	}
	TStrongObjectPtr<UInvenzaSaveGame> Latest(ReadLatest());
	auto* Reset = NewObject<UInvenzaSaveGame>(this);
	Reset->WorldPackage = WorldKey();
	Reset->PlaythroughID = FGuid::NewGuid();
	Reset->bFreshStart = true;
	Reset->Revision = FMath::Max(Revision, Latest.IsValid() ? Latest->Revision : 0) + 1;
	Reset->SavedAt = FDateTime::UtcNow();
	// A durable new-generation marker makes old autosaves ineligible even if deletion fails.
	if (!UGameplayStatics::SaveGameToSlot(Reset, PhysicalSlot(TEXT("Reset"), 0), 0))
	{
		SetStatus(GetDefault<UInvenzaSaveSettings>()->GetSaveText(TEXT("ResetFailed")));
		return false;
	}
	for (const auto& Info : GetSaveSlots())
	{
		for (int32 Generation = 0; Generation < 2; ++Generation)
		{
			UGameplayStatics::DeleteGameInSlot(PhysicalSlot(Info.Slot, Generation), 0);
		}
	}
	bSuppressExitSave = true;
	GetWorld()->GetTimerManager().ClearTimer(AutoSaveTimer);
	const FString Map = WorldKey();
	if (GetWorld()->GetNetMode() == NM_ListenServer || GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		GetWorld()->ServerTravel(Map + TEXT("?listen"));
	}
	else
	{
		UGameplayStatics::OpenLevel(this, FName(Map));
	}
	return true;
}

void UInvenzaSaveSubsystem::ShowSaveMenu(APlayerController* Player)
{
	if (!Player || !Player->IsLocalController())
	{
		return;
	}
	if (IsValid(Window) && Window->IsInViewport())
	{
		Window->RemoveFromParent();
		Window = nullptr;
		return;
	}
	const auto* Settings = GetDefault<UInvenzaSaveSettings>();
	const auto Class = Settings->SaveMenuClass.LoadSynchronous();
	if (!Class)
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveMenuClass is not configured in InvenzaSaveSettings"));
		return;
	}
	Window = CreateWidget<UInvenzaSaveWidget>(Player, Class);
	if (Window)
	{
		Window->AddToViewport(100);
	}
}

void UInvenzaSaveSubsystem::WorldTearingDown(UWorld* World)
{
	if (World == SessionWorld.Get() && Window)
	{
		Window->RemoveFromParent();
		Window = nullptr;
	}
	if (World != SessionWorld.Get() || bSuppressExitSave || !CanManageSession() ||
		!GetDefault<UInvenzaSaveSettings>()->bSaveOnExit || FApp::IsUnattended())
	{
		return;
	}
	FString Error;
	TStrongObjectPtr<UInvenzaSaveGame> Save(CaptureSnapshot(Error));
	if (!Save.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Exit snapshot failed: %s"), *Error);
		return;
	}
	if (PendingWrite)
	{
		Save->Revision = FMath::Max(Save->Revision, PendingWrite->Revision + 1);
	}
	TStrongObjectPtr<UInvenzaSaveGame> A(ReadInvenzaSaveFile(PhysicalSlot(TEXT("Shutdown"), 0), WorldKey()));
	const auto* B = ReadInvenzaSaveFile(PhysicalSlot(TEXT("Shutdown"), 1), WorldKey());
	const int32 Generation = !A.IsValid() || (B && A->Revision <= B->Revision) ? 0 : 1;
	// A teardown cannot wait for a game-thread callback. Use an independent exit file.
	if (!UGameplayStatics::SaveGameToSlot(Save.Get(), PhysicalSlot(TEXT("Shutdown"), Generation), 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to write exit snapshot"));
	}
}

TArray<FInventorySavedItem> UInvenzaSaveSubsystem::GetRecoveryItems() const
{
	TArray<FInventorySavedItem> Result;
	TSet<FString> Live;
	for (const auto& Weak : Collections)
	{
		if (auto* Collection = Weak.Get())
		{
			Live.Add(Collection->GetPersistentOwnerKey());
			Result.Append(Collection->GetRecoveryItems());
		}
	}
	for (const auto& Pair : RetainedOwners)
	{
		if (!Live.Contains(Pair.Key))
		{
			Result.Append(Pair.Value.RecoveryItems);
		}
	}
	return Result;
}

bool UInvenzaSaveSubsystem::RetryRecovery()
{
	if (!CanManageSession() || bBusy)
	{
		return false;
	}
	FString Error;
	TStrongObjectPtr<UInvenzaSaveGame> Snapshot(CaptureSnapshot(Error));
	if (!Snapshot.IsValid() || !ApplySnapshot(Snapshot.Get(), Error))
	{
		UE_LOG(LogTemp, Warning, TEXT("Recovery failed: %s"), *Error);
		SetStatus(GetDefault<UInvenzaSaveSettings>()->GetSaveText(TEXT("RecoveryFailed")));
		return false;
	}
	SetStatus(FText::Format(GetDefault<UInvenzaSaveSettings>()->GetSaveText(TEXT("RecoveryRemaining")),
							GetRecoveryItems().Num()));
	return true;
}
