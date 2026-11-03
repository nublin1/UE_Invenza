// Nublin Studio 2026 All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "ActorComponents/Crafting/CraftingComponent.h"
#include "Data/Inventory/Equipment/EquipmentComponent.h"
#include "Data/Inventory/Equipment/EquipmentSlotDefinition.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Inventory/ListInventory/ListInventory.h"
#include "Data/Inventory/ListInventory/InventoryListEntry.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/ItemData.h"
#include "Data/Items/itemBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Factory/ItemFactory.h"
#include "GameFramework/Pawn.h"
#include "SaveLoad/InvenzaSaveGame.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "UObject/UnrealType.h"
#include "World/PortableInventoryState.h"

static USlotbasedInventory* CreatePortableTestGrid(AActor* Owner, UInventoryCollectionComponent* Collection,
	FIntPoint Size, bool bReference = false)
{
	FInventoryStartupData Startup;
	Startup.Settings.InventoryClass = USlotbasedInventory::StaticClass();
	Startup.Settings.bCollectInvDataFromWidget = false;
	Startup.Settings.bAllowItemReferencing = true;
	Startup.Settings.bIsReferenceContainer = bReference;
	Startup.Settings.InventorySlotBasedSettings.bIgnoreItemSize = bReference;
	Startup.Settings.InventorySlotBasedSettings.InitNumberRows = Size.X;
	Startup.Settings.InventorySlotBasedSettings.InitNumColumns = Size.Y;
	auto* Grid = Cast<USlotbasedInventory>(UInventoryBase::CreateInventoryAdvanced(Owner, Startup, Owner, Collection));
	Collection->AddPawnInventory_Internal(Grid);
	return Grid;
}

static bool AddPortableTestItem(UInventoryBase* Inventory, UItemBase* Item)
{
	FItemMoveData Move;
	Move.SourceItem = Item;
	Move.TargetInventory = Inventory;
	return Inventory->HandleAddItem(Move).ActualAmountAdded == IObjectDataProvider::Execute_GetQuantity(Item);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvenzaPortableBackpackTest, "Invenza.Inventory.PortableBackpacks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInvenzaPortableBackpackTest::RunTest(const FString& Parameters)
{
	auto* Instance = NewObject<UGameInstance>(GEngine);
	Instance->InitializeStandalone();
	auto* World = Instance->GetWorld();
	if (!TestNotNull(TEXT("Standalone world"), World))
	{
		return false;
	}
	auto* Save = Instance->GetSubsystem<UInvenzaSaveSubsystem>();
	Save->SetProfile(TEXT("PortableBackpackAutomation"));
	auto* Subsystem = Instance->GetSubsystem<UInvenzaInventorySettingsSubsystem>();
	auto* Settings = DuplicateObject<UInvenzaInventorySettingsAsset>(Subsystem->GetSettings(), Instance);
	FindFProperty<FObjectPropertyBase>(Subsystem->GetClass(), TEXT("Settings"))->SetObjectPropertyValue_InContainer(Subsystem, Settings);
	Settings->PickupClass = AActor::StaticClass();
	auto* Pawn = World->SpawnActor<APawn>();
	auto* Collection = NewObject<UInventoryCollectionComponent>(Pawn);
	Pawn->AddInstanceComponent(Collection);
	Collection->RegisterComponent();
	Collection->InitItemCollection();
	Collection->SetRuntimeSaveIdentity(TEXT("Player:PortableBackpackTest"));
	auto* Equipment = NewObject<UEquipmentComponent>(Pawn);
	Pawn->AddInstanceComponent(Equipment);
	Equipment->RegisterComponent();
	auto* Manager = NewObject<UIInventoryManager>(Pawn);
	Pawn->AddInstanceComponent(Manager);
	Manager->RegisterComponent();
	Manager->bUsePortableBackpacks = true;
	Collection->SetInvManager(Manager);
	FindFProperty<FObjectPropertyBase>(Manager->GetClass(), TEXT("ItemCollectionRef"))->SetObjectPropertyValue_InContainer(Manager, Collection);
	FindFProperty<FObjectPropertyBase>(Manager->GetClass(), TEXT("GlobalSettings"))->SetObjectPropertyValue_InContainer(Manager, Settings);
	FindFProperty<FObjectPropertyBase>(Manager->GetClass(), TEXT("EquipmentComponentRef"))->SetObjectPropertyValue_InContainer(Manager, Equipment);
	Equipment->OnEquippedItem.AddUniqueDynamic(Manager, &UIInventoryManager::PortableEquipmentChanged);
	Equipment->OnUnequippedItem.AddUniqueDynamic(Manager, &UIInventoryManager::PortableEquipmentChanged);
	// A backpack must resolve any equipment slot whose category accepts it.
	const auto SlotTag = FGameplayTag::RequestGameplayTag(TEXT("Equipment.Slot.Chest"));
	const auto Category = FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Backpack"));
	auto* EquipmentTable = NewObject<UDataTable>(Instance);
	EquipmentTable->RowStruct = FEquipmentSlotDefinition::StaticStruct();
	FEquipmentSlotDefinition SlotDefinition;
	SlotDefinition.SlotTag = SlotTag;
	SlotDefinition.AllowedCategory = Category;
	EquipmentTable->AddRow(TEXT("Backpack"), SlotDefinition);
	FDataTableRowHandle EquipmentRow;
	EquipmentRow.DataTable = EquipmentTable;
	EquipmentRow.RowName = TEXT("Backpack");
	Equipment->SetSlotDefinitions({EquipmentRow});
	Equipment->InitializeSlotsFromTable();
	auto* Gear = CreatePortableTestGrid(Pawn, Collection, FIntPoint(1, 1), true);
	auto GearSettings = Gear->GetInventorySettings();
	GearSettings.InventoryTag = Settings->EquipmentInvTag;
	Gear->SetInventorySettings(GearSettings);
	Gear->GetInventorySlots()[0]->InventorySlotInfo.LinkedEquipmentSlot = SlotTag;
	Gear->GetInventorySlots()[0]->InventorySlotInfo.AllowedCategory = Category;
	auto* Chest = CreatePortableTestGrid(Pawn, Collection, FIntPoint(4, 5));
	auto* Items = NewObject<UDataTable>(Instance);
	Items->RowStruct = FItemData::StaticStruct();
	FItemData BackpackRow;
	BackpackRow.ID = TEXT("TestBackpack");
	BackpackRow.ItemMetaData.ItemCategory = Category;
	BackpackRow.ItemMetaData.ItemNumeraticData.Weight = 2.f;
	BackpackRow.ItemMetaData.ItemInventoryData.Rows = 2;
	BackpackRow.ItemMetaData.ItemInventoryData.Columns = 3;
	Items->AddRow(BackpackRow.ID, BackpackRow);
	FItemData ResourceRow;
	ResourceRow.ID = TEXT("TestResource");
	ResourceRow.ItemMetaData.ItemCategory = FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Ingredient"));
	ResourceRow.ItemMetaData.ItemNumeraticData.Weight = 3.f;
	ResourceRow.ItemMetaData.ItemNumeraticData.InventoryHorizontalSlots = 2;
	ResourceRow.ItemMetaData.ItemNumeraticData.InventoryVerticalSlots = 2;
	Items->AddRow(ResourceRow.ID, ResourceRow);
	FDataTableRowHandle BackpackHandle;
	BackpackHandle.DataTable = Items;
	BackpackHandle.RowName = BackpackRow.ID;
	FDataTableRowHandle ResourceHandle;
	ResourceHandle.DataTable = Items;
	ResourceHandle.RowName = ResourceRow.ID;
	auto* First = UItemFactory::CreateItemByHandle(Pawn, BackpackHandle, 1);
	auto* Second = UItemFactory::CreateItemByHandle(Pawn, BackpackHandle, 1);
	TestNull(TEXT("No personal inventory without a backpack"), Manager->GetPawnMainInventory());
	TestTrue(TEXT("Empty backpack can be carried"), AddPortableTestItem(Chest, First));
	TestTrue(TEXT("Second empty backpack can be carried"), AddPortableTestItem(Chest, Second));
	const FGuid FirstID = First->GetPersistentInstanceID();
	TestTrue(TEXT("Equip first backpack"), Manager->TryEquipPortableBackpack(First, Chest));
	auto* Contents = Cast<USlotbasedInventory>(Manager->GetPawnMainInventory());
	if (!TestNotNull(TEXT("Equipped backpack exposes its inventory"), Contents))
	{
		return false;
	}
	auto* Resource = UItemFactory::CreateItemByHandle(Contents->GetInventoryOwnerActor(), ResourceHandle, 1);
	TestTrue(TEXT("Put a resource inside"), AddPortableTestItem(Contents, Resource));
	const FGuid ResourceID = Resource->GetPersistentInstanceID();
	UItemBase* Equipped = nullptr;
	for (const auto& Slot : Equipment->GetEquipmentSlotsArray())
	{
		Equipped = Cast<UItemBase>(Slot.EquippedItem);
	}
	TestEqual(TEXT("Equip preserves backpack identity"), Equipped->GetPersistentInstanceID(), FirstID);
	TestEqual(TEXT("Backpack weight includes its contents"), Equipped->GetItemSingleWeight_Implementation(), 5.f);
	TestFalse(TEXT("Filled backpack cannot enter another inventory"), Equipped->CanStoreInInventory(Chest));
	TestFalse(TEXT("Backpack cannot contain itself"), Equipped->CanStoreInInventory(Contents));
	Contents->SortItemsInContainerByName();
	TestTrue(TEXT("Sorting retains the original resource"), Contents->GetItemCollectionLinked()->ItemHasInventory(Resource, Contents->GetInventoryContainerID()));
	TestEqual(TEXT("Sorting preserves item identity"), Resource->GetPersistentInstanceID(), ResourceID);
	auto* OriginalSlot = Contents->GetSlotByPosition(FIntPoint::ZeroValue);
	TestTrue(TEXT("Growing a live inventory"), Contents->TryResizeInventory(FIntPoint(4, 4)));
	TestEqual(TEXT("Existing slots remain stable"), Contents->GetSlotByPosition(FIntPoint::ZeroValue), OriginalSlot);
	TestFalse(TEXT("Shrink rejects contents that cannot fit"), Contents->TryResizeInventory(FIntPoint(1, 1)));
	TestEqual(TEXT("Rejected shrink preserves the grid size"), Contents->GetInventorySize(), FIntPoint(4, 4));
	auto* Craft = NewObject<UCraftingComponent>(Pawn);
	Pawn->AddInstanceComponent(Craft);
	Craft->RegisterComponent();
	auto* Pending = FindFProperty<FArrayProperty>(Craft->GetClass(), TEXT("PendingDeliveries"))
		->ContainerPtrToValuePtr<TArray<FCraftItemBatch>>(Craft);
	FCraftItemBatch Refund;
	Refund.Inventory = Contents;
	Refund.Sample = Resource;
	Refund.Amount = 1;
	Pending->Add(Refund);
	Craft->RefreshInventoryObligationIndex();
	TestTrue(TEXT("Pending craft refund locks the backpack"), Manager->IsPortableBackpackRemovalBlocked(Equipped));
	TestFalse(TEXT("Craft refund blocks backpack replacement"), Manager->TryEquipPortableBackpack(Second, Chest));
	TestFalse(TEXT("Craft refund blocks resizing"), Contents->TryResizeInventory(FIntPoint(5, 5)));
	Pending->Reset();
	Craft->RefreshInventoryObligationIndex();
	auto* Trade = FindFProperty<FStructProperty>(Manager->GetClass(), TEXT("TradeSessionItems"))
		->ContainerPtrToValuePtr<FTradeSessionItems>(Manager);
	Trade->Inventory = Chest;
	TestFalse(TEXT("Trading blocks backpack replacement"), Manager->TryEquipPortableBackpack(Second, Chest));
	TestFalse(TEXT("Trading blocks dropping the backpack"), Manager->TryDropPortableBackpack(Equipped, Collection));
	Equipment->Server_UnequipItemFromSlot(SlotTag);
	TestTrue(TEXT("Direct unequip cannot bypass the trade lock"), Equipment->IsItemEquippedInSlot(Equipped, SlotTag));
	Trade->Inventory = nullptr;
	TestFalse(TEXT("Failed ground spawn cancels replacement"), Manager->TryEquipPortableBackpack(Second, Chest));
	TestTrue(TEXT("Failed replacement retains the active backpack"), Equipment->IsItemEquippedInSlot(Equipped, SlotTag));
	const auto SpawnHandler = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda(
		[Pawn](AActor* Actor)
		{
			if (Actor->GetOwner() == Pawn)
			{
				auto* Pickup = NewObject<UPickupComponent>(Actor);
				Actor->AddInstanceComponent(Pickup);
				Pickup->RegisterComponent();
			}
		}));
	TestTrue(TEXT("Replace backpack and drop the previous one"), Manager->TryEquipPortableBackpack(Second, Chest));
	TestNotEqual(TEXT("Different backpacks expose different runtime inventory identifiers"),
		Contents->GetInventoryContainerID(), Manager->GetPawnMainInventory()->GetInventoryContainerID());
	UPickupComponent* Dropped = nullptr;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		auto* Pickup = It->FindComponentByClass<UPickupComponent>();
		auto* Item = Pickup ? Cast<UItemBase>(Pickup->PeekItemData()) : nullptr;
		if (Item && Item->GetPersistentInstanceID() == FirstID)
		{
			Dropped = Pickup;
		}
	}
	if (TestNotNull(TEXT("Old backpack exists on the ground"), Dropped))
	{
		TestTrue(TEXT("Ground backpack opens the same contents"), Dropped->GetMainLootContainer().Get() == Contents);
		TestTrue(TEXT("Contents survive replacement"), Contents->GetItemCollectionLinked()->ItemHasInventory(Resource, Contents->GetInventoryContainerID()));
		TestTrue(TEXT("Equip directly from the ground"), Manager->TryEquipPortableBackpack(Cast<UItemBase>(Dropped->PeekItemData()), nullptr, Dropped));
	}
	World->RemoveOnActorSpawnedHandler(SpawnHandler);
	auto* Hotbar = CreatePortableTestGrid(Pawn, Collection, FIntPoint(1, 1), true);
	FItemMapping Shortcut;
	Shortcut.InventoryID = Hotbar->GetInventoryContainerID();
	Shortcut.bIsReferenceContainer = true;
	Shortcut.OccupiedSlots.Add(Hotbar->GetInventorySlots()[0]);
	Collection->AddItem(Resource, Shortcut);
	Save->RegisterCollection(Collection);
	for (TActorIterator<APortableInventoryState> It(World); It; ++It)
	{
		Save->RegisterCollection(It->Collection);
	}
	FString Error;
	auto* Snapshot = Save->CaptureSnapshot(Error);
	TestNotNull(*FString::Printf(TEXT("Capture portable containers: %s"), *Error), Snapshot);
	if (Snapshot)
	{
		Pending->Add(Refund);
		Craft->RefreshInventoryObligationIndex();
		TestTrue(*FString::Printf(TEXT("Restore portable containers: %s"), *Error), Save->ApplySnapshot(Snapshot, Error));
		TestNotNull(TEXT("Backpack remains active after load"), Manager->GetPawnMainInventory());
		for (const auto& Slot : Equipment->GetEquipmentSlotsArray())
		{
			TestNotNull(TEXT("Loaded equipment points at the restored physical item"),
				Collection->FindMainInventoryMappingForItem(Slot.EquippedItem));
		}
		auto* RestoredShortcut = Cast<UItemBase>(Collection->GetItemFromSlot(
			Hotbar->GetInventorySlots()[0]->InventorySlotInfo.SlotGuid, Hotbar->GetInventoryContainerID()));
		if (TestNotNull(TEXT("Hotbar restores its reference into the backpack"), RestoredShortcut))
		{
			TestEqual(TEXT("Hotbar reference preserves content identity"), RestoredShortcut->GetPersistentInstanceID(), ResourceID);
		}
		TArray<APortableInventoryState*> OldStates;
		for (TActorIterator<APortableInventoryState> It(World); It; ++It)
		{
			OldStates.Add(*It);
		}
		for (auto* State : OldStates)
		{
			Save->UnregisterCollection(State->Collection, EEndPlayReason::RemovedFromWorld);
			State->Destroy();
		}
		TestTrue(*FString::Printf(TEXT("Recreate absent portable state actors: %s"), *Error), Save->ApplySnapshot(Snapshot, Error));
		auto* RestoredContents = Manager->GetPawnMainInventory();
		if (TestNotNull(TEXT("Absent backpack contents are reconstructed"), RestoredContents))
		{
			TestEqual(TEXT("Resized grid survives reconstruction"), RestoredContents->GetInventorySize(), FIntPoint(4, 4));
			const auto RestoredItems = RestoredContents->GetItemCollectionLinked()->GetAllItemsByContainer(
				RestoredContents->GetInventoryContainerID());
			TestEqual(TEXT("Reconstructed contents contain exactly one item"), RestoredItems.Num(), 1);
			if (!RestoredItems.IsEmpty())
			{
				TestEqual(TEXT("Reconstruction preserves content identity"),
					Cast<UItemBase>(RestoredItems[0])->GetPersistentInstanceID(), ResourceID);
				RestoredContents->HandleRemoveItem(RestoredItems[0], 1);
			}
			auto* Root = Collection->GetItemFromSlot(Gear->GetInventorySlots()[0]->InventorySlotInfo.SlotGuid,
				Gear->GetInventoryContainerID());
			TestNotNull(TEXT("Backpack remains in the visible equipment slot"), Root);
			if (auto* PortableRoot = Cast<UItemBase>(Root))
			{
				TestTrue(TEXT("The active inventory belongs to the restored equipped backpack"),
					PortableRoot->GetPortableState()->GetInventory() == RestoredContents);
				TestFalse(TEXT("Removing the last resource empties the backpack"), PortableRoot->HasPortableContents());
				TestFalse(TEXT("No obligations remain after load"), Manager->IsPortableBackpackRemovalBlocked(PortableRoot));
				TestTrue(TEXT("An emptied backpack accepts an ordinary inventory destination"), PortableRoot->CanStoreInInventory(Chest));
			}
			FItemMoveData Carry(Root, Gear, Gear->GetInventorySlots()[0]->InventorySlotInfo.SlotGuid, Chest, {});
			TestTrue(TEXT("The visible backpack matches the equipped item"), Equipment->IsItemEquipped(Root));
			bool bCanCarry = false;
			Manager->CheckEquipmentTransfer(Carry, bCanCarry);
			TestTrue(TEXT("Equipment transfer accepts an empty backpack"), bCanCarry);
			Manager->Handle_ItemTransferRequest(Carry);
			Manager->RefreshPortableBackpack();
			TestTrue(TEXT("An empty equipped backpack can be carried in a chest"),
				Collection->ItemHasInventory(Root, Chest->GetInventoryContainerID()));
			TestNull(TEXT("Carrying an empty backpack does not expose its contents"), Manager->GetPawnMainInventory());
			FInventorySavedOwner CarriedSnapshot;
			TestTrue(TEXT("Carried backpack has exactly one physical owner"), Collection->CapturePersistence(CarriedSnapshot, Error));
			FInventoryStartupData ListStartup;
			ListStartup.Settings.InventoryClass = UListInventory::StaticClass();
			ListStartup.Settings.EntryClass = UInventoryListEntry::StaticClass();
			ListStartup.Settings.CustomInventoryID = TEXT("PortableListTest");
			auto* List = UInventoryBase::CreateInventoryAdvanced(Pawn, ListStartup, Pawn, Collection);
			Collection->AddPawnInventory_Internal(List);
			FItemMoveData CarryToList(Root, Chest, {}, List, {});
			Manager->Handle_ItemTransferRequest(CarryToList);
			const auto CarriedItems = Collection->GetAllItemsByContainer(List->GetInventoryContainerID());
			if (TestEqual(TEXT("Empty backpack can enter a list inventory"), CarriedItems.Num(), 1))
			{
				auto* Carried = Cast<UItemBase>(CarriedItems[0]);
				TestEqual(TEXT("Carrying preserves backpack identity"), Carried->GetPersistentInstanceID(), FirstID);
				TestTrue(TEXT("Reequip the carried empty backpack"), Manager->TryEquipPortableBackpack(Carried, List));
				TestTrue(TEXT("Carrying preserves the backpack's resized inventory"), Manager->GetPawnMainInventory() == RestoredContents);
			}
		}
	}
	Instance->Shutdown();
	return true;
}
#endif
