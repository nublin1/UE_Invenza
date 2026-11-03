// Nublin Studio 2026 All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "Data/Inventory/Equipment/EquipmentComponent.h"
#include "Data/Inventory/Equipment/EquipmentSlotDefinition.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/ItemData.h"
#include "Data/Items/itemBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "SaveLoad/InvenzaSaveGame.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "UI/Inventory/UInventoryBaseWidget.h"
#include "UObject/UnrealType.h"
#include "UObject/StrongObjectPtr.h"
#include "Utility/InvenzayUtility.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvenzaStartingItemsTest, "Invenza.Inventory.StartingItems",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInvenzaStartingItemsTest::RunTest(const FString& Parameters)
{
	auto* Instance = NewObject<UGameInstance>(GEngine);
	Instance->InitializeStandalone();
	auto* World = Instance->GetWorld();
	auto* Save = Instance->GetSubsystem<UInvenzaSaveSubsystem>();
	Save->SetProfile(TEXT("StartingItemsAutomation"));
	auto* Subsystem = Instance->GetSubsystem<UInvenzaInventorySettingsSubsystem>();
	auto* Settings = DuplicateObject<UInvenzaInventorySettingsAsset>(Subsystem->GetSettings(), Instance);
	FindFProperty<FObjectPropertyBase>(Subsystem->GetClass(), TEXT("Settings"))->SetObjectPropertyValue_InContainer(Subsystem, Settings);
	Settings->PickupClass = AActor::StaticClass();
	auto* Items = NewObject<UDataTable>(Instance);
	Items->RowStruct = FItemData::StaticStruct();
	FItemData Bag;
	Bag.ID = TEXT("StartupBag");
	Bag.ItemMetaData.ItemCategory = FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Backpack"));
	Bag.ItemMetaData.ItemInventoryData.Rows = 1;
	Bag.ItemMetaData.ItemInventoryData.Columns = 1;
	Bag.ItemMetaData.ItemInventoryData.MaxWeight = 6.f;
	Items->AddRow(Bag.ID, Bag);
	FItemData Resource;
	Resource.ID = TEXT("StartupResource");
	Resource.ItemMetaData.ItemCategory = FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Ingredient"));
	Resource.ItemMetaData.ItemNumeraticData.Weight = 2.f;
	Resource.ItemMetaData.ItemNumeraticData.MaxStackSizeInCharacter = 5;
	Items->AddRow(Resource.ID, Resource);
	FDataTableRowHandle BagRow;
	BagRow.DataTable = Items;
	BagRow.RowName = Bag.ID;
	FDataTableRowHandle ResourceRow;
	ResourceRow.DataTable = Items;
	ResourceRow.RowName = Resource.ID;
	auto* EquipmentTable = NewObject<UDataTable>(Instance);
	EquipmentTable->RowStruct = FEquipmentSlotDefinition::StaticStruct();
	FEquipmentSlotDefinition Definition;
	Definition.SlotTag = FGameplayTag::RequestGameplayTag(TEXT("Equipment.Slot.Backpack"));
	Definition.AllowedCategory = Bag.ItemMetaData.ItemCategory;
	EquipmentTable->AddRow(TEXT("Bag"), Definition);
	FDataTableRowHandle DefinitionRow;
	DefinitionRow.DataTable = EquipmentTable;
	DefinitionRow.RowName = TEXT("Bag");
	const auto WidgetClass = LoadClass<UUInventoryBaseWidget>(nullptr,
		TEXT("/InventorySystemInvenzaPlugin/UI/Core/SimpleInvGrid/WBP_EqGrid.WBP_EqGrid_C"));
	if (!TestNotNull(TEXT("Authored equipment widget"), WidgetClass))
	{
		Instance->Shutdown();
		return false;
	}
	TArray<TObjectPtr<APawn>> Pawns;
	auto CreatePlayer = [&](int32 BackpackCount, bool bPersist) -> UIInventoryManager*
	{
		auto* Pawn = World->SpawnActor<APawn>();
		Pawns.Add(Pawn);
		auto* Collection = NewObject<UInventoryCollectionComponent>(Pawn);
		Pawn->AddInstanceComponent(Collection);
		Collection->RegisterComponent();
		Collection->bPersistInventories = bPersist;
		Collection->SetRuntimeSaveIdentity(TEXT("Player:StartingItems:") + FString::FromInt(Pawns.Num()));
		auto* Equipment = NewObject<UEquipmentComponent>(Pawn);
		Pawn->AddInstanceComponent(Equipment);
		Equipment->RegisterComponent();
		Equipment->SetSlotDefinitions({DefinitionRow});
		auto* Manager = NewObject<UIInventoryManager>(Pawn);
		Pawn->AddInstanceComponent(Manager);
		Manager->RegisterComponent();
		Collection->InitItemCollection();
		Manager->bUsePortableBackpacks = true;
		FInventoryStartupData Gear;
		Gear.Settings.InventoryClass = USlotbasedInventory::StaticClass();
		Gear.Settings.InventoryWidgetClass = WidgetClass;
		Gear.Settings.InventoryTag = Settings->EquipmentInvTag;
		Gear.Settings.bCollectInvDataFromWidget = true;
		Gear.Settings.bIsReferenceContainer = false;
		Gear.Settings.InventorySlotBasedSettings.bIgnoreItemSize = true;
		if (BackpackCount > 0)
		{
			FInitItemsEntry Entry;
			Entry.Item = BagRow;
			Entry.Amount = BackpackCount;
			Gear.StartItems.Add(Entry);
		}
		auto* Startup = FindFProperty<FArrayProperty>(Manager->GetClass(), TEXT("StartupInventories"))
			->ContainerPtrToValuePtr<TArray<FInventoryStartupData>>(Manager);
		Startup->Add(Gear);
		auto* General = FindFProperty<FArrayProperty>(Manager->GetClass(), TEXT("GeneralStartingItems"))
			->ContainerPtrToValuePtr<TArray<FInitItemsEntry>>(Manager);
		FInitItemsEntry Entry;
		Entry.Item = ResourceRow;
		Entry.Amount = 10;
		General->Add(Entry);
		Manager->InitializeInventoryManager();
		return Manager;
	};
	const auto SpawnHandler = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda(
		[](AActor* Actor)
		{
			if (Actor->GetClass() == AActor::StaticClass())
			{
				auto* Pickup = NewObject<UPickupComponent>(Actor);
				Actor->AddInstanceComponent(Pickup);
				Pickup->RegisterComponent();
			}
		}));
	auto* Manager = CreatePlayer(2, false);
	auto* Collection = Manager->GetOwner()->FindComponentByClass<UInventoryCollectionComponent>();
	auto* Contents = Manager->GetPawnMainInventory();
	if (TestNotNull(TEXT("Startup equips backpack without constructing UI"), Contents))
	{
		const auto Stored = Contents->GetItemCollectionLinked()->GetAllItemsByContainer(Contents->GetInventoryContainerID());
		TestEqual(TEXT("A general stack enters the backpack"), Stored.Num(), 1);
		if (!Stored.IsEmpty())
		{
			TestEqual(TEXT("Weight limit accepts exactly three units"), IObjectDataProvider::Execute_GetQuantity(Stored[0]), 3);
		}
		TestEqual(TEXT("Backpack weight is bounded"), Contents->GetInventoryTotalWeight(), 6.f);
		TestEqual(TEXT("Backpack grid comes from the item row"), Contents->GetInventorySize(), FIntPoint(1, 1));
	}
	int32 GroundResources = 0;
	int32 GroundBags = 0;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		auto* Pickup = It->FindComponentByClass<UPickupComponent>();
		auto* Item = Pickup ? Pickup->PeekItemData() : nullptr;
		if (Item && It->GetOwner() == Manager->GetOwner())
		{
			const auto ID = IObjectDataProvider::Execute_GetItemID(Item);
			GroundResources += ID == Resource.ID ? IObjectDataProvider::Execute_GetQuantity(Item) : 0;
			GroundBags += ID == Bag.ID ? IObjectDataProvider::Execute_GetQuantity(Item) : 0;
		}
	}
	TestEqual(TEXT("Every overflow unit is on the ground"), GroundResources, 7);
	TestEqual(TEXT("Only first startup backpack is equipped; extra bag is dropped"), GroundBags, 1);
	TestFalse(TEXT("Successful grants leave no pending resources"), Manager->HasPendingStartingResources());
	const int32 InventoryCount = Collection->GetActorInventories().Num();
	Manager->InitializeInventoryManager();
	Manager->SetupStartingResources();
	TestEqual(TEXT("Repeated initialization creates no inventories"), Collection->GetActorInventories().Num(), InventoryCount);
	TestTrue(TEXT("Repeated initialization retains the active backpack"), Manager->GetPawnMainInventory() == Contents);
	const auto ExampleClass = LoadClass<APawn>(nullptr,
		TEXT("/InventorySystemInvenzaPlugin/Blueprints/Examples/BP_Character.BP_Character_C"));
	auto* Example = ExampleClass ? World->SpawnActor<APawn>(ExampleClass) : nullptr;
	if (TestNotNull(TEXT("Saved example character"), Example))
	{
		auto* ExampleCollection = Example->FindComponentByClass<UInventoryCollectionComponent>();
		auto* ExampleManager = Example->FindComponentByClass<UIInventoryManager>();
		ExampleCollection->bPersistInventories = false;
		ExampleCollection->InitItemCollection();
		ExampleManager->InitializeInventoryManager();

		TestEqual(TEXT("Only active backpack is a player storage; gear/hotbar/mount excluded"),
			ExampleManager->GetInteractionInventories().Num(), 1);
		FInventoryStartupData PocketData;
		PocketData.Settings = ExampleManager->GetPawnMainInventory()->GetInventorySettings();
		PocketData.Settings.CustomInventoryID = TEXT("TestPermanentPockets");
		PocketData.Settings.InventoryTag = FGameplayTag();
		PocketData.Settings.InventoryType = EInventoryType::Inventory;
		auto* Pockets = UInventoryBase::CreateInventoryAdvanced(Example, PocketData, Example, ExampleCollection);
		ExampleCollection->AddPawnInventory_Internal(Pockets);
		TestEqual(TEXT("Additional ordinary storage is included automatically"), ExampleManager->GetInteractionInventories().Num(), 2);
		PocketData.Settings.CustomInventoryID = TEXT("TestHiddenServiceInventory");
		PocketData.Settings.bOpenWithExternalInventory = false;
		auto* Service = UInventoryBase::CreateInventoryAdvanced(Example, PocketData, Example, ExampleCollection);
		ExampleCollection->AddPawnInventory_Internal(Service);
		TestEqual(TEXT("Explicit service storage is excluded"), ExampleManager->GetInteractionInventories().Num(), 2);
		ExampleCollection->SetExternalInventory(Pockets);
		TestEqual(TEXT("Linked external inventory cannot also be a player storage"), ExampleManager->GetInteractionInventories().Num(), 1);
		ExampleCollection->SetExternalInventory(nullptr);
		auto* ExampleBag = ExampleManager->GetPawnMainInventory();
		if (TestNotNull(TEXT("BP_Character actually equips its saved starting backpack"), ExampleBag))
		{
			TestEqual(TEXT("Saved BackpackSmall supplies a 4 by 5 grid"), ExampleBag->GetInventorySize(), FIntPoint(4, 5));
		}
		TestFalse(TEXT("BP_Character general items and examples are fully delivered"), ExampleManager->HasPendingStartingResources());
		for (auto* Inventory : ExampleCollection->GetActorInventories())
		{
			if (const auto* Grid = Cast<USlotbasedInventory>(Inventory))
			{
				TestFalse(TEXT("Every authored example grid is ready without a UI provider"), Grid->GetInventorySlots().IsEmpty());
			}
		}
	}

	World->RemoveOnActorSpawnedHandler(SpawnHandler);
	AddExpectedError(TEXT("Cannot drop startup item"), EAutomationExpectedErrorFlags::Contains, 1);
	auto* NoBag = CreatePlayer(0, false);
	TestNull(TEXT("No pockets without a backpack"), NoBag->GetPawnMainInventory());
	TestTrue(TEXT("Failed drop retains the undelivered grant"), NoBag->HasPendingStartingResources());
	auto* Pending = FindFProperty<FArrayProperty>(NoBag->GetClass(), TEXT("PendingStartingGrants"))
		->ContainerPtrToValuePtr<TArray<FInventoryStartingGrant>>(NoBag);
	const FGuid PendingID = (*Pending)[0].PendingItem->GetPersistentInstanceID();
	NoBag->SetupStartingResources();
	TestEqual(TEXT("Retry keeps the unplaced stack identity"), (*Pending)[0].PendingItem->GetPersistentInstanceID(), PendingID);
	const auto RetryHandler = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda(
		[](AActor* Actor)
		{
			if (Actor->GetClass() == AActor::StaticClass())
			{
				auto* Pickup = NewObject<UPickupComponent>(Actor);
				Actor->AddInstanceComponent(Pickup);
				Pickup->RegisterComponent();
			}
		}));
	NoBag->SetupStartingResources();
	TestFalse(TEXT("Retry delivers all remaining items"), NoBag->HasPendingStartingResources());
	// Exercise the actual save readiness path. A player owner restored empty must remain empty.
	auto* Restored = CreatePlayer(1, true);
	TestNull(TEXT("Persistent player waits for initial load decision"), Restored->GetPawnMainInventory());
	auto* RestoredCollection = Restored->GetOwner()->FindComponentByClass<UInventoryCollectionComponent>();
	FString Error;
	FInventorySavedOwner SavedOwner;
	TestTrue(TEXT("Capture empty owner before startup delivery"), RestoredCollection->CapturePersistence(SavedOwner, Error));
	auto* Snapshot = NewObject<UInvenzaSaveGame>(Instance);
	Snapshot->WorldPackage = UWorld::RemovePIEPrefix(World->GetOutermost()->GetName());
	Snapshot->PlaythroughID = FGuid::NewGuid();
	Snapshot->Owners.Add(SavedOwner);
	if (TestTrue(TEXT("Restore empty state before startup delivery"), Save->ApplySnapshot(Snapshot, Error)))
	{
		Restored->SetupStartingResources();
		TestNull(TEXT("Save restore does not equip a startup backpack"), Restored->GetPawnMainInventory());
		TestFalse(TEXT("Save restore cancels general startup grants"), Restored->HasPendingStartingResources());
	}
	auto* Fresh = CreatePlayer(1, true);
	auto* PlayerController = World->SpawnActor<APlayerController>();
	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	PlayerController->Possess(Cast<APawn>(Fresh->GetOwner()));
	TestTrue(TEXT("Fresh pawn has a real player state"), Cast<APawn>(Fresh->GetOwner())->IsPlayerControlled());
	Save->NotifyPlayerReady();
	World->GetTimerManager().Tick(0.02f);
	// UE queues next-tick delegates until an engine frame has passed.
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
		[this, Instance, World, Save, Fresh, Restored, RestoredCollection, RetryHandler,
		 KeepAlive = MakeShared<TStrongObjectPtr<UGameInstance>>(Instance)]()
		{
			World->GetTimerManager().Tick(0.02f);
			TestNotNull(TEXT("Save readiness grants a new player the startup backpack"), Fresh->GetPawnMainInventory());
			TestNull(TEXT("Save readiness preserves the restored player's empty equipment"), Restored->GetPawnMainInventory());
			TestFalse(TEXT("Fresh player's remaining startup items are all delivered"), Fresh->HasPendingStartingResources());
			World->RemoveOnActorSpawnedHandler(RetryHandler);
			Save->UnregisterCollection(Fresh->GetOwner()->FindComponentByClass<UInventoryCollectionComponent>(), EEndPlayReason::Quit);
			Save->UnregisterCollection(RestoredCollection, EEndPlayReason::Quit);
			Instance->Shutdown();
		}, 0.05f));
	return true;
}
#endif
