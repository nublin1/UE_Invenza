// Nublin Studio 2026 All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/ItemData.h"
#include "Data/Items/itemBase.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Factory/ItemFactory.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SaveLoad/InventorySaveTypes.h"
#include "UObject/UnrealType.h"
#include "World/Buildings/Storage/StorageGridVisualizer.h"
#include "World/PortableInventoryState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvenzaItemRowConfigurationTest, "Invenza.Inventory.ItemRowConfiguration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInvenzaItemRowConfigurationTest::RunTest(const FString& Parameters)
{
	auto* Instance = NewObject<UGameInstance>(GEngine);
	Instance->InitializeStandalone();
	auto* World = Instance->GetWorld();
	auto* Owner = World->SpawnActor<AActor>();
	auto* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	auto* FirstMaterial = UMaterialInstanceDynamic::Create(UMaterial::GetDefaultMaterial(MD_Surface), Owner);
	auto* SecondMaterial = UMaterialInstanceDynamic::Create(UMaterial::GetDefaultMaterial(MD_Surface), Owner);
	auto* Table = NewObject<UDataTable>(Instance);
	Table->RowStruct = FItemData::StaticStruct();
	FItemData Row;
	Row.ID = TEXT("OrdinaryItem");
	Row.ItemMetaData.ItemCategory = FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Ingredient"));
	Row.ItemMetaData.ItemAssetData.Mesh = Mesh;
	Row.ItemMetaData.ItemAssetData.Materials = {FirstMaterial};
	Row.ItemMetaData.ItemAssetData.MeshScale = FVector(0.5f);
	Row.ItemMetaData.ItemStorageData.StorageMesh = Mesh;
	Row.ItemMetaData.ItemStorageData.Materials = {FirstMaterial};
	Row.ItemMetaData.ItemStorageData.MeshScale = FVector(0.5f);
	Table->AddRow(Row.ID, Row);
	FDataTableRowHandle Handle;
	Handle.DataTable = Table;
	Handle.RowName = Row.ID;
	auto* Ordinary = UItemFactory::CreateItemByHandle(Owner, Handle, 1);
	TestFalse(TEXT("Ordinary category does not create a portable inventory"), Ordinary->IsPortableInventory());
	Row.ID = TEXT("SameItemAsBackpack");
	Row.ItemMetaData.ItemCategory = FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Backpack"));
	Row.ItemMetaData.ItemInventoryData.Rows = 3;
	Row.ItemMetaData.ItemInventoryData.Columns = 7;
	Row.ItemMetaData.ItemInventoryData.MaxWeight = 19.f;
	Table->AddRow(Row.ID, Row);
	Handle.RowName = Row.ID;
	auto* Backpack = UItemFactory::CreateItemByHandle(Owner, Handle, 1);
	TestTrue(TEXT("The same item becomes a backpack through its category"), Backpack->IsPortableInventory());
	auto* State = Backpack->EnsurePortableState();
	if (TestNotNull(TEXT("Row data creates the portable inventory without a definition asset"), State))
	{
		TestEqual(TEXT("Internal size is taken from the item row"), State->GetInventory()->GetInventorySize(), FIntPoint(3, 7));
		TestEqual(TEXT("Internal capacity is taken from the item row"), State->GetInventory()->GetInventorySettings().InventoryMaxWeightCapacity, 19.f);
	}
	auto* Visual = NewObject<UStaticMeshComponent>(Owner);
	Owner->AddInstanceComponent(Visual);
	Owner->SetRootComponent(Visual);
	Visual->RegisterComponent();
	auto* Pickup = NewObject<UPickupComponent>(Owner);
	Owner->AddInstanceComponent(Pickup);
	Pickup->RegisterComponent();
	FInitItemsEntry Entry;
	Entry.Item = Handle;
	Entry.Amount = 1;
	Pickup->InitializeDrop(Entry);
	TestTrue(TEXT("Pickup uses the shared mesh"), Visual->GetStaticMesh() == Mesh);
	TestTrue(TEXT("Pickup applies the row's material"), Visual->GetMaterial(0) == FirstMaterial);
	TestEqual(TEXT("Pickup applies the row's mesh scale"), Visual->GetRelativeScale3D(), FVector(0.5f));
	auto* PickupItem = Cast<UItemBase>(Pickup->PeekItemData());
	auto Metadata = IObjectDataProvider::Execute_GetItemRef(PickupItem);
	Metadata.ItemAssetData.Materials = {SecondMaterial};
	PickupItem->SetItemRef(Metadata);
	PickupItem->OnItemDataReplicated.Broadcast(PickupItem);
	TestTrue(TEXT("A replicated data update changes the material on the same mesh"), Visual->GetMaterial(0) == SecondMaterial);
	Metadata.ItemAssetData.Materials.Reset();
	PickupItem->SetItemRef(Metadata);
	PickupItem->OnItemDataReplicated.Broadcast(PickupItem);
	TestEqual(TEXT("Missing override restores the mesh material"), Visual->GetMaterial(0), Mesh->GetMaterial(0));

	auto* StorageOwner = World->SpawnActor<AActor>();
	auto* Collection = NewObject<UInventoryCollectionComponent>(StorageOwner);
	StorageOwner->AddInstanceComponent(Collection);
	Collection->RegisterComponent();
	Collection->InitItemCollection();
	FInventoryStartupData Startup;
	Startup.Settings.InventoryClass = USlotbasedInventory::StaticClass();
	Startup.Settings.bCollectInvDataFromWidget = false;
	Startup.Settings.InventorySlotBasedSettings.InitNumberRows = 2;
	Startup.Settings.InventorySlotBasedSettings.InitNumColumns = 2;
	auto* Inventory = UInventoryBase::CreateInventoryAdvanced(StorageOwner, Startup, StorageOwner, Collection);
	Collection->AddPawnInventory_Internal(Inventory);
	Handle.RowName = TEXT("OrdinaryItem");
	auto* Stored = UItemFactory::CreateItemByHandle(StorageOwner, Handle, 1);
	FItemMoveData Move;
	Move.SourceItem = Stored;
	Move.TargetInventory = Inventory;
	TestEqual(TEXT("Place the item in storage"), Inventory->HandleAddItem(Move).ActualAmountAdded, 1);
	auto* Storage = NewObject<UStorageGridVisualizer>(StorageOwner);
	StorageOwner->AddInstanceComponent(Storage);
	StorageOwner->SetRootComponent(Storage);
	Storage->SetStaticMesh(Mesh);
	Storage->RegisterComponent();
	Storage->RefreshItemVisuals();
	auto* Visuals = FindFProperty<FMapProperty>(Storage->GetClass(), TEXT("ItemVisuals"))
		->ContainerPtrToValuePtr<TMap<TObjectPtr<UObject>, TObjectPtr<UStaticMeshComponent>>>(Storage);
	auto* StoredVisual = Visuals->FindRef(Stored).Get();
	if (TestNotNull(TEXT("Storage renders the item"), StoredVisual))
	{
		TestTrue(TEXT("Storage applies the row's label material"), StoredVisual->GetMaterial(0) == FirstMaterial);
		TestEqual(TEXT("Storage preserves its authored size using the common mesh"), StoredVisual->GetRelativeScale3D(), FVector(0.5f));
	}
	auto StorageMetadata = IObjectDataProvider::Execute_GetItemRef(Stored);
	StorageMetadata.ItemStorageData.StorageMethod = EStorageMethod::SmallBoxes;
	StorageMetadata.ItemStorageData.Materials = {SecondMaterial};
	Stored->SetItemRef(StorageMetadata);
	Stored->SetQuantity_Implementation(3);
	Storage->RefreshItemVisuals();
	auto* Stack = Cast<UInstancedStaticMeshComponent>(Visuals->FindRef(Stored));
	if (TestNotNull(TEXT("The same mesh supports a stack of small boxes"), Stack))
	{
		TestEqual(TEXT("Small-box stack keeps all instances"), Stack->GetInstanceCount(), 3);
		TestTrue(TEXT("Small-box instances use the row's material"), Stack->GetMaterial(0) == SecondMaterial);
		StorageMetadata.ItemStorageData.MeshScale = FVector(0.25f);
		Stored->SetItemRef(StorageMetadata);
		Storage->RefreshItemVisuals();
		FTransform Transform;
		Stack->GetInstanceTransform(0, Transform);
		TestEqual(TEXT("Changing row scale rebuilds existing box instances"), Transform.GetScale3D(), FVector(0.25f));
	}
	FInventorySavedItem Legacy;
	Legacy.InstanceID = FGuid::NewGuid();
	Legacy.Quantity = 1;
	Legacy.DefinitionTable = FSoftObjectPath(TEXT("/InventorySystemInvenzaPlugin/Data/Portable/DT_Backpacks.DT_Backpacks"));
	Legacy.DefinitionRow = TEXT("BackpackSmall");
	auto* Migrated = UItemFactory::CreateItemFromSave(Owner, Legacy);
	if (TestNotNull(TEXT("Old saves resolve the moved backpack row"), Migrated))
	{
		TestEqual(TEXT("Definition migration preserves the instance identity"), Migrated->GetPersistentInstanceID(), Legacy.InstanceID);
		TestTrue(TEXT("The migrated item is still a backpack"), Migrated->IsPortableInventory());
	}
	Instance->Shutdown();
	return true;
}
#endif
