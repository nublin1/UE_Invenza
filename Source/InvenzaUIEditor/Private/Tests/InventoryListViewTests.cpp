// Nublin Studio 2026 All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "ActorComponents/InventoryCollectionComponent.h"
#include "Components/ListView.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Inventory/ListInventory/InventoryListEntry.h"
#include "Data/Inventory/ListInventory/ListInventory.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/Items/itemBase.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "UI/Inventory/InventoryListViewWidget.h"
#include "UI/Inventory/InventoryViewEntry.h"
#include "Widgets/SWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvenzaReadOnlyListViewTest, "Invenza.UI.ReadOnlyListView",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

static UInventoryBase* CreateListViewTestInventory(APawn* Owner, UInventoryCollectionComponent* Collection,
	UClass* InventoryClass)
{
	FInventoryStartupData Startup;
	Startup.Settings.InventoryClass = InventoryClass;
	Startup.Settings.EntryClass = UInventoryListEntry::StaticClass();
	Startup.Settings.bCollectInvDataFromWidget = false;
	Startup.Settings.InventorySlotBasedSettings.InitNumberRows = 3;
	Startup.Settings.InventorySlotBasedSettings.InitNumColumns = 3;
	UInventoryBase* Inventory = UInventoryBase::CreateInventoryAdvanced(Collection, Startup, Owner, Collection);
	Collection->AddPawnInventory_Internal(Inventory);
	return Inventory;
}

static UItemBase* CreateListViewTestItem(UObject* Outer, const TCHAR* Name, int32 Quantity)
{
	UItemBase* Item = NewObject<UItemBase>(Outer);
	FItemMetaData Data;
	Data.ItemTextData.DisplayName = FText::FromString(Name);
	Item->SetItemRef(Data);
	Item->SetQuantity_Implementation(Quantity);
	return Item;
}

static FItemMapping AddListViewTestItem(UInventoryCollectionComponent* Collection, UInventoryBase* Inventory,
	UObject* Item, UInventorySlotData* Slot)
{
	FItemMapping Mapping;
	Mapping.InventoryID = Inventory->GetInventoryContainerID();
	Mapping.OccupiedSlots.Add(Slot);
	return Collection->AddItem(Item, Mapping);
}

bool FInvenzaReadOnlyListViewTest::RunTest(const FString& Parameters)
{
	UClass* ViewClass = LoadClass<UInventoryListViewWidget>(nullptr,
		TEXT("/InventorySystemInvenzaPlugin/UI/Inventory/Views/WBP_InventoryListView.WBP_InventoryListView_C"));
	if (!TestNotNull(TEXT("Designer-authored list view"), ViewClass))
	{
		return false;
	}
	UGameInstance* Instance = NewObject<UGameInstance>(GEngine);
	Instance->InitializeStandalone();
	UWorld* World = Instance->GetWorld();
	APawn* Owner = World->SpawnActor<APawn>();
	UInventoryCollectionComponent* Collection = NewObject<UInventoryCollectionComponent>(Owner);
	Collection->bPersistInventories = false;
	Owner->AddInstanceComponent(Collection);
	Collection->RegisterComponent();
	USlotbasedInventory* Grid = CastChecked<USlotbasedInventory>(
		CreateListViewTestInventory(Owner, Collection, USlotbasedInventory::StaticClass()));
	UListInventory* List = CastChecked<UListInventory>(
		CreateListViewTestInventory(Owner, Collection, UListInventory::StaticClass()));
	UItemBase* Apples = CreateListViewTestItem(Collection, TEXT("Apples"), 5);
	UItemBase* Rope = CreateListViewTestItem(Collection, TEXT("Rope"), 2);
	const TArray<UInventorySlotData*> OriginalSlots = Grid->GetInventorySlots();
	const FIntPoint OriginalSize = Grid->GetInventorySize();
	AddListViewTestItem(Collection, Grid, Apples, OriginalSlots[0]);
	AddListViewTestItem(Collection, Grid, Rope, OriginalSlots[1]);
	AddListViewTestItem(Collection, List, Apples, NewObject<UInventorySlotData>(List));
	TArray<FGuid> OriginalPositions = Collection->GetOccupatedSlotsIDByContainerName(Grid->GetInventoryContainerID(), Apples);
	FString OriginalSlotState;
	FInventorySlotInfo::StaticStruct()->ExportText(OriginalSlotState, &OriginalSlots[0]->InventorySlotInfo,
		nullptr, nullptr, PPF_None, nullptr);

	UInventoryListViewWidget* First = CreateWidget<UInventoryListViewWidget>(World, ViewClass);
	UInventoryListViewWidget* Second = CreateWidget<UInventoryListViewWidget>(World, ViewClass);
	TSharedPtr<SWidget> FirstSlate = First->TakeWidget();
	TSharedPtr<SWidget> SecondSlate = Second->TakeWidget();
	First->SetSourceInventory(Grid);
	Second->SetSourceInventory(Grid);
	TestEqual(TEXT("Grid yields one row per stack"), First->GetViewEntries().Num(), 2);
	UInventoryViewEntry* FirstAppleEntry = First->GetViewEntries()[0];
	TestNotEqual(TEXT("Rows belong to their own view"), FirstAppleEntry, Second->GetViewEntries()[0]);
	First->SetSearchText(FText::FromString(TEXT("app")));
	TestEqual(TEXT("First view filters locally"), First->GetVisibleEntries().Num(), 1);
	TestEqual(TEXT("Second view keeps its own filter"), Second->GetVisibleEntries().Num(), 2);
	Second->SetSortMode(EInventoryViewSortMode::QuantityDescending);
	TestEqual(TEXT("Local quantity sort"), Second->GetVisibleEntries()[0]->GetItem(), static_cast<UObject*>(Apples));

	// Simulate a client quantity notification and a collection update in the same frame.
	Apples->SetQuantity_Implementation(9);
	Apples->OnItemDataReplicated.Broadcast(Apples);
	Collection->RemoveItem(Rope, Grid->GetInventoryContainerID());
	Collection->OnInventoryItemsChanged.Broadcast(Grid->GetInventoryContainerID());
	World->GetTimerManager().Tick(0.01f);
	TestEqual(TEXT("Quantity event updates existing entry"), FirstAppleEntry->Quantity, 9);
	TestEqual(TEXT("Entry identity survives updates"), First->GetViewEntries()[0], FirstAppleEntry);
	TestEqual(TEXT("Composition event reaches both views"), Second->GetViewEntries().Num(), 1);

	Second->SetSourceInventory(List);
	TestEqual(TEXT("The same widget supports list inventory"), Second->GetViewEntries().Num(), 1);
	TestEqual(TEXT("A native list slot was not created for viewing"), List->InvSlotsArray.Num(), 0);
	TestEqual(TEXT("Native filtered slots remain untouched"), List->FilteredInvSlotsArray.Num(), 0);
	TestTrue(TEXT("Grid slot identities remain unchanged"), Grid->GetInventorySlots() == OriginalSlots);
	TestTrue(TEXT("Grid dimensions remain unchanged"), Grid->GetInventorySize() == OriginalSize);
	TestTrue(TEXT("Item coordinates remain unchanged"), Collection->GetOccupatedSlotsIDByContainerName(
		Grid->GetInventoryContainerID(), Apples) == OriginalPositions);
	FString CurrentSlotState;
	FInventorySlotInfo::StaticStruct()->ExportText(CurrentSlotState, &OriginalSlots[0]->InventorySlotInfo,
		nullptr, nullptr, PPF_None, nullptr);
	TestEqual(TEXT("Slot state and reservations remain unchanged"), CurrentSlotState, OriginalSlotState);
	TestTrue(TEXT("Views never register as the primary inventory UI"), Collection->GetContainerWidgetMap().IsEmpty());

	First->SetSourceInventory(nullptr);
	TestTrue(TEXT("Clearing source removes rows"), First->GetViewEntries().IsEmpty());
	First->SetSourceInventory(Grid);
	TestEqual(TEXT("Reconnecting fills from current data"), First->GetViewEntries().Num(), 1);
	Owner->OnEndPlay.Broadcast(Owner, EEndPlayReason::Destroyed);
	TestTrue(TEXT("Owner destruction clears the first view"), First->GetViewEntries().IsEmpty());
	TestTrue(TEXT("Owner destruction clears the second view"), Second->GetViewEntries().IsEmpty());
	FirstSlate.Reset();
	SecondSlate.Reset();
	Owner->Destroy();
	Instance->Shutdown();
	return true;
}
#endif
