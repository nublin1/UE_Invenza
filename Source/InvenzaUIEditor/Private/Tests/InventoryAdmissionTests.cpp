// Nublin Studio 2026 All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/Inventory/ListInventory/ListInventory.h"
#include "Data/Inventory/ListInventory/InventoryListEntry.h"
#include "Data/Items/itemBase.h"
#include "Data/ItemData.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Factory/ItemFactory.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/TreeView.h"
#include "Components/EditableTextBox.h"
#include "UI/Inventory/Admission/InventoryAdmissionWidget.h"
#include "UI/Inventory/Admission/InventoryCategoryTreeEntry.h"
#include "UI/Core/Buttons/UIButton.h"
#include "SaveLoad/InventorySaveTypes.h"
#include "UObject/UnrealType.h"
#include "Widgets/SWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvenzaInventoryAdmissionTest, "Invenza.Inventory.Admission",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

static UInventoryBase* CreateAdmissionTestInventory(UInventoryCollectionComponent* Collection, UClass* Class)
{
	FInventoryStartupData Startup;
	Startup.Settings.InventoryClass = Class;
	Startup.Settings.EntryClass = UInventoryListEntry::StaticClass();
	Startup.Settings.bCollectInvDataFromWidget = false;
	Startup.Settings.bAllowAdmissionRulesEditing = true;
	Startup.Settings.InventorySlotBasedSettings.InitNumberRows = 2;
	Startup.Settings.InventorySlotBasedSettings.InitNumColumns = 3;
	auto* Inventory = UInventoryBase::CreateInventoryAdvanced(Collection, Startup, Collection->GetOwner(), Collection);
	Collection->AddPawnInventory_Internal(Inventory);
	if (auto* Grid = Cast<USlotbasedInventory>(Inventory))
	{
		for (auto* Slot : Grid->GetInventorySlots())
		{
			Slot->InventorySlotInfo.AllowedCategory = FGameplayTag::RequestGameplayTag(TEXT("Item.Category"));
		}
	}
	return Inventory;
}

bool FInvenzaInventoryAdmissionTest::RunTest(const FString& Parameters)
{
	auto* Instance = NewObject<UGameInstance>(GEngine);
	Instance->InitializeStandalone();
	auto* Pawn = Instance->GetWorld()->SpawnActor<APawn>();
	auto* Collection = NewObject<UInventoryCollectionComponent>(Pawn);
	Collection->bPersistInventories = false;
	Pawn->AddInstanceComponent(Collection);
	Collection->RegisterComponent();
	auto* Manager = NewObject<UIInventoryManager>(Pawn);
	Pawn->AddInstanceComponent(Manager);
	Manager->RegisterComponent();
	FindFProperty<FObjectPropertyBase>(UIInventoryManager::StaticClass(), TEXT("ItemCollectionRef"))
		->SetObjectPropertyValue_InContainer(Manager, Collection);
	auto* Source = CreateAdmissionTestInventory(Collection, USlotbasedInventory::StaticClass());
	auto* Target = CastChecked<USlotbasedInventory>(CreateAdmissionTestInventory(Collection, USlotbasedInventory::StaticClass()));
	auto* List = CreateAdmissionTestInventory(Collection, UListInventory::StaticClass());

	const FGameplayTag Root = FGameplayTag::RequestGameplayTag(TEXT("Item.Category"));
	const FGameplayTag Ingredient = FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Ingredient"));
	const FGameplayTag Armor = FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Armor"));
	auto* Table = NewObject<UDataTable>(Instance);
	Table->RowStruct = FItemData::StaticStruct();
	FItemData Log;
	Log.ID = TEXT("AdmissionLog");
	Log.ItemMetaData.ItemCategory = Ingredient;
	FItemData Vine = Log;
	Vine.ID = TEXT("AdmissionVine");
	Table->AddRow(Log.ID, Log);
	Table->AddRow(Vine.ID, Vine);
	FDataTableRowHandle LogRow;
	LogRow.DataTable = Table;
	LogRow.RowName = Log.ID;
	FDataTableRowHandle VineRow = LogRow;
	VineRow.RowName = Vine.ID;
	auto* LogItem = UItemFactory::CreateItemByHandle(Pawn, LogRow, 1);
	auto* VineItem = UItemFactory::CreateItemByHandle(Pawn, VineRow, 1);
	if (!TestNotNull(TEXT("Log fixture"), LogItem) || !TestNotNull(TEXT("Vine fixture"), VineItem))
	{
		Instance->Shutdown();
		return false;
	}

	FInventoryAdmissionRules Rules;
	TestTrue(TEXT("Empty rules allow all categories"), Rules.Allows(Armor));
	Rules.AllowedCategories.AddTag(Root);
	Rules.ExcludedCategories.AddTag(Ingredient);
	TestFalse(TEXT("Child exclusion overrides parent allowance"), Rules.Allows(Ingredient));
	TestTrue(TEXT("Sibling category remains allowed"), Rules.Allows(Armor));
	Rules.AllowedItems.Add(LogRow);
	TestFalse(TEXT("Category exclusion overrides an individual item allowance"), Rules.AllowsItem(Ingredient, LogRow));
	Target->ApplyAdmissionRules(Rules);
	List->ApplyAdmissionRules(Rules);
	FItemMoveData Move;
	Move.SourceItem = LogItem;
	Move.TargetInventory = Target;
	TestEqual(TEXT("Grid rejects inbound item despite allowed cells"), Target->HandleAddItem(Move, true).ActualAmountAdded, 0);
	Move.TargetInventory = List;
	TestEqual(TEXT("List rejects the same item"), List->HandleAddItem(Move, true).ActualAmountAdded, 0);

	Rules = {};
	Rules.AllowedItems.Add(LogRow);
	Target->ApplyAdmissionRules(Rules);
	TestTrue(TEXT("Individual resource is allowed"), Target->CanReceiveItem(LogItem));
	TestFalse(TEXT("Another resource in the same category is rejected"), Target->CanReceiveItem(VineItem));

	FItemMapping Mapping;
	Mapping.InventoryID = Target->GetInventoryContainerID();
	Mapping.OccupiedSlots.Add(Target->GetInventorySlots()[0]);
	Collection->AddItem(VineItem, Mapping);
	TestTrue(TEXT("Existing excluded physical item can be rearranged"), Target->CanReceiveItem(VineItem));
	auto* IncomingVine = UItemFactory::CreateItemByHandle(Pawn, VineRow, 1);
	TestFalse(TEXT("Existing forbidden item does not allow external replenishment"), Target->CanReceiveItem(IncomingVine));
	Move.SourceItem = VineItem;
	Move.SourceInventory = Target;
	Move.TargetInventory = Target;
	Move.SourceSlotID = Target->GetInventorySlots()[0]->InventorySlotInfo.SlotGuid;
	Move.TargetSlotID = Target->GetInventorySlots()[1]->InventorySlotInfo.SlotGuid;
	TestTrue(TEXT("Internal move succeeds"), Target->HandleAddItem(Move, false).OperationResult != EItemAddResult::IAR_NoItemAdded);
	TestTrue(TEXT("Sorting preserves excluded contents"), Target->ReconfigureGrid(Target->GetInventorySize(), true));

	Mapping.InventoryID = Source->GetInventoryContainerID();
	Mapping.OccupiedSlots = {CastChecked<USlotbasedInventory>(Source)->GetInventorySlots()[0]};
	Collection->AddItem(LogItem, Mapping);
	Mapping.OccupiedSlots = {CastChecked<USlotbasedInventory>(Source)->GetInventorySlots()[1]};
	Collection->AddItem(IncomingVine, Mapping);
	Manager->TransferAllowedItems(Source, Target);
	TestEqual(TEXT("Bulk move accepts a previously absent allowed item type"), Collection->GetAllItemsByContainer(Target->GetInventoryContainerID()).Num(), 2);
	TestEqual(TEXT("Bulk move leaves forbidden items at source"), Collection->GetAllItemsByContainer(Source->GetInventoryContainerID()).Num(), 1);

	auto Settings = Target->GetInventorySettings();
	Settings.bAllowAdmissionRulesEditing = false;
	Target->SetInventorySettings(Settings);
	Manager->Server_SetAdmissionRules(Target, FInventoryAdmissionRules(), FGuid::NewGuid());
	TestTrue(TEXT("Read-only configuration rejects server edit"), Target->GetAdmissionRules() == Rules);
	Settings.bAllowAdmissionRulesEditing = true;
	Target->SetInventorySettings(Settings);
	FInventoryAdmissionRules InvalidRules;
	InvalidRules.ExcludedCategories.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Equipment.Slot.Weapon")));
	Manager->Server_SetAdmissionRules(Target, InvalidRules, FGuid::NewGuid());
	TestTrue(TEXT("Non-category tags are rejected"), Target->GetAdmissionRules() == Rules);
	Manager->Server_SetAdmissionRules(Target, FInventoryAdmissionRules(), FGuid::NewGuid());
	TestTrue(TEXT("Accessible editable inventory accepts a valid server edit"), Target->GetAdmissionRules() == FInventoryAdmissionRules());
	Target->ApplyAdmissionRules(Rules);

	auto* StackTarget = CastChecked<USlotbasedInventory>(CreateAdmissionTestInventory(Collection, USlotbasedInventory::StaticClass()));
	FItemData StackRow = Vine;
	StackRow.ID = TEXT("AdmissionStack");
	StackRow.ItemMetaData.ItemNumeraticData.MaxStackSizeInCharacter = 10;
	Table->AddRow(StackRow.ID, StackRow);
	FDataTableRowHandle StackHandle = VineRow;
	StackHandle.RowName = StackRow.ID;
	Move = {};
	Move.SourceItem = UItemFactory::CreateItemByHandle(Pawn, StackHandle, 4);
	Move.TargetInventory = StackTarget;
	TestEqual(TEXT("Seed a stack before excluding it"), StackTarget->HandleAddItem(Move, false).ActualAmountAdded, 4);
	FInventoryAdmissionRules ExcludeIngredients;
	ExcludeIngredients.ExcludedCategories.AddTag(Ingredient);
	StackTarget->ApplyAdmissionRules(ExcludeIngredients);
	Move.SourceItem = UItemFactory::CreateItemByHandle(Pawn, StackHandle, 2);
	Move.TargetSlotID = StackTarget->GetInventorySlots()[0]->InventorySlotInfo.SlotGuid;
	TestEqual(TEXT("Incoming quantity cannot replenish an excluded stack"), StackTarget->HandleAddItem(Move, false).ActualAmountAdded, 0);
	UObject* ExistingStack = Collection->GetAllItemsByContainer(StackTarget->GetInventoryContainerID())[0];
	const FGuid SplitSlot = StackTarget->GetInventorySlots()[1]->InventorySlotInfo.SlotGuid;
	TestEqual(TEXT("Excluded existing stack can be split internally"), StackTarget->SplitExistingStack(
		ExistingStack, 2, SplitSlot, EItemOrientationType::Horizontal).ActualAmountAdded, 2);
	TestEqual(TEXT("Internal split retains the original remainder"), IObjectDataProvider::Execute_GetQuantity(ExistingStack), 2);
	StackTarget->GetInventorySlots()[2]->InventorySlotInfo.AllowedCategory = Armor;
	TestEqual(TEXT("Failed internal split does not bypass cell requirements"), StackTarget->SplitExistingStack(
		ExistingStack, 1, StackTarget->GetInventorySlots()[2]->InventorySlotInfo.SlotGuid,
		EItemOrientationType::Horizontal).ActualAmountAdded, 0);
	TestEqual(TEXT("Failed split restores the source amount"), IObjectDataProvider::Execute_GetQuantity(ExistingStack), 2);

	FInventorySavedOwner Saved;
	FString Error;
	TestTrue(TEXT("Capture includes admission state"), Collection->CapturePersistence(Saved, Error));
	Target->ApplyAdmissionRules(FInventoryAdmissionRules());
	FInventoryPreparedOwner Plan;
	if (TestTrue(TEXT("Prepare accepts existing excluded items"), Collection->PreparePersistence(Saved, Plan, Error)))
	{
		Collection->CommitPersistence(Plan);
		TestTrue(TEXT("Save restore retains per-item rules"), Target->GetAdmissionRules() == Rules);
		TestEqual(TEXT("Save restore retains forbidden contents"), Collection->GetAllItemsByContainer(Target->GetInventoryContainerID()).Num(), 2);
	}
	auto* Controller = Instance->GetWorld()->SpawnActor<APlayerController>();
	auto* LocalPlayer = NewObject<ULocalPlayer>(GEngine);
	LocalPlayer->PlayerController = Controller;
	Controller->Player = LocalPlayer;
	Instance->GetWorld()->AddController(Controller);
	Controller->Possess(Pawn);
	TestTrue(TEXT("UI fixture possesses the inventory owner"), Controller->GetPawn() == Pawn);
	TestTrue(TEXT("UI fixture inventory remains editable"), Manager->CanEditAdmissionRules(Target));
	const auto WindowClass = LoadClass<UInventoryAdmissionWidget>(nullptr,
		TEXT("/InventorySystemInvenzaPlugin/UI/Inventory/Admission/WBP_InventoryAdmission.WBP_InventoryAdmission_C"));
	if (TestNotNull(TEXT("Authored admission window"), WindowClass))
	{
		auto* Window = CreateWidget<UInventoryAdmissionWidget>(Controller, WindowClass);
		if (!TestNotNull(TEXT("Admission window instance"), Window))
		{
			Instance->Shutdown();
			return false;
		}
		TSharedPtr<SWidget> WindowSlate = Window->TakeWidget();
		Window->SetInventory(Target);
		auto* Tree = Cast<UTreeView>(Window->GetWidgetFromName(TEXT("CategoryTree")));
		auto* Search = Cast<UEditableTextBox>(Window->GetWidgetFromName(TEXT("SearchInput")));
		auto* Apply = Cast<UUIButton>(Window->GetWidgetFromName(TEXT("ApplyButton")));
		TestTrue(TEXT("Window resolves the interacting player's manager"), Window->CanEdit());
		TestEqual(TEXT("Tree exposes one category root"), Tree->GetListItems().Num(), 1);
		auto* RootNode = CastChecked<UInventoryCategoryTreeEntry>(Tree->GetListItems()[0]);
		UInventoryCategoryTreeEntry* IngredientNode = nullptr;
		for (UInventoryCategoryTreeEntry* Node : RootNode->Children)
		{
			if (Node->Category == Ingredient)
			{
				IngredientNode = Node;
			}
		}
		if (!TestNotNull(TEXT("Catalog ingredient branch"), IngredientNode) ||
			!TestTrue(TEXT("Catalog has multiple resources"), IngredientNode->Children.Num() > 1))
		{
			WindowSlate.Reset();
			Instance->Shutdown();
			return false;
		}
		auto* FirstLeaf = IngredientNode->Children[0].Get();
		auto* OtherLeaf = IngredientNode->Children[1].Get();
		Window->SetNodeAllowed(RootNode, false);
		TestEqual(TEXT("Blocking the root blocks its entire subtree"), Window->GetNodeSelection(RootNode), EInventoryAdmissionSelection::Blocked);
		TestFalse(TEXT("Blocking everything does not accidentally mean an empty unrestricted list"),
			Window->GetDraftRules().AllowsItem(FirstLeaf->Category, FirstLeaf->ItemDefinition));
		Window->ToggleNode(FirstLeaf);
		TestEqual(TEXT("One selected resource makes its category partial"), Window->GetNodeSelection(IngredientNode), EInventoryAdmissionSelection::Partial);
		TestEqual(TEXT("Partial selection propagates to the root"), Window->GetNodeSelection(RootNode), EInventoryAdmissionSelection::Partial);
		TestTrue(TEXT("Selecting a child of a blocked branch permits that resource"),
			Window->GetDraftRules().AllowsItem(FirstLeaf->Category, FirstLeaf->ItemDefinition));
		TestFalse(TEXT("Selecting a resource leaves its sibling blocked"),
			Window->GetDraftRules().AllowsItem(OtherLeaf->Category, OtherLeaf->ItemDefinition));
		Search->SetText(FirstLeaf->Label);
		Search->OnTextChanged.Broadcast(Search->GetText());
		TestEqual(TEXT("Search does not turn a partial category into a fully allowed category"),
			Window->GetNodeSelection(IngredientNode), EInventoryAdmissionSelection::Partial);
		Search->SetText(FText::GetEmpty());
		Search->OnTextChanged.Broadcast(Search->GetText());
		Window->ToggleNode(IngredientNode);
		TestEqual(TEXT("Clicking partial allows the entire category"), Window->GetNodeSelection(IngredientNode), EInventoryAdmissionSelection::Allowed);
		TestTrue(TEXT("Category selection permits the previously blocked resource"),
			Window->GetDraftRules().AllowsItem(OtherLeaf->Category, OtherLeaf->ItemDefinition));
		Window->ToggleNode(IngredientNode);
		TestEqual(TEXT("Clicking allowed blocks the entire category"), Window->GetNodeSelection(IngredientNode), EInventoryAdmissionSelection::Blocked);
		Window->ToggleNode(RootNode);
		TestTrue(TEXT("Allowing everything restores unrestricted empty rules"), Window->GetDraftRules() == FInventoryAdmissionRules());
		Window->ToggleNode(FirstLeaf);
		TestEqual(TEXT("A blocked resource makes an allowed category partial"), Window->GetNodeSelection(IngredientNode), EInventoryAdmissionSelection::Partial);
		Window->ToggleNode(RootNode);
		TestTrue(TEXT("Draft changes enable Apply"), Apply->GetIsEnabled());
		TestTrue(TEXT("Draft editing leaves inventory unchanged"), Target->GetAdmissionRules() == Rules);
		Apply->OnButtonClicked.Broadcast(Apply);
		TestTrue(TEXT("Apply commits the tree selection"), Target->GetAdmissionRules() == FInventoryAdmissionRules());
		Window->SetInventory(Target);
		Settings.bAllowAdmissionRulesEditing = false;
		Target->SetInventorySettings(Settings);
		TestFalse(TEXT("Viewer disallows edits when configured read-only"), Window->CanEdit());
		const FInventoryAdmissionRules BeforeReadOnlyEdit = Window->GetDraftRules();
		Window->ToggleNode(CastChecked<UInventoryCategoryTreeEntry>(Tree->GetListItems()[0]));
		TestTrue(TEXT("Read-only viewer does not modify the draft"), Window->GetDraftRules() == BeforeReadOnlyEdit);
		Search->SetText(FText::FromString(TEXT("CategoryDoesNotExist")));
		Search->OnTextChanged.Broadcast(Search->GetText());
		TestTrue(TEXT("Read-only search filters the tree"), Tree->GetListItems().IsEmpty());
		Search->SetText(FText::GetEmpty());
		Search->OnTextChanged.Broadcast(Search->GetText());
		TestEqual(TEXT("Clearing read-only search restores the root"), Tree->GetListItems().Num(), 1);
		Window->ReleaseInput();
		Window->RemoveFromParent();
		WindowSlate.Reset();
	}
	Instance->Shutdown();
	return true;
}
#endif
