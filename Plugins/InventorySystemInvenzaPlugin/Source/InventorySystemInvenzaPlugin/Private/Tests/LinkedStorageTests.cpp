//  Nublin Studio 2026 All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "ActorComponents/Interactable/LinkedStorageComponent.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/LinkedStorageIdentityComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/ItemData.h"
#include "Data/Items/itemBase.h"
#include "Data/Settings/LinkedStorageDefinition.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Factory/ItemFactory.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "SaveLoad/InvenzaSaveGame.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "Subsystems/LinkedStorageSubsystem.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"
#include "World/LinkedStorageChest.h"
#include "World/LinkedStorageState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvenzaLinkedStorageTest, "Invenza.Storage.LinkedStorage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

static APawn* CreateLinkedStorageTestPlayer(UWorld* World, const FString& ID)
{
	auto* PC = World->SpawnActor<APlayerController>();
	auto* Pawn = World->SpawnActor<APawn>();
	auto* Root = NewObject<USceneComponent>(Pawn);
	Pawn->SetRootComponent(Root);
	Root->RegisterComponent();
	PC->Possess(Pawn);
	auto* Identity = NewObject<ULinkedStorageIdentityComponent>(PC);
	PC->AddInstanceComponent(Identity);
	Identity->RegisterComponent();
	Identity->SetPersistentPlayerID(ID);
	return Pawn;
}

static ALinkedStorageChest* CreateLinkedStorageTestChest(UWorld* World, ULinkedStorageDefinition* Definition,
	ELinkedStorageAccessMode Mode)
{
	auto* Chest = World->SpawnActor<ALinkedStorageChest>();
	Chest->LinkedStorage->StorageDefinition = Definition;
	Chest->LinkedStorage->StorageAccessMode = Mode;
	return Chest;
}

static void OpenLinkedStorageTestSession(UIInventoryManager* Manager, UInteractableComponent* Chest)
{
	struct FOpenLinkedStorageTestParameters
	{
		UInteractableComponent* Target;
		EInteractionType Type;
	};
	FOpenLinkedStorageTestParameters Data{Chest, EInteractionType::Primary};
	Manager->ProcessEvent(Manager->FindFunctionChecked(TEXT("Server_HandleInteract")), &Data);
}

bool FInvenzaLinkedStorageTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UGameInstance> Instance(NewObject<UGameInstance>(GEngine));
	Instance->InitializeStandalone();
	UWorld* World = Instance->GetWorld();
	if (!TestNotNull(TEXT("Isolated game world"), World))
	{
		return false;
	}
	auto* Registry = World->GetSubsystem<ULinkedStorageSubsystem>();
	auto* Save = Instance->GetSubsystem<UInvenzaSaveSubsystem>();
	Save->SetProfile(TEXT("LinkedStorageAutomation"));
	auto* A = CreateLinkedStorageTestPlayer(World, TEXT("Account-A"));
	auto* B = CreateLinkedStorageTestPlayer(World, TEXT("Account-B"));
	TStrongObjectPtr<ULinkedStorageDefinition> Definition(NewObject<ULinkedStorageDefinition>(GetTransientPackage()));
	Definition->StorageGroupID = TEXT("LinkedStorageAutomation");
	Definition->Inventory.Settings.InventoryClass = USlotbasedInventory::StaticClass();
	Definition->Inventory.Settings.bCollectInvDataFromWidget = false;
	Definition->Inventory.Settings.InventorySlotBasedSettings.InitNumberRows = 4;
	Definition->Inventory.Settings.InventorySlotBasedSettings.InitNumColumns = 4;
	TStrongObjectPtr<UDataTable> Table(NewObject<UDataTable>(GetTransientPackage()));
	Table->RowStruct = FItemData::StaticStruct();
	FItemData Row;
	Row.ID = TEXT("LinkedStorageTestItem");
	Row.ItemMetaData.ItemNumeraticData.MaxStackSizeInCharacter = 100;
	Row.ItemMetaData.ItemCategory = Instance->GetSubsystem<UInvenzaInventorySettingsSubsystem>()->GetSettings()->AnyCategoryGameplayTag;
	Table->AddRow(Row.ID, Row);
	FInitItemsEntry Initial;
	Initial.Item.DataTable = Table.Get();
	Initial.Item.RowName = Row.ID;
	Initial.Amount = 7;
	Definition->Inventory.StartItems.Add(Initial);
	auto* ChestA = CreateLinkedStorageTestChest(World, Definition.Get(), ELinkedStorageAccessMode::Shared);
	auto* ChestB = CreateLinkedStorageTestChest(World, Definition.Get(), ELinkedStorageAccessMode::Shared);
	TestTrue(TEXT("Player A acquires shared group"), ChestA->LinkedStorage->TryAcquireInteraction(A));
	const auto Entries = ChestA->LinkedStorage->GetInventoriesForPlayer(A);
	if (!TestEqual(TEXT("Exactly one physical inventory"), Entries.Num(), 1))
	{
		Instance->Shutdown();
		return false;
	}
	auto* Inventory = Entries[0].Inventory.Get();
	auto* Shared = Cast<ALinkedStorageState>(Inventory->GetInventoryOwnerActor());
	TestNotNull(TEXT("Dedicated physical owner"), Shared);
	TestTrue(TEXT("Another chest is locked for another player"), ChestB->LinkedStorage->IsInteractionLockedByOther(B));
	TestFalse(TEXT("Other player cannot acquire through another chest"), ChestB->LinkedStorage->TryAcquireInteraction(B));
	TestFalse(TEXT("Other player cannot acquire through same chest"), ChestA->LinkedStorage->TryAcquireInteraction(B));
	TestEqual(TEXT("Blocked player receives no catalog"), ChestB->LinkedStorage->GetInventoriesForPlayer(B).Num(), 0);
	TestFalse(TEXT("Blocked player cannot operate on a known inventory"), ChestB->LinkedStorage->ContainsInventoryForPlayer(Inventory, B));
	const auto InitialItems = Shared->Collection->GetAllItemsByContainer(Inventory->GetInventoryContainerID());
	if (!TestEqual(TEXT("Initial stack issued once"), InitialItems.Num(), 1))
	{
		Instance->Shutdown();
		return false;
	}
	TestEqual(TEXT("Exact initial quantity"), IObjectDataProvider::Execute_GetQuantity(InitialItems[0]), 7);
	Shared->AddViewer(A);
	TestTrue(TEXT("Authorized connection receives storage"), static_cast<AActor*>(Shared)->IsNetRelevantFor(A->GetController(), A, FVector::ZeroVector));
	TestFalse(TEXT("Other connection receives no contents even nearby"), static_cast<AActor*>(Shared)->IsNetRelevantFor(B->GetController(), B, FVector::ZeroVector));
	ChestA->LinkedStorage->ReleaseInteraction(A);
	TestFalse(TEXT("Release revokes content replication"), static_cast<AActor*>(Shared)->IsNetRelevantFor(A->GetController(), A, FVector::ZeroVector));
	TestTrue(TEXT("Group unlocks after closing"), ChestB->LinkedStorage->TryAcquireInteraction(B));
	TestEqual(TEXT("Other chest opens identical physical inventory"), ChestB->LinkedStorage->GetInventoriesForPlayer(B)[0].Inventory.Get(), Inventory);
	TestEqual(TEXT("Opening another chest does not refill"), Shared->Collection->GetAllItemsByContainer(Inventory->GetInventoryContainerID()).Num(), 1);
	ChestB->LinkedStorage->ReleaseInteraction(B);

	// A remote access point has no impact on the physical owner's lifetime.
	ChestA->Destroy();
	TestEqual(TEXT("Chest destruction preserves storage"), Registry->FindStorage(Shared->Collection->GetPersistentOwnerKey()), Shared);
	TestTrue(TEXT("Remaining chest can reopen"), ChestB->LinkedStorage->TryAcquireInteraction(A));
	A->SetActorLocation(FVector(100000.f, 0.f, 0.f));
	TestFalse(TEXT("Out of range lease cannot authorize item operations"), ChestB->LinkedStorage->ContainsInventoryForPlayer(Inventory, A));
	TestTrue(TEXT("Out of range owner does not permanently lock group"), ChestB->LinkedStorage->TryAcquireInteraction(B));
	ChestB->LinkedStorage->ReleaseInteraction(A);
	TestTrue(TEXT("Stale release cannot unlock successor's lease"), ChestB->LinkedStorage->IsInteractionOwnedBy(B));
	ChestB->LinkedStorage->ReleaseInteraction(B);
	A->SetActorLocation(FVector::ZeroVector);
	APlayerController* OriginalAController = Cast<APlayerController>(A->GetController());
	APlayerController* OriginalBController = Cast<APlayerController>(B->GetController());
	TestTrue(TEXT("Lease before possession replacement"), ChestB->LinkedStorage->TryAcquireInteraction(A));
	Shared->AddViewer(A);
	OriginalBController->Possess(A);
	TestFalse(TEXT("New controller cannot inherit previous connection's contents"),
		static_cast<AActor*>(Shared)->IsNetRelevantFor(OriginalBController, A, FVector::ZeroVector));
	TestFalse(TEXT("Possession replacement revokes operations"), ChestB->LinkedStorage->ContainsInventoryForPlayer(Inventory, A));
	ChestB->LinkedStorage->ReleaseInteraction(A);
	OriginalBController->Possess(B);
	OriginalAController->Possess(A);

	auto* PersonalA = CreateLinkedStorageTestChest(World, Definition.Get(), ELinkedStorageAccessMode::Personal);
	auto* PersonalB = CreateLinkedStorageTestChest(World, Definition.Get(), ELinkedStorageAccessMode::Personal);
	TestTrue(TEXT("A opens personal storage"), PersonalA->LinkedStorage->TryAcquireInteraction(A));
	TestTrue(TEXT("B opens independent personal storage through same chest"), PersonalA->LinkedStorage->TryAcquireInteraction(B));
	auto* AInventory = PersonalA->LinkedStorage->GetInventoriesForPlayer(A)[0].Inventory.Get();
	auto* BInventory = PersonalA->LinkedStorage->GetInventoriesForPlayer(B)[0].Inventory.Get();
	TestNotEqual(TEXT("Personal inventories differ"), AInventory, BInventory);
	TestNotEqual(TEXT("Shared and personal modes differ"), Inventory, AInventory);
	TestFalse(TEXT("B cannot access A's known private inventory"), PersonalA->LinkedStorage->ContainsInventoryForPlayer(AInventory, B));
	PersonalA->LinkedStorage->ReleaseInteraction(A);
	TestTrue(TEXT("A opens second personal chest"), PersonalB->LinkedStorage->TryAcquireInteraction(A));
	TestEqual(TEXT("Personal contents available through another chest"), PersonalB->LinkedStorage->GetInventoriesForPlayer(A)[0].Inventory.Get(), AInventory);
	PersonalB->LinkedStorage->ReleaseInteraction(A);
	PersonalA->LinkedStorage->ReleaseInteraction(B);
	auto* ReconnectedA = CreateLinkedStorageTestPlayer(World, TEXT("Account-A"));
	TestTrue(TEXT("Reconnected account opens personal storage"), PersonalB->LinkedStorage->TryAcquireInteraction(ReconnectedA));
	TestEqual(TEXT("Personal identity survives controller replacement"),
		PersonalB->LinkedStorage->GetInventoriesForPlayer(ReconnectedA)[0].Inventory.Get(), AInventory);
	PersonalB->LinkedStorage->ReleaseInteraction(ReconnectedA);

	TStrongObjectPtr<ULinkedStorageDefinition> OtherDefinition(DuplicateObject<ULinkedStorageDefinition>(Definition.Get(), GetTransientPackage()));
	OtherDefinition->StorageGroupID = TEXT("OtherLinkedStorageAutomation");
	auto* Other = Registry->FindOrCreate(OtherDefinition.Get(), ELinkedStorageAccessMode::Shared, A);
	TestNotEqual(TEXT("Groups are independent"), Other, Shared);
	TStrongObjectPtr<ULinkedStorageDefinition> Conflict(DuplicateObject<ULinkedStorageDefinition>(Definition.Get(), GetTransientPackage()));
	TestNull(TEXT("Conflicting definition for same group rejected"), Registry->FindOrCreate(Conflict.Get(), ELinkedStorageAccessMode::Shared, A));
	TestTrue(TEXT("Empty group rejected"), [&]()
	{
		Conflict->StorageGroupID = NAME_None;
		return Registry->MakeStorageKey(Conflict.Get(), ELinkedStorageAccessMode::Shared, A).IsEmpty();
	}());

	// Empty inventory must remain empty after save/load, including offline personal owners.
	Shared->Collection->RemoveItemFromAllContainers(InitialItems[0]);
	for (auto* Storage : {Shared, Cast<ALinkedStorageState>(AInventory->GetInventoryOwnerActor()),
		Cast<ALinkedStorageState>(BInventory->GetInventoryOwnerActor()), Other})
	{
		Save->RegisterCollection(Storage->Collection);
	}
	auto* Anchor = NewObject<UInventoryCollectionComponent>(A);
	A->AddInstanceComponent(Anchor);
	Anchor->RegisterComponent();
	Anchor->InitItemCollection();
	Anchor->SetRuntimeSaveIdentity(TEXT("Player:LinkedStorageTestAnchor"));
	FInventoryStartupData AnchorStartup = Definition->Inventory;
	AnchorStartup.Settings.bCollectInvDataFromWidget = false;
	Anchor->AddPawnInventory_Internal(UInventoryBase::CreateInventoryAdvanced(A, AnchorStartup, A, Anchor));
	Save->RegisterCollection(Anchor);

	// Exercise the real server session and inventory interaction interface, including ID resolution.
	auto* Manager = NewObject<UIInventoryManager>(A);
	A->AddInstanceComponent(Manager);
	Manager->RegisterComponent();
	FindFProperty<FObjectPropertyBase>(UIInventoryManager::StaticClass(), TEXT("ItemCollectionRef"))->SetObjectPropertyValue_InContainer(Manager, Anchor);
	Anchor->InitItemCollection();
	OpenLinkedStorageTestSession(Manager, ChestB->LinkedStorage);
	TestTrue(TEXT("Manager session authorizes shared inventory"), Manager->IsInventoryAccessible(Inventory));
	TestEqual(TEXT("Inventory ID resolves through storage session"), Anchor->GetInventoryByID(Inventory->GetInventoryContainerID()), Inventory);
	TestTrue(TEXT("Opening through manager enables content replication"), Shared->HasViewer(A));
	auto* MoveItem = UItemFactory::CreateItemByHandle(Shared, Initial.Item, 5);
	FItemMoveData Move;
	Move.SourceItem = MoveItem;
	Move.TargetInventory = Inventory;
	Inventory->HandleAddItem(Move);
	Move.SourceInventory = Inventory;
	Move.TargetInventory = Anchor->GetActorInventories()[0];
	IInventoryInteractionHandler::Execute_ItemTransferRequest(Manager, Move);
	TestEqual(TEXT("Transfer withdraws from physical storage"), Shared->Collection->GetItemLocations().Items.Num(), 0);
	TestEqual(TEXT("Transfer deposits in player inventory"), Anchor->GetItemLocations().Items.Num(), 1);
	Move.SourceItem = Anchor->GetItemLocations().Items[0].Item;
	TestEqual(TEXT("Withdrawn item lives under the player actor"), Move.SourceItem->GetTypedOuter<AActor>(), static_cast<AActor*>(A));
	TestEqual(TEXT("Full transfer preserves persistent item identity"), CastChecked<UItemBase>(Move.SourceItem)->GetPersistentInstanceID(), MoveItem->GetPersistentInstanceID());
	FInventoryStartupData ShortcutStartup = AnchorStartup;
	ShortcutStartup.Settings.bIsReferenceContainer = true;
	auto* Shortcut = UInventoryBase::CreateInventoryAdvanced(A, ShortcutStartup, A, Anchor);
	Anchor->AddPawnInventory_Internal(Shortcut);
	FItemMapping ShortcutMapping;
	ShortcutMapping.InventoryID = Shortcut->GetInventoryContainerID();
	ShortcutMapping.bIsReferenceContainer = true;
	ShortcutMapping.OccupiedSlots.Add(CastChecked<USlotbasedInventory>(Shortcut)->GetInventorySlots()[0]);
	Anchor->AddItem(Move.SourceItem, ShortcutMapping);
	Move.SourceInventory = Anchor->GetActorInventories()[0];
	Move.TargetInventory = Inventory;
	IInventoryInteractionHandler::Execute_ItemTransferRequest(Manager, Move);
	TestEqual(TEXT("Reverse transfer removes player copy"), Anchor->GetItemLocations().Items.Num(), 0);
	TestEqual(TEXT("Full deposit clears hotbar shortcuts in the old collection"), Anchor->GetAllItemsByContainer(Shortcut->GetInventoryContainerID()).Num(), 0);
	TestEqual(TEXT("Reverse transfer restores shared item"), Shared->Collection->GetItemLocations().Items.Num(), 1);
	Move.SourceItem = Shared->Collection->GetItemLocations().Items[0].Item;
	TestEqual(TEXT("Deposited item lives under the storage actor"), Move.SourceItem->GetTypedOuter<AActor>(), static_cast<AActor*>(Shared));
	IInventoryInteractionHandler::Execute_ItemDeleteRequest(Manager, Inventory->GetInventoryContainerID(), Move.SourceItem);
	TestEqual(TEXT("Destroy resolves and removes the actual stored item"), Shared->Collection->GetItemLocations().Items.Num(), 0);
	FInventoryStartupData LimitedStartup = AnchorStartup;
	LimitedStartup.Settings.InventorySlotBasedSettings.InitNumberRows = 1;
	LimitedStartup.Settings.InventorySlotBasedSettings.InitNumColumns = 1;
	auto* Limited = UInventoryBase::CreateInventoryAdvanced(A, LimitedStartup, A, Anchor);
	Anchor->AddPawnInventory_Internal(Limited);
	auto* AlmostFull = UItemFactory::CreateItemByHandle(A, Initial.Item, 99);
	FItemMoveData Fill;
	Fill.SourceItem = AlmostFull;
	Fill.TargetInventory = Limited;
	TestEqual(TEXT("Prepare nearly full destination"), Limited->HandleAddItem(Fill).ActualAmountAdded, 99);
	auto* PartialSource = UItemFactory::CreateItemByHandle(Shared, Initial.Item, 5);
	Fill.SourceItem = PartialSource;
	Fill.TargetInventory = Inventory;
	Inventory->HandleAddItem(Fill);
	Fill.SourceInventory = Inventory;
	Fill.TargetInventory = Limited;
	IInventoryInteractionHandler::Execute_ItemTransferRequest(Manager, Fill);
	TestEqual(TEXT("Partial transfer fills only available stack capacity"), IObjectDataProvider::Execute_GetQuantity(AlmostFull), 100);
	TestEqual(TEXT("Partial transfer preserves untransferred source quantity"), IObjectDataProvider::Execute_GetQuantity(PartialSource), 4);
	IInventoryInteractionHandler::Execute_ItemTransferRequest(Manager, Fill);
	TestEqual(TEXT("Full destination leaves source unchanged"), IObjectDataProvider::Execute_GetQuantity(PartialSource), 4);
	Anchor->RemoveItemFromAllContainers(AlmostFull);
	Shared->Collection->RemoveItemFromAllContainers(PartialSource);
	Manager->CloseStorageAccess();
	TestFalse(TEXT("Manager close revokes storage access"), Manager->IsInventoryAccessible(Inventory));
	TestFalse(TEXT("Manager close releases group lease"), ChestB->LinkedStorage->IsInteractionLockedByOther(B));
	OpenLinkedStorageTestSession(Manager, PersonalA->LinkedStorage);
	TestTrue(TEXT("Manager personal catalog authorizes owner"), Manager->IsInventoryAccessible(AInventory));
	TestFalse(TEXT("Manager personal catalog rejects another account"), Manager->IsInventoryAccessible(BInventory));
	Manager->CloseStorageAccess();
	const FString SharedKey = Shared->Collection->GetPersistentOwnerKey();
	const FString AKey = Registry->MakeStorageKey(Definition.Get(), ELinkedStorageAccessMode::Personal, A);
	const FString BKey = Registry->MakeStorageKey(Definition.Get(), ELinkedStorageAccessMode::Personal, B);
	FString Error;
	TStrongObjectPtr<UInvenzaSaveGame> Snapshot(Save->CaptureSnapshot(Error));
	if (TestNotNull(TEXT("Capture shared and offline personal storage"), Snapshot.Get()))
	{
		TestEqual(TEXT("One record per physical storage plus player"), Snapshot->Owners.Num(), 5);
		// The fixture has no game-loop startup tick; initialize its playthrough generation explicitly.
		Snapshot->PlaythroughID = FGuid::NewGuid();
		for (auto* Storage : {Shared, Cast<ALinkedStorageState>(AInventory->GetInventoryOwnerActor()),
			Cast<ALinkedStorageState>(BInventory->GetInventoryOwnerActor()), Other})
		{
			Save->UnregisterCollection(Storage->Collection, EEndPlayReason::RemovedFromWorld);
			Storage->Destroy();
		}
		const bool bRestored = Save->ApplySnapshot(Snapshot.Get(), Error);
		TestTrue(TEXT("Restore physical owners transactionally: ") + Error, bRestored);
		auto* Restored = Registry->FindOrCreate(Definition.Get(), ELinkedStorageAccessMode::Shared, A);
		if (TestNotNull(TEXT("Registry discovers restored owner"), Restored))
		{
			TestEqual(TEXT("Stable storage identity"), Restored->Collection->GetPersistentOwnerKey(), SharedKey);
			TestEqual(TEXT("Empty storage is not refilled"), Restored->Collection->GetItemLocations().Items.Num(), 0);
			TestFalse(TEXT("Session locks are not persisted"), Restored->IsLockedFor(B));
		}
		TestNotNull(TEXT("Offline A storage restored"), Registry->FindStorage(AKey));
		TestNotNull(TEXT("Offline B storage restored"), Registry->FindStorage(BKey));
	}
	Instance->Shutdown();
	return true;
}
#endif
