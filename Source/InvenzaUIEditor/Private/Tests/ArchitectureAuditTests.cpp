// Nublin Studio 2026 All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "ActorComponents/Crafting/CraftingComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/ItemData.h"
#include "Data/Items/itemBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Factory/ItemFactory.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "SaveLoad/InvenzaSaveGame.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "Subsystems/PortableInventorySubsystem.h"
#include "UObject/StructOnScope.h"
#include "UObject/UnrealType.h"
#include "Utility/InventoryGrantService.h"
#include "World/PortableInventoryState.h"

static UPickupComponent* CreateAuditPhysicsPickup(AActor* Actor)
{
	auto* Body = NewObject<UBoxComponent>(Actor);
	Actor->AddInstanceComponent(Body);
	Actor->SetRootComponent(Body);
	Body->SetBoxExtent(FVector(16.f));
	Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Body->SetSimulatePhysics(true);
	Body->RegisterComponent();
	auto* Pickup = NewObject<UPickupComponent>(Actor);
	Actor->AddInstanceComponent(Pickup);
	Pickup->RegisterComponent();
	return Pickup;
}

static void InvokeAuditInteractionRPC(UObject* Object, FName FunctionName, UInteractableComponent* Target,
	uint32 Serial = 1)
{
	auto* Function = Object->FindFunctionChecked(FunctionName);
	FStructOnScope Parameters(Function);
	FindFProperty<FObjectPropertyBase>(Function, TEXT("Target"))->SetObjectPropertyValue_InContainer(
		Parameters.GetStructMemory(), Target);
	if (auto* Property = FindFProperty<FUInt32Property>(Function, TEXT("Serial")))
	{
		Property->SetPropertyValue_InContainer(Parameters.GetStructMemory(), Serial);
	}
	// Primary is the zero-valued action in the zero-initialized parameter frame.
	Object->ProcessEvent(Function, Parameters.GetStructMemory());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvenzaArchitectureAuditTest, "Invenza.Inventory.ArchitectureAudit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInvenzaArchitectureAuditTest::RunTest(const FString& Parameters)
{
	auto* Instance = NewObject<UGameInstance>(GEngine);
	Instance->InitializeStandalone();
	auto* World = Instance->GetWorld();
	auto* Save = Instance->GetSubsystem<UInvenzaSaveSubsystem>();
	Save->SetProfile(TEXT("ArchitectureAuditAutomation"));
	auto* SettingsSubsystem = Instance->GetSubsystem<UInvenzaInventorySettingsSubsystem>();
	auto* Settings = DuplicateObject<UInvenzaInventorySettingsAsset>(SettingsSubsystem->GetSettings(), Instance);
	FindFProperty<FObjectPropertyBase>(SettingsSubsystem->GetClass(), TEXT("Settings"))
		->SetObjectPropertyValue_InContainer(SettingsSubsystem, Settings);
	Settings->PickupClass = AActor::StaticClass();
	auto* Pawn = World->SpawnActor<APawn>();
	auto* Collection = NewObject<UInventoryCollectionComponent>(Pawn);
	Pawn->AddInstanceComponent(Collection);
	Collection->RegisterComponent();
	Collection->InitItemCollection();
	Collection->SetRuntimeSaveIdentity(TEXT("Player:ArchitectureAudit"));
	auto* Manager = NewObject<UIInventoryManager>(Pawn);
	Pawn->AddInstanceComponent(Manager);
	Manager->RegisterComponent();
	FindFProperty<FObjectPropertyBase>(Manager->GetClass(), TEXT("ItemCollectionRef"))
		->SetObjectPropertyValue_InContainer(Manager, Collection);
	FindFProperty<FObjectPropertyBase>(Manager->GetClass(), TEXT("GlobalSettings"))
		->SetObjectPropertyValue_InContainer(Manager, Settings);
	Save->RegisterCollection(Collection);
	auto CreateGrid = [&](float Capacity, bool bReceiving, bool bReference = false)
	{
		FInventoryStartupData Startup;
		Startup.Settings.InventoryClass = USlotbasedInventory::StaticClass();
		Startup.Settings.bCollectInvDataFromWidget = false;
		Startup.Settings.bOpenWithExternalInventory = false;
		Startup.Settings.bReceiveGrantedItems = bReceiving;
		Startup.Settings.bIsReferenceContainer = bReference;
		Startup.Settings.InventoryMaxWeightCapacity = Capacity;
		Startup.Settings.InventorySlotBasedSettings.InitNumberRows = 1;
		Startup.Settings.InventorySlotBasedSettings.InitNumColumns = 2;
		auto* Grid = UInventoryBase::CreateInventoryAdvanced(Pawn, Startup, Pawn, Collection);
		Collection->AddPawnInventory_Internal(Grid);
		return Grid;
	};
	auto* Main = CreateGrid(2.f, true);
	auto* Pocket = CreateGrid(10.f, true);
	auto* Service = CreateGrid(100.f, false);
	auto* Reference = CreateGrid(100.f, true, true);
	auto* Gear = CreateGrid(100.f, true);
	auto GearSettings = Gear->GetInventorySettings();
	GearSettings.InventoryTag = Settings->EquipmentInvTag;
	Gear->SetInventorySettings(GearSettings);
	FindFProperty<FObjectPropertyBase>(Manager->GetClass(), TEXT("MainPawnInventoryRef"))
		->SetObjectPropertyValue_InContainer(Manager, Main);
	const auto Receiving = Manager->GetReceivingInventories();
	TestEqual(TEXT("Only main storage and pocket receive automatic grants"), Receiving.Num(), 2);
	TestTrue(TEXT("Primary storage is first"), Receiving[0] == Main);
	TestTrue(TEXT("Storage without any UI can receive"), Receiving.Contains(Pocket));
	TestFalse(TEXT("Service storage is excluded"), Receiving.Contains(Service));
	TestFalse(TEXT("Reference hotbar is excluded"), Receiving.Contains(Reference));
	TestFalse(TEXT("Equipment is excluded"), Receiving.Contains(Gear));
	TestTrue(TEXT("UI filtering stays independent"), Manager->GetInteractionInventories().IsEmpty());
	auto* DisplayLinks = FindFProperty<FStructProperty>(Collection->GetClass(), TEXT("LinkedInventories"))
		->ContainerPtrToValuePtr<FLinkedInventories>(Collection);
	DisplayLinks->ExternalInventory = Pocket;
	auto* CraftLinks = FindFProperty<FStructProperty>(Collection->GetClass(), TEXT("LinkedCraftInventories"))
		->ContainerPtrToValuePtr<FLinkedCraftInventories>(Collection);
	CraftLinks->InputInventory = Main;
	TestTrue(TEXT("Displaying owned pockets as external does not disable receiving"), Manager->GetReceivingInventories().Contains(Pocket));
	TestTrue(TEXT("Using main storage for crafting does not disable receiving"), Manager->GetReceivingInventories().Contains(Main));
	auto* CraftServiceStorage = CreateGrid(100.f, true);
	auto* ServiceCraft = NewObject<UCraftingComponent>(Pawn);
	Pawn->AddInstanceComponent(ServiceCraft);
	ServiceCraft->RegisterComponent();
	FindFProperty<FObjectPropertyBase>(ServiceCraft->GetClass(), TEXT("InputInventory"))
		->SetObjectPropertyValue_InContainer(ServiceCraft, CraftServiceStorage);
	TestFalse(TEXT("Actual crafting service storage is excluded without UI links"), Manager->GetReceivingInventories().Contains(CraftServiceStorage));
	DisplayLinks->ExternalInventory = nullptr;
	CraftLinks->InputInventory = nullptr;
	auto* Table = NewObject<UDataTable>(Instance);
	Table->RowStruct = FItemData::StaticStruct();
	FItemData Resource;
	Resource.ID = TEXT("AuditResource");
	Resource.ItemMetaData.ItemCategory = FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Ingredient"));
	Resource.ItemMetaData.ItemNumeraticData.Weight = 1.f;
	Resource.ItemMetaData.ItemNumeraticData.MaxStackSizeInCharacter = 20;
	Table->AddRow(Resource.ID, Resource);
	FItemData Bag;
	Bag.ID = TEXT("AuditBag");
	Bag.ItemMetaData.ItemCategory = FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Backpack"));
	Bag.ItemMetaData.ItemInventoryData.Rows = 2;
	Bag.ItemMetaData.ItemInventoryData.Columns = 2;
	Table->AddRow(Bag.ID, Bag);
	FDataTableRowHandle ResourceRow;
	ResourceRow.DataTable = Table;
	ResourceRow.RowName = Resource.ID;
	FDataTableRowHandle BagRow;
	BagRow.DataTable = Table;
	BagRow.RowName = Bag.ID;
	auto* Stack = UItemFactory::CreateItemByHandle(Pawn, ResourceRow, 5);
	FInventoryGrantPolicy GrantPolicy;
	const auto Granted = FInventoryGrantService::Grant(Stack, Receiving, Pawn, GrantPolicy);
	TestEqual(TEXT("Grant uses pocket after main fills"), Granted.Added, 5);
	TestEqual(TEXT("No grant remainder"), Granted.Remaining, 0);
	TestEqual(TEXT("Main respects weight capacity"), Main->GetInventoryTotalWeight(), 2.f);
	TestEqual(TEXT("Pocket receives remaining quantity"), Pocket->GetInventoryTotalWeight(), 3.f);
	auto* TooLarge = UItemFactory::CreateItemByHandle(Pawn, ResourceRow, 4);
	GrantPolicy.bAllowPartial = false;
	const auto Rejected = FInventoryGrantService::Grant(TooLarge, {Main}, Pawn, GrantPolicy);
	TestEqual(TEXT("Whole-stack placement cannot partially consume source"), Rejected.Remaining, 4);
	GrantPolicy.Overflow = EInventoryGrantOverflow::DropAndRetainOnFailure;
	AddExpectedError(TEXT("Grant overflow failed"), EAutomationExpectedErrorFlags::Contains, 1);
	const auto Retained = FInventoryGrantService::Grant(TooLarge, {}, Pawn, GrantPolicy);
	TestEqual(TEXT("Failed startup drop retains all units"), Retained.Remaining, 4);
	TestEqual(TEXT("Failed startup drop discards nothing"), Retained.Discarded, 0);
	GrantPolicy.Overflow = EInventoryGrantOverflow::DropAndDiscardOnFailure;
	AddExpectedError(TEXT("Grant overflow failed"), EAutomationExpectedErrorFlags::Contains, 1);
	const auto Discarded = FInventoryGrantService::Grant(TooLarge, {}, Pawn, GrantPolicy);
	TestEqual(TEXT("Bundle explicitly discards failed overflow"), Discarded.Discarded, 4);
	TestEqual(TEXT("Discarded overflow is removed from the source"), IObjectDataProvider::Execute_GetQuantity(TooLarge), 0);

	// Reflection calls the actual RPC implementations, including forged direct completion requests.
	auto* Interaction = NewObject<UInteractionComponent>(Pawn);
	Pawn->AddInstanceComponent(Interaction);
	Interaction->RegisterComponent();
	FScriptDelegate Dispatch;
	Dispatch.BindUFunction(Manager, TEXT("InteractRequest"));
	Interaction->OnInteract.AddUnique(Dispatch);
	auto* Ground = World->SpawnActor<AActor>();
	auto* Root = NewObject<USceneComponent>(Ground);
	Ground->SetRootComponent(Root);
	Root->RegisterComponent();
	auto* Pickup = NewObject<UPickupComponent>(Ground);
	Ground->AddInstanceComponent(Pickup);
	Pickup->RegisterComponent();
	Pickup->RestorePersistentItem(UItemFactory::CreateItemByHandle(Pickup, ResourceRow, 1));
	auto* Actions = FindFProperty<FMapProperty>(Pickup->GetClass(), TEXT("InteractableDataMap"))
		->ContainerPtrToValuePtr<TMap<EInteractionType, FInteractableData>>(Pickup);
	auto& Action = Actions->FindOrAdd(EInteractionType::Primary);
	Action.DefaultInteractableType = EInteractableType::Pickup;
	Action.InteractableDuration = 0.5f;
	Action.bHoldToInteract = true;
	auto FreeMainSettings = Main->GetInventorySettings();
	FreeMainSettings.InventoryMaxWeightCapacity = 10.f;
	Main->SetInventorySettings(FreeMainSettings);
	InvokeAuditInteractionRPC(Manager, TEXT("Server_HandleInteract"), Pickup);
	TestFalse(TEXT("Direct inventory RPC cannot bypass Begin"), Ground->IsActorBeingDestroyed());
	InvokeAuditInteractionRPC(Interaction, TEXT("Server_TrackInteraction"), Pickup, 1);
	TestFalse(TEXT("Completion without a session is rejected"), Ground->IsActorBeingDestroyed());
	InvokeAuditInteractionRPC(Interaction, TEXT("Server_BeginInteraction"), Pickup, 1);
	InvokeAuditInteractionRPC(Interaction, TEXT("Server_TrackInteraction"), Pickup, 1);
	TestFalse(TEXT("Early completion cannot pick up immediately"), Ground->IsActorBeingDestroyed());
	InvokeAuditInteractionRPC(Interaction, TEXT("Server_StopInteraction"), Pickup, 1);
	TestFalse(TEXT("Cancel releases exclusive lock"), Pickup->IsInteractionOwnedBy(Pawn));
	for (int32 Tick = 0; Tick < 6; ++Tick)
	{
		World->Tick(LEVELTICK_All, 0.1f);
	}
	TestFalse(TEXT("Canceled early completion cannot execute later"), Ground->IsActorBeingDestroyed());
	InvokeAuditInteractionRPC(Interaction, TEXT("Server_BeginInteraction"), Pickup, 1);
	TestFalse(TEXT("Replayed Begin serial is rejected"), Pickup->IsInteractionOwnedBy(Pawn));
	InvokeAuditInteractionRPC(Interaction, TEXT("Server_BeginInteraction"), Pickup, 2);
	const double HoldStart = World->GetTimeSeconds();
	for (int32 Tick = 0; Tick < 6; ++Tick)
	{
		World->Tick(LEVELTICK_All, 0.1f);
	}
	TestTrue(TEXT("World advances through the server hold"), World->GetTimeSeconds() - HoldStart >= 0.5);
	InvokeAuditInteractionRPC(Interaction, TEXT("Server_TrackInteraction"), Pickup, 2);
	TestTrue(TEXT("Completed server hold collects the item"), Ground->IsActorBeingDestroyed());
	const float CompletedWeight = Main->GetInventoryTotalWeight();
	InvokeAuditInteractionRPC(Interaction, TEXT("Server_TrackInteraction"), Pickup, 2);
	TestEqual(TEXT("Duplicate completion cannot grant twice"), Main->GetInventoryTotalWeight(), CompletedWeight);

	// The actor is still present when the snapshot is loaded; both contents and transform must roll back.
	auto* SavedGround = World->SpawnActor<AActor>();
	auto* SavedPickup = CreateAuditPhysicsPickup(SavedGround);
	auto* SavedBody = Cast<UBoxComponent>(SavedGround->GetRootComponent());
	TestTrue(TEXT("Restore probe really simulates physics"), SavedBody->IsSimulatingPhysics());
	SavedPickup->RestorePersistentItem(UItemFactory::CreateItemByHandle(SavedPickup, ResourceRow, 2));
	const FTransform Original(FRotator(0.f, 45.f, 0.f), FVector(50.f, 70.f, 20.f));
	SavedGround->SetActorTransform(Original);
	Save->RegisterPickup(SavedPickup);
	FString Error;
	auto* Snapshot = Save->CaptureSnapshot(Error);
	if (TestNotNull(*Error, Snapshot))
	{
		SavedGround->SetActorLocation(FVector(800.f));
		SavedBody->SetPhysicsLinearVelocity(FVector(100.f, 200.f, 300.f));
		SavedBody->SetPhysicsAngularVelocityInDegrees(FVector(40.f));
		if (TestTrue(*Error, Save->ApplySnapshot(Snapshot, Error)))
		{
			TestTrue(TEXT("Existing pickup returns to saved transform"), SavedGround->GetActorTransform().Equals(Original));
			TestTrue(TEXT("Loading clears physical linear velocity"), SavedBody->GetPhysicsLinearVelocity().IsNearlyZero());
			TestTrue(TEXT("Loading clears physical angular velocity"), SavedBody->GetPhysicsAngularVelocityInDegrees().IsNearlyZero());
		}
		AActor* RecreatedGround = nullptr;
		const auto SpawnHandler = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda(
			[&RecreatedGround](AActor* Actor)
			{
				if (Actor->GetClass() == AActor::StaticClass())
				{
					CreateAuditPhysicsPickup(Actor);
					RecreatedGround = Actor;
				}
			}));
		SavedGround->Destroy();
		if (TestTrue(*Error, Save->ApplySnapshot(Snapshot, Error)) &&
			TestNotNull(TEXT("Missing pickup is recreated"), RecreatedGround))
		{
			SavedGround = RecreatedGround;
			TestTrue(TEXT("Recreated pickup uses the same saved transform"), SavedGround->GetActorTransform().Equals(Original));
			TestEqual(TEXT("Recreated pickup restores contents"),
				IObjectDataProvider::Execute_GetQuantity(SavedGround->FindComponentByClass<UPickupComponent>()->GetPickupItems()[0]), 2);
		}
		World->RemoveOnActorSpawnedHandler(SpawnHandler);
	}

	// Registry lookup and audience remain independent of distance and unrelated backpack count.
	auto* Registry = World->GetSubsystem<UPortableInventorySubsystem>();
	auto* Controller = World->SpawnActor<APlayerController>();
	Controller->Possess(Pawn);
	auto* OtherPawn = World->SpawnActor<APawn>();
	auto* OtherController = World->SpawnActor<APlayerController>();
	OtherController->Possess(OtherPawn);
	auto* Portable = UItemFactory::CreateItemByHandle(Pawn, BagRow, 1);
	auto* State = Portable->EnsurePortableState();
	if (TestNotNull(TEXT("Portable state initialized"), State))
	{
		TestEqual(TEXT("GUID registry returns the exact state"), Registry->FindState(Portable->GetPersistentInstanceID()), State);
		TestTrue(TEXT("Carrier receives contents"), State->CanReplicateTo(Controller));
		TestFalse(TEXT("Unrelated player does not receive contents"), State->CanReplicateTo(OtherController));
		State->SetCarrier(OtherPawn);
		TestFalse(TEXT("Former carrier loses access"), State->CanReplicateTo(Controller));
		TestTrue(TEXT("New carrier gains access"), State->CanReplicateTo(OtherController));
		State->SetCarrier(SavedGround);
		TestFalse(TEXT("Ground bag is not broadcast to nearby players"), State->CanReplicateTo(Controller));
		auto* Linked = FindFProperty<FStructProperty>(Collection->GetClass(), TEXT("LinkedInventories"))
			->ContainerPtrToValuePtr<FLinkedInventories>(Collection);
		Linked->ExternalInventory = State->GetInventory();
		TestTrue(TEXT("Active ground-container observer receives contents"), State->CanReplicateTo(Controller));
		Linked->ExternalInventory = nullptr;
		TestFalse(TEXT("Closing the container revokes observation"), State->CanReplicateTo(Controller));
		auto* Content = UItemFactory::CreateItemByHandle(State, ResourceRow, 2);
		FInventoryGrantPolicy ContentPolicy;
		FInventoryGrantService::Grant(Content, {State->GetInventory()}, Pawn, ContentPolicy);
		Portable->RefreshPortableSummary();
		TestTrue(TEXT("Replicated summary records contents"),
			FindFProperty<FBoolProperty>(Portable->GetClass(), TEXT("bPortableHasContents"))->GetPropertyValue_InContainer(Portable));
		TestEqual(TEXT("Replicated summary includes contents weight"),
			FindFProperty<FFloatProperty>(Portable->GetClass(), TEXT("PortableContentsWeight"))->GetPropertyValue_InContainer(Portable), 2.f);
		TestFalse(TEXT("Filled state cannot be permanently destroyed"), State->DestroyPermanently());
		for (auto* Item : State->Collection->GetAllItemsByContainer(State->GetInventory()->GetInventoryContainerID()))
		{
			State->Collection->RemoveItemFromAllContainers(Item);
		}
		auto* Craft = NewObject<UCraftingComponent>(Pawn);
		Pawn->AddInstanceComponent(Craft);
		Craft->RegisterComponent();
		auto* Deliveries = FindFProperty<FArrayProperty>(Craft->GetClass(), TEXT("PendingDeliveries"))
			->ContainerPtrToValuePtr<TArray<FCraftItemBatch>>(Craft);
		auto& Refund = Deliveries->AddDefaulted_GetRef();
		Refund.Inventory = State->GetInventory();
		Refund.Amount = 1;
		Craft->RefreshInventoryObligationIndex();
		TestTrue(TEXT("Indexed craft refund blocks state deletion"), State->HasCraftObligations());
		TestFalse(TEXT("Outstanding refund cannot be destroyed"), State->DestroyPermanently());
		Deliveries->Empty();
		Craft->RefreshInventoryObligationIndex();
		TestFalse(TEXT("Refund completion removes the index entry"), State->HasCraftObligations());
		Save->RegisterCollection(State->Collection);
		const FString OwnerKey = State->Collection->GetPersistentOwnerKey();
		TestTrue(TEXT("Empty unused portable state can be destroyed"), State->DestroyPermanently());
		TestNull(TEXT("Destroyed state is absent from the registry"), Registry->FindState(Portable->GetPersistentInstanceID()));
		Snapshot = Save->CaptureSnapshot(Error);
		if (TestNotNull(*Error, Snapshot))
		{
			TestFalse(TEXT("Deleted portable owner is absent from future saves"), Snapshot->Owners.ContainsByPredicate(
				[&OwnerKey](const FInventorySavedOwner& Owner)
				{
					return Owner.Key == OwnerKey;
				}));
		}
	}
	const double Start = FPlatformTime::Seconds();
	TArray<UItemBase*> Bags;
	for (int32 Index = 0; Index < 100; ++Index)
	{
		auto* Item = UItemFactory::CreateItemByHandle(Pawn, BagRow, 1);
		TestNotNull(TEXT("Bulk state registered"), Item->EnsurePortableState());
		Bags.Add(Item);
	}
	for (int32 Index = 0; Index < 10000; ++Index)
	{
		auto* Item = Bags[Index % Bags.Num()];
		if (!Registry->FindState(Item->GetPersistentInstanceID()))
		{
			AddError(TEXT("Bulk registry lookup failed"));
			break;
		}
	}
	AddInfo(FString::Printf(TEXT("100 states and 10000 GUID lookups: %.2f ms"), (FPlatformTime::Seconds() - Start) * 1000.0));
	Instance->Shutdown();
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}
#endif
