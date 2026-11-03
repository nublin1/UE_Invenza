// Nublin Studio 2026 All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "Components/StaticMeshComponent.h"
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
#include "UI/Inventory/UInventoryBaseWidget.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvenzaMultiPickupTest, "Invenza.Inventory.MultiPickup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInvenzaMultiPickupTest::RunTest(const FString& Parameters)
{
	auto* Instance = NewObject<UGameInstance>(GEngine);
	Instance->InitializeStandalone();
	auto* World = Instance->GetWorld();
	auto* Save = Instance->GetSubsystem<UInvenzaSaveSubsystem>();
	Save->SetProfile(TEXT("MultiPickupAutomation"));
	auto* Subsystem = Instance->GetSubsystem<UInvenzaInventorySettingsSubsystem>();
	auto* Settings = DuplicateObject<UInvenzaInventorySettingsAsset>(Subsystem->GetSettings(), Instance);
	FindFProperty<FObjectPropertyBase>(Subsystem->GetClass(), TEXT("Settings"))->SetObjectPropertyValue_InContainer(Subsystem, Settings);
	Settings->PickupClass = AActor::StaticClass();
	auto* Table = NewObject<UDataTable>(Instance);
	Table->RowStruct = FItemData::StaticStruct();
	for (const auto ID : {FName(TEXT("ResourceA")), FName(TEXT("ResourceB"))})
	{
		FItemData Row;
		Row.ID = ID;
		Row.ItemMetaData.ItemCategory = FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Ingredient"));
		Row.ItemMetaData.ItemNumeraticData.Weight = ID == TEXT("ResourceA") ? 1.f : 2.f;
		Row.ItemMetaData.ItemNumeraticData.MaxStackSizeInCharacter = 2;
		Table->AddRow(ID, Row);
	}
	auto Entry = [&](FName Row, int32 Quantity)
	{
		FInitItemsEntry Result;
		Result.Item.DataTable = Table;
		Result.Item.RowName = Row;
		Result.Amount = Quantity;
		return Result;
	};
	auto* Pawn = World->SpawnActor<APawn>();
	auto* Collection = NewObject<UInventoryCollectionComponent>(Pawn);
	Pawn->AddInstanceComponent(Collection);
	Collection->RegisterComponent();
	Collection->bPersistInventories = false;
	Collection->InitItemCollection();
	auto* Interaction = NewObject<UInteractionComponent>(Pawn);
	Pawn->AddInstanceComponent(Interaction);
	Interaction->RegisterComponent();
	auto* Manager = NewObject<UIInventoryManager>(Pawn);
	Pawn->AddInstanceComponent(Manager);
	Manager->RegisterComponent();
	Manager->InitializeInventoryManager();
	TestFalse(TEXT("Authority test pawn has no local controller"), Pawn->IsLocallyControlled());
	TestTrue(TEXT("Authority handles interactions even without a local UI"),
		Interaction->OnInteract.Contains(Manager, FName(TEXT("InteractRequest"))));
	FindFProperty<FObjectPropertyBase>(Manager->GetClass(), TEXT("ItemCollectionRef"))->SetObjectPropertyValue_InContainer(Manager, Collection);
	FindFProperty<FObjectPropertyBase>(Manager->GetClass(), TEXT("GlobalSettings"))->SetObjectPropertyValue_InContainer(Manager, Settings);
	Save->RegisterCollection(Collection);
	auto CreateStorage = [&](bool bReference, float Capacity)
	{
		FInventoryStartupData Startup;
		Startup.Settings.InventoryClass = USlotbasedInventory::StaticClass();
		Startup.Settings.InventoryWidgetClass = UUInventoryBaseWidget::StaticClass();
		Startup.Settings.ContainerWidgetClass = UInventoryContainerWidget::StaticClass();
		Startup.Settings.bIsReferenceContainer = bReference;
		Startup.Settings.bCollectInvDataFromWidget = false;
		Startup.Settings.InventoryMaxWeightCapacity = Capacity;
		Startup.Settings.InventorySlotBasedSettings.InitNumberRows = 1;
		Startup.Settings.InventorySlotBasedSettings.InitNumColumns = 3;
		auto* Inventory = UInventoryBase::CreateInventoryAdvanced(Pawn, Startup, Pawn, Collection);
		Collection->AddPawnInventory_Internal(Inventory);
		return Inventory;
	};
	auto* Storage = CreateStorage(false, 3.f);
	auto* Hotbar = CreateStorage(true, 100.f);
	FindFProperty<FObjectPropertyBase>(Manager->GetClass(), TEXT("MainPawnInventoryRef"))->SetObjectPropertyValue_InContainer(Manager, Storage);
	bool bAttachDrop = true;
	const auto Handler = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda(
		[&](AActor* Actor)
		{
			if (Actor->GetOwner() == Pawn && bAttachDrop)
			{
				auto* Pickup = NewObject<UPickupComponent>(Actor);
				Actor->AddInstanceComponent(Pickup);
				Pickup->RegisterComponent();
			}
		}));
	auto CreateBundle = [&]()
	{
		auto* Actor = World->SpawnActor<AActor>();
		auto* Pickup = NewObject<UPickupComponent>(Actor);
		Actor->AddInstanceComponent(Pickup);
		Pickup->RegisterComponent();
		Pickup->InitializeItems({Entry(TEXT("ResourceA"), 4), Entry(TEXT("ResourceB"), 2)});
		return Pickup;
	};
	auto Quantity = [](const TArray<UObject*>& Items, FName ID)
	{
		int32 Total = 0;
		for (auto* Item : Items)
		{
			if (IObjectDataProvider::Execute_GetItemID(Item) == ID)
			{
				Total += IObjectDataProvider::Execute_GetQuantity(Item);
			}
		}
		return Total;
	};
	auto GroundItems = [&]()
	{
		TArray<UObject*> Result;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (!It->IsActorBeingDestroyed() && It->GetOwner() == Pawn)
			{
				if (auto* Pickup = It->FindComponentByClass<UPickupComponent>())
				{
					Result.Append(Pickup->GetPickupItems());
				}
			}
		}
		return Result;
	};
	auto* Bundle = CreateBundle();
	TestTrue(TEXT("Nonempty list enables bundle mode"), Bundle->IsBundlePickup());
	TestEqual(TEXT("Oversize configured quantity is split, not clamped away"), Quantity(Bundle->GetPickupItems(), TEXT("ResourceA")), 4);
	TestEqual(TEXT("Configured list creates three independent stacks"), Bundle->GetPickupItems().Num(), 3);
	TestFalse(TEXT("Pickup requires interaction lock ownership"), Bundle->CollectItems(Manager));
	TestTrue(TEXT("Acquire bundle"), Bundle->TryAcquireInteraction(Pawn));
	TestTrue(TEXT("Pickup accepts partial storage and drops rest"), Bundle->CollectItems(Manager));
	TestTrue(TEXT("Source box is destroyed"), Bundle->GetOwner()->IsActorBeingDestroyed());
	TestEqual(TEXT("Exactly three resource A units enter capacity-limited bag"), Quantity(Collection->GetAllItemsByContainer(Storage->GetInventoryContainerID()), TEXT("ResourceA")), 3);
	TestEqual(TEXT("One resource A unit is on the ground"), Quantity(GroundItems(), TEXT("ResourceA")), 1);
	TestEqual(TEXT("All resource B units are on the ground"), Quantity(GroundItems(), TEXT("ResourceB")), 2);
	TestTrue(TEXT("Hotbar is excluded from bundle storage"), Collection->GetAllItemsByContainer(Hotbar->GetInventoryContainerID()).IsEmpty());
	TestFalse(TEXT("Repeated stale request cannot deliver a second time"), Bundle->CollectItems(Manager));
	TSet<FGuid> IDs;
	auto AllItems = GroundItems();
	AllItems.Append(Collection->GetAllItemsByContainer(Storage->GetInventoryContainerID()));
	for (auto* Object : AllItems)
	{
		auto* Item = CastChecked<UItemBase>(Object);
		TestFalse(TEXT("Every physical output has a unique identity"), IDs.Contains(Item->GetPersistentInstanceID()));
		IDs.Add(Item->GetPersistentInstanceID());
		TestFalse(TEXT("Inventory item does not retain destroyed source as outer"), Item->IsIn(Bundle->GetOwner()));
	}

	// A second ordinary storage is used after the full main bag.
	auto* Pockets = CreateStorage(false, 2.f);
	auto* Second = CreateBundle();
	Second->TryAcquireInteraction(Pawn);
	TestTrue(TEXT("Additional permanent storage is supported"), Second->CollectItems(Manager));
	TestEqual(TEXT("Pockets receive the two units that fit"), Quantity(Collection->GetAllItemsByContainer(Pockets->GetInventoryContainerID()), TEXT("ResourceA")), 2);

	// Without player storage every resource is dropped, then the source disappears.
	FindFProperty<FObjectPropertyBase>(Manager->GetClass(), TEXT("MainPawnInventoryRef"))->SetObjectPropertyValue_InContainer(Manager, nullptr);
	FindFProperty<FArrayProperty>(Collection->GetClass(), TEXT("ActorInventories"))
		->ContainerPtrToValuePtr<TArray<TObjectPtr<UInventoryBase>>>(Collection)->Reset();
	auto* NoBag = CreateBundle();
	NoBag->TryAcquireInteraction(Pawn);
	const int32 GroundBefore = Quantity(GroundItems(), TEXT("ResourceA"));
	TestTrue(TEXT("No backpack still consumes source and drops everything"), NoBag->CollectItems(Manager));
	TestEqual(TEXT("No backpack adds all four resource A units to ground"), Quantity(GroundItems(), TEXT("ResourceA")), GroundBefore + 4);

	// Spawn failures deliberately discard overflow, report an error, and still consume the box.
	bAttachDrop = false;
	auto* Failed = CreateBundle();
	Failed->TryAcquireInteraction(Pawn);
	AddExpectedError(TEXT("remainder discarded"), EAutomationExpectedErrorFlags::Contains, 3);
	TestTrue(TEXT("Failed ground creation still completes collection"), Failed->CollectItems(Manager));
	TestTrue(TEXT("Failed ground creation does not retain source"), Failed->GetOwner()->IsActorBeingDestroyed());
	TestEqual(TEXT("Failed ground creation adds no phantom units"), Quantity(GroundItems(), TEXT("ResourceA")), GroundBefore + 4);
	bAttachDrop = true;

	// Save/load keeps every stack and its identity rather than silently saving only the first.
	auto* Persistent = CreateBundle();
	FString Error;
	auto* Snapshot = Save->CaptureSnapshot(Error);
	if (TestNotNull(*Error, Snapshot))
	{
		const auto* Record = Snapshot->Pickups.FindByPredicate([&](const auto& Saved)
		{
			return Saved.Key == Persistent->GetPersistentSaveKey();
		});
		if (TestNotNull(TEXT("Bundle is present in snapshot"), Record))
		{
			TestTrue(TEXT("Saved record identifies bundle mode"), Record->bIsBundle);
			TestEqual(TEXT("All three runtime stacks are saved"), Record->Items.Num(), 3);
			const FGuid FirstID = CastChecked<UItemBase>(Persistent->PeekItemData())->GetPersistentInstanceID();
			Persistent->InitializeItems({Entry(TEXT("ResourceB"), 1)});
			TestTrue(*Error, Save->ApplySnapshot(Snapshot, Error));
			TestEqual(TEXT("Load restores original resource A total"), Quantity(Persistent->GetPickupItems(), TEXT("ResourceA")), 4);
			TestEqual(TEXT("Load restores original item identity"), CastChecked<UItemBase>(Persistent->PeekItemData())->GetPersistentInstanceID(), FirstID);
		}
	}

	// A separately selected mesh must never be replaced by the first resource mesh.
	auto* VisualActor = World->SpawnActor<AActor>();
	auto* VisualMesh = NewObject<UStaticMeshComponent>(VisualActor);
	VisualActor->AddInstanceComponent(VisualMesh);
	VisualActor->SetRootComponent(VisualMesh);
	VisualMesh->RegisterComponent();
	auto* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	auto* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	auto VisualRow = *Table->FindRow<FItemData>(TEXT("ResourceA"), TEXT("MultiPickup"));
	VisualRow.ItemMetaData.ItemAssetData.Mesh = Cube;
	VisualRow.ItemMetaData.ItemAssetData.MeshScale = FVector(2.f);
	Table->AddRow(TEXT("ResourceA"), VisualRow);
	auto* Visual = NewObject<UPickupComponent>(VisualActor);
	VisualActor->AddInstanceComponent(Visual);
	Visual->MeshSource = EPickupMeshSource::CustomMesh;
	Visual->CustomMesh = Sphere;
	Visual->CustomMeshScale = FVector(0.5f);
	auto* Actions = FindFProperty<FMapProperty>(Visual->GetClass(), TEXT("InteractableDataMap"))
		->ContainerPtrToValuePtr<TMap<EInteractionType, FInteractableData>>(Visual);
	FInteractableData Action;
	Action.bHoldToInteract = true;
	Action.InteractableDuration = 2.f;
	Action.ActionTag = FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Ingredient"));
	Actions->Add(EInteractionType::Primary, Action);
	Visual->RegisterComponent();
	Visual->InitializeItems({Entry(TEXT("ResourceA"), 2)});
	TestTrue(TEXT("Separate mesh overrides the first resource mesh"), VisualMesh->GetStaticMesh() == Sphere);
	Visual->RefreshAppearance();
	TestEqual(TEXT("Repeated mesh refresh does not compound scale"), VisualMesh->GetRelativeScale3D(), FVector(0.5f));
	Visual->MeshSource = EPickupMeshSource::FirstItem;
	Visual->RefreshAppearance();
	TestTrue(TEXT("First item mode selects the resource mesh"), VisualMesh->GetStaticMesh() == Cube);
	TestEqual(TEXT("First item mode applies authored resource scale"), VisualMesh->GetRelativeScale3D(), FVector(2.f));
	TestTrue(TEXT("Bundle initialization preserves hold requirement"), Visual->GetInteractableData().bHoldToInteract);
	TestEqual(TEXT("Bundle initialization preserves authored hold duration"), Visual->GetInteractableData().InteractableDuration, 2.f);
	TestEqual(TEXT("Bundle initialization preserves authored action tag"), Visual->GetInteractableData().ActionTag, Action.ActionTag);

	// The legacy singular format and its all-or-nothing behavior remain valid.
	auto* LegacyActor = World->SpawnActor<AActor>();
	auto* Legacy = NewObject<UPickupComponent>(LegacyActor);
	LegacyActor->AddInstanceComponent(Legacy);
	Legacy->RegisterComponent();
	Legacy->InitializeDrop(Entry(TEXT("ResourceA"), 2));
	Legacy->TryAcquireInteraction(Pawn);
	TestFalse(TEXT("Legacy single item without bag is not consumed"), Legacy->CollectItems(Manager));
	TestFalse(TEXT("Legacy source survives failed ordinary pickup"), LegacyActor->IsActorBeingDestroyed());
	FInventorySavedPickup OldRecord;
	OldRecord.Item.InstanceID = FGuid::NewGuid();
	TestEqual(TEXT("Old singular save records are still readable"), OldRecord.GetSavedItems().Num(), 1);

	World->RemoveOnActorSpawnedHandler(Handler);
	Instance->Shutdown();
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return !HasAnyErrors();
}
#endif
