// Nublin Studio 2026 All Rights Reserved.

// Regression coverage for authoritative drops through reference and physical inventories.
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "ActorComponents/UIInventoryManager.h"
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
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "UObject/StructOnScope.h"
#include "UObject/UnrealType.h"
#include "Utility/InvenzayUtility.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvenzaReferenceDropTest, "Invenza.Inventory.DropFromReferences",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

static UInventoryBase* CreateDropTestInventory(APawn* Pawn, UInventoryCollectionComponent* Collection,
	UClass* InventoryClass, bool bReference)
{
	FInventoryStartupData Startup;
	Startup.Settings.InventoryClass = InventoryClass;
	Startup.Settings.EntryClass = UInventoryListEntry::StaticClass();
	Startup.Settings.bIsReferenceContainer = bReference;
	Startup.Settings.bCollectInvDataFromWidget = false;
	Startup.Settings.InventorySlotBasedSettings.InitNumberRows = 2;
	Startup.Settings.InventorySlotBasedSettings.InitNumColumns = 2;
	UInventoryBase* Inventory = UInventoryBase::CreateInventoryAdvanced(Collection, Startup, Pawn, Collection);
	Collection->AddPawnInventory_Internal(Inventory);
	return Inventory;
}

static void AddDropTestMapping(UInventoryCollectionComponent* Collection, UInventoryBase* Inventory, UObject* Item)
{
	FItemMapping Mapping;
	Mapping.InventoryID = Inventory->GetInventoryContainerID();
	Mapping.bIsReferenceContainer = Inventory->GetInventorySettings().bIsReferenceContainer;
	UInventorySlotData* Slot = NewObject<UInventorySlotData>(Inventory);
	Slot->InventorySlotInfo.SlotGuid = FGuid::NewGuid();
	Mapping.OccupiedSlots.Add(Slot);
	Collection->AddItem(Item, Mapping);
}

bool FInvenzaReferenceDropTest::RunTest(const FString& Parameters)
{
	UGameInstance* Instance = NewObject<UGameInstance>(GEngine);
	Instance->InitializeStandalone();
	UWorld* World = Instance->GetWorld();
	if (!TestNotNull(TEXT("Standalone test world"), World))
	{
		return false;
	}
	APawn* Pawn = World->SpawnActor<APawn>();
	if (!TestNotNull(TEXT("Authority pawn"), Pawn))
	{
		return false;
	}
	UInventoryCollectionComponent* Collection = NewObject<UInventoryCollectionComponent>(Pawn);
	Pawn->AddInstanceComponent(Collection);
	Collection->RegisterComponent();
	UIInventoryManager* Manager = NewObject<UIInventoryManager>(Pawn);
	Pawn->AddInstanceComponent(Manager);
	Manager->RegisterComponent();
	Collection->InitItemCollection();
	FObjectPropertyBase* CollectionProperty = FindFProperty<FObjectPropertyBase>(UIInventoryManager::StaticClass(), TEXT("ItemCollectionRef"));
	CollectionProperty->SetObjectPropertyValue_InContainer(Manager, Collection);
	UInvenzaInventorySettingsSubsystem* Subsystem = Instance->GetSubsystem<UInvenzaInventorySettingsSubsystem>();
	UInvenzaInventorySettingsAsset* Settings = DuplicateObject<UInvenzaInventorySettingsAsset>(Subsystem->GetSettings(), Instance);
	FindFProperty<FObjectPropertyBase>(Subsystem->GetClass(), TEXT("Settings"))->SetObjectPropertyValue_InContainer(Subsystem, Settings);
	Settings->PickupClass = AActor::StaticClass();

	UInventoryBase* UnrelatedMain = CreateDropTestInventory(Pawn, Collection, USlotbasedInventory::StaticClass(), false);
	UInventoryBase* Physical = CreateDropTestInventory(Pawn, Collection, USlotbasedInventory::StaticClass(), false);
	UInventoryBase* Reference = CreateDropTestInventory(Pawn, Collection, USlotbasedInventory::StaticClass(), true);
	UInventoryBase* SecondReference = CreateDropTestInventory(Pawn, Collection, USlotbasedInventory::StaticClass(), true);
	UInventoryBase* ListPhysical = CreateDropTestInventory(Pawn, Collection, UListInventory::StaticClass(), false);
	UDataTable* Table = NewObject<UDataTable>(Instance);
	Table->RowStruct = FItemData::StaticStruct();
	FItemData Row;
	Row.ID = TEXT("DropRegressionItem");
	Row.ItemMetaData.bIsDroppable = true;
	Row.ItemMetaData.bIsDeletable = true;
	Row.ItemMetaData.ItemNumeraticData.MaxStackSizeInCharacter = 100;
	Table->AddRow(Row.ID, Row);
	FDataTableRowHandle Definition;
	Definition.DataTable = Table;
	Definition.RowName = Row.ID;
	UItemBase* Item = nullptr;
	bool bAttachPickupComponent = true;
	bool bInvalidateDuringSpawn = false;
	UInventoryBase* CurrentPhysical = Physical;
	const FDelegateHandle SpawnHandler = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda(
		[&](AActor* Actor)
		{
			if (Actor->GetOwner() != Pawn)
			{
				return;
			}
			if (bAttachPickupComponent)
			{
				UPickupComponent* Pickup = NewObject<UPickupComponent>(Actor);
				Actor->AddInstanceComponent(Pickup);
				Pickup->RegisterComponent();
			}
			if (bInvalidateDuringSpawn)
			{
				Collection->RemoveItem(Item, CurrentPhysical->GetInventoryContainerID());
			}
		}));
	const auto ResetItem = [&](UInventoryBase* Inventory)
	{
		if (Item)
		{
			Collection->RemoveItemFromAllContainers(Item);
		}
		CurrentPhysical = Inventory;
		Item = UItemFactory::CreateItemByHandle(Collection, Definition, 10);
		AddDropTestMapping(Collection, Inventory, Item);
		AddDropTestMapping(Collection, Reference, Item);
		AddDropTestMapping(Collection, SecondReference, Item);
	};
	const auto PickupQuantity = [&]()
	{
		int32 Total = 0;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (IsValid(*It) && !It->IsActorBeingDestroyed() && It->GetOwner() == Pawn)
			{
				UPickupComponent* Pickup = It->FindComponentByClass<UPickupComponent>();
				if (Pickup && Pickup->PeekItemData())
				{
					Total += IObjectDataProvider::Execute_GetQuantity(Pickup->PeekItemData());
				}
			}
		}
		return Total;
	};

	ResetItem(Physical);
	Manager->HandleItemDrop(FItemDropData(Item, Reference, 3));
	TestEqual(TEXT("Reference drop consumes physical quantity"), Item->GetQuantity_Implementation(), 7);
	TestEqual(TEXT("Partial drop preserves all mappings"), Collection->FindAllMappingsForItem(Item).Num(), 3);
	TestEqual(TEXT("Exactly three units spawned"), PickupQuantity(), 3);
	TestFalse(TEXT("Unrelated main inventory is not used"), Collection->ItemHasInventory(Item, UnrelatedMain->GetInventoryContainerID()));
	Manager->HandleItemDrop(FItemDropData(Item, Reference, 7));
	TestEqual(TEXT("Full drop removes physical item and every reference"), Collection->FindAllMappingsForItem(Item).Num(), 0);
	TestEqual(TEXT("Ten total units conserved after full drop"), PickupQuantity(), 10);
	Manager->HandleItemDrop(FItemDropData(Item, Reference, 7));
	TestEqual(TEXT("Repeated stale request cannot spawn another pickup"), PickupQuantity(), 10);

	ResetItem(Physical);
	Collection->RemoveItem(Item, Physical->GetInventoryContainerID());
	Manager->HandleItemDrop(FItemDropData(Item, Reference, 3));
	TestEqual(TEXT("Orphan reference does not spawn"), PickupQuantity(), 10);
	TestEqual(TEXT("Orphan quantity remains unchanged"), Item->GetQuantity_Implementation(), 10);

	ResetItem(Physical);
	Manager->HandleItemDrop(FItemDropData(Item, Reference, 0));
	Manager->HandleItemDrop(FItemDropData(Item, Reference, -1));
	Manager->HandleItemDrop(FItemDropData(Item, Reference, 11));
	Manager->HandleItemDrop(FItemDropData(NewObject<UDataTable>(Collection), Reference, 1));
	TestEqual(TEXT("Invalid requests do not consume items"), Item->GetQuantity_Implementation(), 10);
	TestEqual(TEXT("Invalid requests do not spawn"), PickupQuantity(), 10);

	FArrayProperty* InventoriesProperty = FindFProperty<FArrayProperty>(Collection->GetClass(), TEXT("ActorInventories"));
	void* InventoryArray = InventoriesProperty->ContainerPtrToValuePtr<void>(Collection);
	TArray<TObjectPtr<UInventoryBase>>* Array = static_cast<TArray<TObjectPtr<UInventoryBase>>*>(InventoryArray);
	Array->Remove(Physical);
	Manager->HandleItemDrop(FItemDropData(Item, Reference, 3));
	TestEqual(TEXT("Inaccessible physical inventory does not spawn"), PickupQuantity(), 10);
	TestEqual(TEXT("Inaccessible physical inventory is unchanged"), Item->GetQuantity_Implementation(), 10);
	Collection->AddPawnInventory_Internal(Physical);

	bAttachPickupComponent = false;
	Manager->HandleItemDrop(FItemDropData(Item, Reference, 3));
	TestEqual(TEXT("Missing pickup component preserves quantity"), Item->GetQuantity_Implementation(), 10);
	TestEqual(TEXT("Missing pickup component preserves all mappings"), Collection->FindAllMappingsForItem(Item).Num(), 3);
	bAttachPickupComponent = true;
	IObjectDataProvider::Execute_SetQuantity(Item, 101);
	Manager->HandleItemDrop(FItemDropData(Item, Reference, 101));
	TestEqual(TEXT("Pickup initialization that clamps quantity is rejected"), Item->GetQuantity_Implementation(), 101);
	TestEqual(TEXT("Failed pickup initialization leaves no extra units in world"), PickupQuantity(), 10);
	IObjectDataProvider::Execute_SetQuantity(Item, 10);
	Settings->PickupClass = nullptr;
	AddExpectedError(TEXT("PickupClass is not set."), EAutomationExpectedErrorFlags::Contains, 1);
	Manager->HandleItemDrop(FItemDropData(Item, Reference, 3));
	TestEqual(TEXT("Unconfigured pickup preserves quantity"), Item->GetQuantity_Implementation(), 10);
	Settings->PickupClass = AActor::StaticClass();

	bInvalidateDuringSpawn = true;
	Manager->HandleItemDrop(FItemDropData(Item, Reference, 3));
	TestEqual(TEXT("Inventory changed by spawn callback leaves no extra pickup"), PickupQuantity(), 10);
	bInvalidateDuringSpawn = false;

	ResetItem(ListPhysical);
	Manager->HandleItemDrop(FItemDropData(Item, Reference, 4));
	TestEqual(TEXT("List inventory partial drop consumes its quantity"), Item->GetQuantity_Implementation(), 6);
	TestEqual(TEXT("List inventory reference mappings survive partial drop"), Collection->FindAllMappingsForItem(Item).Num(), 3);
	Manager->HandleItemDrop(FItemDropData(Item, ListPhysical, 6));
	TestEqual(TEXT("Direct physical drop also clears references"), Collection->FindAllMappingsForItem(Item).Num(), 0);
	TestEqual(TEXT("Both physical inventory types conserve quantity"), PickupQuantity(), 20);

	ResetItem(Physical);
	Reference->HandleRemoveItem(Item, 10);
	TestTrue(TEXT("Removing only a shortcut still preserves the physical item"), Collection->ItemHasInventory(Item, Physical->GetInventoryContainerID()));
	TestEqual(TEXT("Shortcut deletion preserves item quantity"), Item->GetQuantity_Implementation(), 10);
	TestEqual(TEXT("Shortcut deletion spawns nothing"), PickupQuantity(), 20);

	ResetItem(Physical);
	FindFProperty<FObjectPropertyBase>(Manager->GetClass(), TEXT("PendingContextItem"))->SetObjectPropertyValue_InContainer(Manager, Item);
	FindFProperty<FObjectPropertyBase>(Manager->GetClass(), TEXT("PendingContextInv"))->SetObjectPropertyValue_InContainer(Manager, Reference);
	UFunction* ResponseFunction = Manager->FindFunctionChecked(TEXT("OnInventoryModalResponse"));
	FStructOnScope ResponseParameters(ResponseFunction);
	FModalResult Response;
	Response.ResultInteractionType = EObjectInteractionType::Destroy;
	FindFProperty<FStructProperty>(ResponseFunction, TEXT("Result"))->CopyCompleteValue(
		FindFProperty<FStructProperty>(ResponseFunction, TEXT("Result"))->ContainerPtrToValuePtr<void>(ResponseParameters.GetStructMemory()), &Response);
	Manager->ProcessEvent(ResponseFunction, ResponseParameters.GetStructMemory());
	TestEqual(TEXT("Destroy from hotbar context removes every mapping"), Collection->FindAllMappingsForItem(Item).Num(), 0);
	TestEqual(TEXT("Destroy clears the physical inventory"), Collection->GetAllItemsByContainer(Physical->GetInventoryContainerID()).Num(), 0);
	TestEqual(TEXT("Destroy clears the source reference inventory"), Collection->GetAllItemsByContainer(Reference->GetInventoryContainerID()).Num(), 0);
	TestEqual(TEXT("Destroy clears other reference inventories"), Collection->GetAllItemsByContainer(SecondReference->GetInventoryContainerID()).Num(), 0);
	TestEqual(TEXT("Destroy does not spawn a pickup"), PickupQuantity(), 20);
	Manager->Server_OnItemDelete_Implementation(Reference->GetInventoryContainerID(), Item);
	TestEqual(TEXT("Repeated destroy is harmless"), Collection->FindAllMappingsForItem(Item).Num(), 0);

	ResetItem(ListPhysical);
	Manager->Server_OnItemDelete_Implementation(ListPhysical->GetInventoryContainerID(), Item);
	TestEqual(TEXT("Destroy from physical list inventory clears its references"), Collection->FindAllMappingsForItem(Item).Num(), 0);
	TestEqual(TEXT("Destroy from list inventory does not spawn"), PickupQuantity(), 20);

	ResetItem(Physical);
	Manager->Server_OnItemDelete_Implementation(UnrelatedMain->GetInventoryContainerID(), Item);
	TestEqual(TEXT("Destroy from an inventory that does not contain the item is rejected"), Collection->FindAllMappingsForItem(Item).Num(), 3);
	Manager->Server_OnItemDelete_Implementation(TEXT("UnknownInventory"), Item);
	TestEqual(TEXT("Destroy from an unknown inventory is rejected"), Collection->FindAllMappingsForItem(Item).Num(), 3);
	Collection->RemoveItem(Item, Physical->GetInventoryContainerID());
	Manager->Server_OnItemDelete_Implementation(Reference->GetInventoryContainerID(), Item);
	TestEqual(TEXT("Destroy also removes every orphan shortcut"), Collection->FindAllMappingsForItem(Item).Num(), 0);

	World->RemoveOnActorSpawnedHandler(SpawnHandler);
	Instance->Shutdown();
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return !HasAnyErrors();
}
#endif
