// Nublin Studio 2026 All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Components/SpinBox.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Inventory/ListInventory/ListInventory.h"
#include "Data/Inventory/ListInventory/InventoryListEntry.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/Items/itemBase.h"
#include "Data/ItemData.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Interface/UI/ModalInterface.h"
#include "Interface/UIInterface.h"
#include "UI/Core/Modal/InventoryTakeAmountHeaderWidget.h"
#include "UObject/UnrealType.h"
#include "Widgets/SWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvenzaTakeAmountTest, "Invenza.Inventory.TakeAmount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

static UInventoryCollectionComponent* CreateAmountTestCollection(AActor* Owner)
{
	UInventoryCollectionComponent* Collection = NewObject<UInventoryCollectionComponent>(Owner);
	Collection->bPersistInventories = false;
	Owner->AddInstanceComponent(Collection);
	Collection->RegisterComponent();
	return Collection;
}

static UInventoryBase* CreateAmountTestInventory(UInventoryCollectionComponent* Collection, UClass* Class, bool bReference = false)
{
	FInventoryStartupData Startup;
	Startup.Settings.InventoryClass = Class;
	Startup.Settings.EntryClass = UInventoryListEntry::StaticClass();
	Startup.Settings.bIsReferenceContainer = bReference;
	Startup.Settings.bCollectInvDataFromWidget = false;
	Startup.Settings.InventorySlotBasedSettings.bIgnoreItemSize = true;
	Startup.Settings.InventorySlotBasedSettings.InitNumberRows = 1;
	Startup.Settings.InventorySlotBasedSettings.InitNumColumns = 1;
	UInventoryBase* Inventory = UInventoryBase::CreateInventoryAdvanced(Collection, Startup, Collection->GetOwner(), Collection);
	Collection->AddPawnInventory_Internal(Inventory);
	if (USlotbasedInventory* Grid = Cast<USlotbasedInventory>(Inventory))
	{
		for (UInventorySlotData* Slot : Grid->GetInventorySlots())
		{
			Slot->InventorySlotInfo.AllowedCategory = FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Ingredient"));
		}
	}
	return Inventory;
}

static void PlaceAmountTestItem(UInventoryCollectionComponent* Collection, UInventoryBase* Inventory, UItemBase* Item)
{
	FItemMapping Mapping;
	Mapping.InventoryID = Inventory->GetInventoryContainerID();
	Mapping.bIsReferenceContainer = Inventory->GetInventorySettings().bIsReferenceContainer;
	USlotbasedInventory* Grid = Cast<USlotbasedInventory>(Inventory);
	Mapping.OccupiedSlots.Add(Grid ? Grid->GetInventorySlots()[0] : NewObject<UInventorySlotData>(Inventory));
	Collection->AddItem(Item, Mapping);
}

bool FInvenzaTakeAmountTest::RunTest(const FString& Parameters)
{
	UGameInstance* Instance = NewObject<UGameInstance>(GEngine);
	Instance->InitializeStandalone();
	UWorld* World = Instance->GetWorld();
	APawn* Player = World->SpawnActor<APawn>();
	APawn* OtherPlayer = World->SpawnActor<APawn>();
	AActor* StorageOwner = World->SpawnActor<AActor>();
	UInventoryCollectionComponent* PlayerCollection = CreateAmountTestCollection(Player);
	UInventoryCollectionComponent* StorageCollection = CreateAmountTestCollection(StorageOwner);
	UIInventoryManager* Manager = NewObject<UIInventoryManager>(Player);
	Player->AddInstanceComponent(Manager);
	Manager->RegisterComponent();
	UInteractableComponent* Interaction = NewObject<UInteractableComponent>(StorageOwner);
	StorageOwner->AddInstanceComponent(Interaction);
	Interaction->RegisterComponent();
	Interaction->TryAcquireInteraction(Player);
	USlotbasedInventory* Bag = CastChecked<USlotbasedInventory>(CreateAmountTestInventory(PlayerCollection, USlotbasedInventory::StaticClass()));
	USlotbasedInventory* Storage = CastChecked<USlotbasedInventory>(CreateAmountTestInventory(StorageCollection, USlotbasedInventory::StaticClass()));
	UInventoryBase* Shortcut = CreateAmountTestInventory(StorageCollection, UListInventory::StaticClass(), true);
	UInventoryBase* OtherBag = CreateAmountTestInventory(PlayerCollection, UListInventory::StaticClass());
	PlayerCollection->SetExternalInventory(Storage);
	auto SetManagerObject = [&](const TCHAR* Name, UObject* Value)
	{
		FindFProperty<FObjectPropertyBase>(UIInventoryManager::StaticClass(), Name)->SetObjectPropertyValue_InContainer(Manager, Value);
	};
	SetManagerObject(TEXT("ItemCollectionRef"), PlayerCollection);
	SetManagerObject(TEXT("MainPawnInventoryRef"), Bag);
	SetManagerObject(TEXT("TrackedInteractionTarget"), Interaction);
	FItemData Definition;
	Definition.ItemMetaData.ItemCategory = FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Ingredient"));
	Definition.ItemMetaData.ItemNumeraticData.MaxStackSizeInCharacter = 100;
	Definition.ItemMetaData.ItemNumeraticData.Weight = 1.f;
	UItemBase* Source = NewObject<UItemBase>(StorageCollection);
	Source->InitItem(TEXT("TakeAmountTest"), Definition, 20);
	PlaceAmountTestItem(StorageCollection, Storage, Source);
	PlaceAmountTestItem(StorageCollection, Shortcut, Source);
	auto Request = [&](int32 Amount)
	{
		FInventoryTakeAmountRequest Data;
		Data.RequestID = FGuid::NewGuid();
		Data.Move.SourceInventory = Storage;
		Data.Move.SourceItem = Source;
		Data.Move.SourceSlotID = Storage->GetInventorySlots()[0]->InventorySlotInfo.SlotGuid;
		Data.Move.TargetInventory = OtherBag; // Untrusted caller target must be ignored.
		Data.Move.RequestedAmount = Amount;
		Data.ExpectedSourceQuantity = Source->GetQuantity_Implementation();
		Data.ExpectedAvailableAmount = Source->GetFreeAmount_Implementation();
		return Data;
	};
	const FGuid OriginalID = Source->GetPersistentInstanceID();
	TestEqual(TEXT("Full storage still permits a partial withdrawal"), Manager->HandleTakeAmount(Request(6)), EInventoryTakeAmountResult::Success);
	TestEqual(TEXT("Source keeps the remainder"), Source->GetQuantity_Implementation(), 14);
	TArray<UObject*> BagItems = PlayerCollection->GetAllItemsByContainer(Bag->GetInventoryContainerID());
	if (!TestEqual(TEXT("One physical receiving stack"), BagItems.Num(), 1))
	{
		return false;
	}
	UItemBase* Received = CastChecked<UItemBase>(BagItems[0]);
	TestEqual(TEXT("Exactly the selected amount arrives"), Received->GetQuantity_Implementation(), 6);
	TestTrue(TEXT("Partial transfer retains source shortcut"), StorageCollection->ItemHasInventory(Source, Shortcut->GetInventoryContainerID()));
	TestNotEqual(TEXT("Partial transfer receives its own persistent identity"), Received->GetPersistentInstanceID(), OriginalID);
	TestTrue(TEXT("Caller-supplied target was ignored"), PlayerCollection->GetAllItemsByContainer(OtherBag->GetInventoryContainerID()).IsEmpty());
	TestTrue(TEXT("Received object belongs to the recipient actor"), Received->GetOuter() == Player);
	TestEqual(TEXT("Second withdrawal merges into a full bag"), Manager->HandleTakeAmount(Request(3)), EInventoryTakeAmountResult::Success);
	TestEqual(TEXT("Merged quantity"), Received->GetQuantity_Implementation(), 9);

	Source->ReserveAmount_Implementation(StorageOwner, 5);
	TestEqual(TEXT("Reservations reduce availability"), Source->GetFreeAmount_Implementation(), 6);
	TestEqual(TEXT("Reserved units cannot be taken"), Manager->HandleTakeAmount(Request(7)), EInventoryTakeAmountResult::AmountReserved);
	Source->ReleaseReservation_Implementation(StorageOwner, 2);
	TestEqual(TEXT("Partial reservation release retains the rest"), Source->GetFreeAmount_Implementation(), 8);
	FInventoryTakeAmountRequest Stale = Request(2);
	Source->SetQuantityForTransfer(10);
	TestEqual(TEXT("Stale quantity is rejected"), Manager->HandleTakeAmount(Stale), EInventoryTakeAmountResult::SourceChanged);
	TestEqual(TEXT("Rejected request did not change target"), Received->GetQuantity_Implementation(), 9);
	Source->SetQuantityForTransfer(11);
	Source->ReleaseReservation_Implementation(StorageOwner, 100);
	FInventorySettings Settings = Bag->GetInventorySettings();
	Settings.InventoryMaxWeightCapacity = 10.f;
	Bag->SetInventorySettings(Settings);
	TestEqual(TEXT("Insufficient weight rejects the complete request"), Manager->HandleTakeAmount(Request(2)), EInventoryTakeAmountResult::NoCapacity);
	TestEqual(TEXT("Weight rejection leaves source unchanged"), Source->GetQuantity_Implementation(), 11);
	TestEqual(TEXT("Weight rejection leaves target unchanged"), Received->GetQuantity_Implementation(), 9);
	Settings.InventoryMaxWeightCapacity = -1.f;
	Bag->SetInventorySettings(Settings);
	Received->SetQuantityForTransfer(100);
	TestEqual(TEXT("No room rejects the complete request"), Manager->HandleTakeAmount(Request(2)), EInventoryTakeAmountResult::NoCapacity);
	TestEqual(TEXT("No-room rejection leaves source unchanged"), Source->GetQuantity_Implementation(), 11);
	Received->SetQuantityForTransfer(9);
	Interaction->ReleaseInteraction(Player);
	Interaction->TryAcquireInteraction(OtherPlayer);
	TestEqual(TEXT("Another player's lock denies access"), Manager->HandleTakeAmount(Request(1)), EInventoryTakeAmountResult::AccessDenied);
	Interaction->ReleaseInteraction(OtherPlayer);
	Interaction->TryAcquireInteraction(Player);
	FInventoryTakeAmountRequest BadSource = Request(1);
	BadSource.Move.SourceInventory = Shortcut;
	TestEqual(TEXT("Reference source is not physical storage"), Manager->HandleTakeAmount(BadSource), EInventoryTakeAmountResult::AccessDenied);
	FInventoryTakeAmountRequest InvalidAmount = Request(0);
	TestEqual(TEXT("Zero amount is rejected"), Manager->HandleTakeAmount(InvalidAmount), EInventoryTakeAmountResult::InvalidRequest);
	TestEqual(TEXT("Whole remainder can be taken"), Manager->HandleTakeAmount(Request(11)), EInventoryTakeAmountResult::Success);
	TestTrue(TEXT("Full withdrawal removes physical source"), StorageCollection->GetAllItemsByContainer(Storage->GetInventoryContainerID()).IsEmpty());
	TestTrue(TEXT("Full withdrawal removes shortcuts"), StorageCollection->GetAllItemsByContainer(Shortcut->GetInventoryContainerID()).IsEmpty());
	TestEqual(TEXT("Total quantity is conserved"), Received->GetQuantity_Implementation(), 20);

	UClass* HeaderClass = LoadClass<UInventoryTakeAmountHeaderWidget>(nullptr,
		TEXT("/InventorySystemInvenzaPlugin/UI/Core/Modals/ModalHeaderParts/WBP_TakeAmountHeader.WBP_TakeAmountHeader_C"));
	if (TestNotNull(TEXT("Authored amount window"), HeaderClass))
	{
		UInventoryTakeAmountHeaderWidget* Header = CreateWidget<UInventoryTakeAmountHeaderWidget>(World, HeaderClass);
		FModalHeaderData HeaderData;
		HeaderData.SourceInventory = Bag;
		HeaderData.ContextItem = Received;
		IModalInterface::Execute_ConfigureHeader(Header, HeaderData);
		TSharedPtr<SWidget> HeaderSlate = Header->TakeWidget();
		USpinBox* Input = Cast<USpinBox>(Header->GetWidgetFromName(TEXT("AmountInput")));
		if (TestNotNull(TEXT("Quantity input"), Input))
		{
			Input->SetValue(7.f);
			FModalHeaderResult Result = IModalInterface::Execute_GetHeaderResult(Header);
			TestEqual(TEXT("Modal returns the selected integer amount"), Result.SelectedQuantity, 7);
			TestEqual(TEXT("Modal records its source snapshot"), Result.ExpectedSourceQuantity, 20);
			Header->SetServerFeedback(FText::GetEmpty(), 4, 3);
			Result = IModalInterface::Execute_GetHeaderResult(Header);
			TestEqual(TEXT("Stale reply clamps to current availability"), Result.SelectedQuantity, 3);
		}
	}
	UInvenzaInventorySettingsAsset* SettingsAsset = LoadObject<UInvenzaInventorySettingsAsset>(nullptr,
		TEXT("/InventorySystemInvenzaPlugin/Data/Settings/InvenzaSettings.InvenzaSettings"));
	UClass* FooterClass = SettingsAsset ? SettingsAsset->ModalFooterWidgets.FindRef(EModalFooterType::Selection).Get() : nullptr;
	if (TestNotNull(TEXT("Existing selection footer"), FooterClass) && SettingsAsset)
	{
		UUserWidget* Footer = CreateWidget<UUserWidget>(World, FooterClass);
		TSharedPtr<SWidget> FooterSlate = Footer->TakeWidget();
		TArray<EObjectInteractionType> Actions = {EObjectInteractionType::TakeAmount, EObjectInteractionType::Split, EObjectInteractionType::Drop};
		TArray<FModalAction> Display;
		for (EObjectInteractionType Action : Actions)
		{
			Display.Add(SettingsAsset->ModalActions.FindRef(Action));
		}
		IModalInterface::Execute_ConfigureButtons(Footer, Actions, Display);
		TestEqual(TEXT("Text take action and existing composed buttons are all bound"), IUIInterface::Execute_GetButtons(Footer).Num(), 3);
	}
	Interaction->ReleaseInteraction(Player);
	Player->Destroy();
	OtherPlayer->Destroy();
	StorageOwner->Destroy();
	Instance->Shutdown();
	return true;
}
#endif
