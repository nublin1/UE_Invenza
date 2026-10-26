//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/UIInventoryManager.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "Engine/GameInstance.h"
#include "Data/Inventory/Equipment/EquipmentTransferTypes.h"
#include "ActorComponents/Interactable/CharacterInventoryAccessComponent.h"
#include "ActorComponents/Interactable/StorageAccessComponent.h"

#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/Crafting/CraftingComponent.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/Equipment/EquipmentComponent.h"
#include "Data/Inventory/ListInventory/ListInventory.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "Factory/InvenzaWidgetFactory.h"
#include "Interface/Interaction/InteractionUIProvider.h"
#include "Interface/Interaction/LootContainerProvider.h"
#include "Interface/Interaction/VendorProvider.h"
#include "Interface/Inventory/InvUIProvider.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"
#include "UI/Interaction/InteractionWidget.h"
#include "UI/Inventory/InventorySlot.h"
#include "UI/Inventory/ListInventoryWidget.h"
#include "Data/Trade/TradeTypes.h"
#include "Net/UnrealNetwork.h"
#include "UI/Core/Zones/WorldDropZoneWidget.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Interface/Interaction/CraftProvider.h"
#include "Subsystems/ModalWindowManager.h"
#include "UI/Craft/CraftDashboard.h"
#include "UI/Craft/CraftMenuChoose.h"
#include "Utility/InputUtility.h"
#include "Utility/InterfaceUtils.h"
#include "Utility/InvenzayUtility.h"

class UEnhancedInputLocalPlayerSubsystem;

UIInventoryManager::UIInventoryManager()
{
	SetIsReplicatedByDefault(true);
}

void UIInventoryManager::BeginPlay()
{
	bEndingPlay = false;
	Super::BeginPlay();

	OwnerPawnRef = Cast<APawn>(GetOwner());

	GlobalSettings = UInvenzayUtility::GetInvenzaGlobalSettings(GetWorld());

	GetWorld()->GetTimerManager().SetTimer(InventoryInitTimer, this, &UIInventoryManager::InitializeInventoryManager,
										   0.1f, false);
}

void UIInventoryManager::TickComponent(float DeltaTime, ELevelTick TickType,
									   FActorComponentTickFunction* ThisTickFunction)
{
}

void UIInventoryManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UIInventoryManager, TradeSessionItems, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UIInventoryManager, StorageSession, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UIInventoryManager, CharacterInventoryAccess, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UIInventoryManager, CharacterInventoryRevision, COND_OwnerOnly);
	DOREPLIFETIME(UIInventoryManager, InventoryWidgetInitMap);
	DOREPLIFETIME(UIInventoryManager, MainPawnInventoryRef);
	DOREPLIFETIME(UIInventoryManager, SessionClientTrust);
	DOREPLIFETIME(UIInventoryManager, VendorProviderCurrent);
	DOREPLIFETIME(UIInventoryManager, LootContainerProvider);

	DOREPLIFETIME_CONDITION(UIInventoryManager, CraftProvider, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UIInventoryManager, ActiveCraftComponentRef, COND_OwnerOnly);
}

void UIInventoryManager::InitializeInventoryManager()
{
	auto Collection = GetOwner()->FindComponentByClass<UInventoryCollectionComponent>();
	if (!Collection)
	{
		return;
	}

	ItemCollectionRef = Collection;
	Collection->OnActorInventoriesChanged.AddUniqueDynamic(this, &ThisClass::QueueInventoryUIRefresh);

	if (auto EquipmentComponent = GetOwner()->FindComponentByClass<UEquipmentComponent>())
	{
		EquipmentComponentRef = EquipmentComponent;
	}

	if (auto CraftingComponent = GetOwner()->FindComponentByClass<UCraftingComponent>())
	{
		PawnCraftingComponentRef = CraftingComponent;
	}

	if (UInteractionComponent* InteractionComp = GetOwner()->FindComponentByClass<UInteractionComponent>())
	{
		InteractionComponent = InteractionComp;
	}

	CreateInventories();

	if (OwnerPawnRef && OwnerPawnRef->IsLocallyControlled())
	{
		InitInvWidgets();
		InitCraftWidgets();
		BindWorldDropZoneEvents();
		CreateWidgetsForInventories();
		InitializeBindings();
		BindEvents();
	}

	SetupStartingResources();

	SetupAdditionalComponents();

	OnInitializationCompleteDelegate.Broadcast();
	if (GetOwner()->HasAuthority())
	{
		auto* Save = GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>();
		SessionClientTrust = Save->GetClientTrust();
		Save->NotifyPlayerReady();
	}
	OnRep_StorageSession();
}

void UIInventoryManager::CreateInventories()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	for (FInventoryStartupData& StartupData : StartupInventories)
	{
		UInventoryBase* Inventory =
			UInvenzayUtility::CreateStartupInventory(this, ItemCollectionRef, StartupData, StartingItems);

		if (!Inventory)
		{
			continue;
		}

		BindInventoryEvents(Inventory);

		if (StartupData.Settings.InventoryTag == GlobalSettings->MainInvTag)
		{
			MainPawnInventoryRef = Inventory;
		}

		if (!StartupData.Settings.bCollectInvDataFromWidget)
		{
			InventoryWidgetInitMap.Add(Inventory);
		}
	}
}

void UIInventoryManager::CreateWidgetsForInventories()
{
	if (InventoryWidgetInitMap.IsEmpty())
	{
		return;
	}

	for (auto& Inventory : InventoryWidgetInitMap)
	{
		if (ItemCollectionRef->HasContainerWidget(Inventory))
		{
			UE_LOG(LogTemp, Warning, TEXT("Widget for inventory %s already exists"), *GetNameSafe(Inventory));
			continue;
		}

		auto InventoryWidget = CreateInventoryWidget(Inventory);
		if (!InventoryWidget)
		{
			continue;
		}

		UIInvProvider->AddPawnInvContainerWidget(InventoryWidget);
	}
}

UInventoryContainerWidget* UIInventoryManager::CreateInventoryWidget(UInventoryBase* InvToLink)
{
	if (bEndingPlay || !GetWorld() || GetWorld()->bIsTearingDown)
	{
		return nullptr;
	}
	if (!InvToLink || !OwnerPawnRef || !OwnerPawnRef->IsLocallyControlled())
	{
		return nullptr;
	}
	if (!UIInvProvider)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIInventoryManager::InitInvWidgets: UIProvider is not set!"));
		return nullptr;
	}

	auto InvSettings = InvToLink->GetInventorySettings();

	APlayerController* PC = Cast<APlayerController>(OwnerPawnRef->GetController());
	if (!PC)
	{
		return nullptr;
	}

	auto InvContainer = UInvenzaWidgetFactory::CreateConteinerInventoryWidget(
		PC, InvSettings.ContainerWidgetClass, InvSettings.InventoryWidgetClass, InvSettings.OperationPanelWidgetClass);

	if (!InvContainer)
	{
		return nullptr;
	}

	auto InvWidget = InvContainer->GetInventoryWidgetFromContainerSlot();
	InvWidget->SetInventoryBaseRef(InvToLink);
	InvWidget->SetUISettings(UISettings);
	InvWidget->InitializeInventoryWidgetWithSettings();

	InvContainer->InitializeInventoryContainer();

	InvWidget->OnItemDroppedDelegate.AddUniqueDynamic(this, &UIInventoryManager::ItemTransferRequest);

	ItemCollectionRef->RegisterContainerWidget(InvToLink, InvContainer);

	return InvContainer;
}

void UIInventoryManager::InitInvWidgets()
{
	if (!UIInvProvider)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIInventoryManager::InitInvWidgets: UIProvider is not set!"));
		return;
	}

	auto AllPawnContInvs = UIInvProvider->GetAllPawnInvContainers();
	if (AllPawnContInvs.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("UIInventoryManager::InitInvWidgets: PawnInvs is empty!"));
		return;
	}

	for (auto ContInvs : AllPawnContInvs)
	{
		if (!ContInvs || !ContInvs->GetInventoryWidgetFromContainerSlot())
		{
			continue;
		}

		auto InvWidget = ContInvs->GetInventoryWidgetFromContainerSlot();

		if (!InvWidget->TargetInventoryTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("UIInventoryManager::InitInvWidgets: TargetInventoryTag is not set!"));
			continue;
		};

		auto TarInv = ItemCollectionRef->GetInventoryByTag(InvWidget->TargetInventoryTag);
		if (!TarInv)
		{
			continue;
		}

		if (InvWidget->GetInventoryRef() == TarInv && ItemCollectionRef->HasContainerWidget(TarInv))
		{
			continue;
		}
		if (USlotbasedInventoryWidget* SlotBased = Cast<USlotbasedInventoryWidget>(InvWidget))
		{
			SlotBased->SetInventoryBaseRef(TarInv);
			SlotBased->SetUISettings(UISettings);
			InvWidget->InitializeInventoryWidget();

			if (auto* Grid = Cast<USlotbasedInventory>(TarInv); Grid && Grid->GetInventorySlots().IsEmpty())
			{
				ItemCollectionRef->Server_SetSlotBasedInventoryWidgetInitData(
					TarInv->GetInventoryContainerID(), SlotBased->CollectInitSlotsDataFromWidget());
				Execute_RebuildInventoryRequest(this, TarInv->GetInventoryContainerID());
			}
			else
			{
				InvWidget->InitializeInventoryWidgetWithSettings();
			}
		}
		else if (UListInventoryWidget* ListBased = Cast<UListInventoryWidget>(InvWidget))
		{
			InvWidget->InitializeInventoryWidget();

			auto TarListInv = Cast<UListInventory>(TarInv);
			if (!TarListInv)
			{
				continue;
			}

			ListBased->SetInventoryBaseRef(TarListInv);
			ListBased->SetUISettings(UISettings);
			ListBased->BindDelegated();
		}

		InvWidget->OnItemDroppedDelegate.AddUniqueDynamic(this, &UIInventoryManager::ItemTransferRequest);

		ItemCollectionRef->RegisterContainerWidget(TarInv, ContInvs);
	}

	for (auto ContainerBase : UIInvProvider->GetAllPawnInvContainers())
	{
		ContainerBase->InitializeInventoryContainer();
	}
}

void UIInventoryManager::InitCraftWidgets()
{
	if (!PawnCraftingComponentRef || !UIInvProvider)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPawnRef->GetController());
	if (!PC)
	{
		return;
	}

	if (UISettings.bCreateCraftWidgetsDynamically)
	{
		auto DashboardWidget = UInvenzaWidgetFactory::CreateInvenzaWidget(PC, UISettings.CraftDashboardClass);
		if (!DashboardWidget)
		{
			return;
		}

		UIInvProvider->AddPawnCraftDashboardWidget(DashboardWidget);

		auto CraftChooseWidget = UInvenzaWidgetFactory::CreateInvenzaWidget(PC, UISettings.CraftMenuChooseClass);
		if (!CraftChooseWidget)
		{
			return;
		}

		UIInvProvider->AddPawnCraftDashboardWidget(CraftChooseWidget);
	}

	UIInvProvider->BindCraftWidgets();
}

void UIInventoryManager::SetupStartingResources()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	UInvenzayUtility::SetupStartingResources(this, StartingItems);
}

void UIInventoryManager::SetupAdditionalComponents()
{
	if (!PawnCraftingComponentRef || !UIInvProvider)
	{
		return;
	}

	auto FindInventory = [this](const FGameplayTag& OverrideTag, const FGameplayTag& DefaultTag) -> UInventoryBase*
	{
		if (OverrideTag.IsValid())
		{
			if (UInventoryBase* Inventory = ItemCollectionRef->GetInventoryByTag(OverrideTag))
			{
				return Inventory;
			}
		}
		if (DefaultTag.IsValid())
		{
			if (UInventoryBase* Inventory = ItemCollectionRef->GetInventoryByTag(DefaultTag))
			{
				return Inventory;
			}
		}
		return nullptr;
	};

	const FCraftingInventoryOverrides& Overrides = PawnCraftingComponentRef->GetConfig();

	UInventoryBase* InputInventory = FindInventory(Overrides.InputInventoryTag, GlobalSettings->InputInvTagByDefault);
	UInventoryBase* OutputInventory =
		FindInventory(Overrides.OutputInventoryTag, GlobalSettings->OutputInvTagByDefault);
	UInventoryBase* FuelInventory = FindInventory(Overrides.FuelInventoryTag, GlobalSettings->FuelInvTagByDefault);

	if (IsValid(InputInventory))
	{
		PawnCraftingComponentRef->SetInputInventory(InputInventory);
	}
	if (IsValid(OutputInventory))
	{
		PawnCraftingComponentRef->SetOutputInventory(OutputInventory);
	}
	if (IsValid(FuelInventory))
	{
		PawnCraftingComponentRef->SetFuelInventory(FuelInventory);
	}

	PawnCraftingComponentRef->RequestInitCraftingComponent();
}

void UIInventoryManager::ItemContextMenuRequest_Implementation(const FString& FromInventory, FGuid SlotGuid,
															   UObject* Item)
{
	if (!Item || FromInventory.IsEmpty() || !SlotGuid.IsValid())
	{
		return;
	}

	PendingContextItem = Item;
	PendingContextInv = ItemCollectionRef->GetInventoryByID(FromInventory);
	PendingContextSlotGuid = SlotGuid;

	auto AllowedActions = UInvenzayUtility::CollectAccessibleObjectActions(GetWorld(), Item);

	auto InventoryActions = CollectAccessibleInventoryActions();
	for (const auto& Pair : InventoryActions)
	{
		AllowedActions.Add(Pair);
	}

	//ValidateInventoryContextActions(Item, FromInventory, SlotGuid,AllowedActions);

	if (AllowedActions.Num() == 0)
	{
		return;
	}

	UModalWindowManager* ModalManager = nullptr;
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->GetLocalPlayer())
	{
		ModalManager = PC->GetLocalPlayer()->GetSubsystem<UModalWindowManager>();
	}
	if (!ModalManager)
	{
		return;
	}

	FModalResultDelegate ResponseDelegate;
	ResponseDelegate.BindDynamic(this, &UIInventoryManager::OnInventoryModalResponse);

	FModalHeaderData HeaderData(EModalHeaderType::None, FText());
	ModalManager->OpenModalFlow(PendingContextItem, HeaderData, EModalFooterType::Selection, AllowedActions,
								ResponseDelegate);
}

UInventoryBase* UIInventoryManager::ResolveTargetInventory(UInventoryBase* SourceInventory)
{
	if (!SourceInventory)
	{
		return nullptr;
	}

	if (CharacterInventoryAccess)
	{
		return SourceInventory->GetItemCollectionLinked() == ItemCollectionRef
				   ? CharacterInventoryAccess->GetMainLootContainer().Get()
				   : MainPawnInventoryRef.Get();
	}

	if (ItemCollectionRef->GetLinkedInventories().VendorInventory)
	{
		return (SourceInventory == ItemCollectionRef->GetLinkedInventories().VendorInventory)
				   ? (MainPawnInventoryRef.Get())
				   : ItemCollectionRef->GetLinkedInventories().VendorInventory.Get();
	}

	if (ItemCollectionRef->GetLinkedInventories().ExternalInventory)
	{
		return (SourceInventory == ItemCollectionRef->GetLinkedInventories().ExternalInventory)
				   ? (MainPawnInventoryRef.Get())
				   : ItemCollectionRef->GetLinkedInventories().ExternalInventory.Get();
	}

	return MainPawnInventoryRef;
}

bool UIInventoryManager::FindSuitableEquipmentSlot(UObject* Item, UInventoryBase*& OutEquipmentInventory,
												   UInventorySlotData*& OutSuitableSlot,
												   UInventorySlotData*& OutFreeSlot)
{
	OutEquipmentInventory = nullptr;
	OutSuitableSlot = nullptr;
	OutFreeSlot = nullptr;

	if (!ItemCollectionRef || !Item || !EquipmentComponentRef || !GlobalSettings)
	{
		return false;
	}

	const FGameplayTag RequiredCategory = IObjectDataProvider::Execute_GetItemRef(Item).ItemCategory;
	if (!RequiredCategory.IsValid())
	{
		return false;
	}

	for (UInventoryBase* EquipmentInventory :
		 ItemCollectionRef->GetAllInventoriesByTag(GlobalSettings->EquipmentInvTag))
	{
		if (!EquipmentInventory)
		{
			continue;
		}

		for (const FGuid& SlotID : EquipmentInventory->GetSlotsWithLinkedEquipment())
		{
			UInventorySlotData* Slot = EquipmentInventory->GetSlotByGuid(SlotID);
			if (!Slot)
			{
				continue;
			}

			const FGameplayTag& AllowedCategory = Slot->InventorySlotInfo.AllowedCategory;
			const FGameplayTag& EquipmentSlotTag = Slot->InventorySlotInfo.LinkedEquipmentSlot;

			if (!AllowedCategory.IsValid() || !EquipmentSlotTag.IsValid() ||
				!AllowedCategory.MatchesTagExact(RequiredCategory) ||
				!EquipmentComponentRef->DoesSlotExist(EquipmentSlotTag) ||
				!EquipmentComponentRef->IsCategoryCompatibleWithSlot(EquipmentSlotTag, RequiredCategory))
			{
				continue;
			}

			if (!OutSuitableSlot)
			{
				OutSuitableSlot = Slot;
				OutEquipmentInventory = EquipmentInventory;
			}

			if (EquipmentInventory->bIsSlotEmpty(Slot, TArray<UInventorySlotData*>()))
			{
				OutFreeSlot = Slot;
				OutEquipmentInventory = EquipmentInventory;
				return true;
			}
		}
	}

	return OutSuitableSlot != nullptr;
}

void UIInventoryManager::OnQuickTransferItem_Implementation(FItemMoveData InData)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_OnQuickTransferItem(InData);
	}
	else
	{
		Handle_QuickTransfer_Internal(InData);
	}
}

void UIInventoryManager::Server_OnQuickTransferItem_Implementation(FItemMoveData InData)
{
	Handle_QuickTransfer_Internal(InData);
}

void UIInventoryManager::Handle_QuickTransfer_Internal(FItemMoveData InData)
{
	if (!MainPawnInventoryRef)
	{
		return;
	}

	FItemMoveData ItemMoveData = InData;

	if (ItemMoveData.SourceInventory == ItemMoveData.TargetInventory)
	{
		return;
	}

	ItemMoveData.TargetInventory = ResolveTargetInventory(ItemMoveData.SourceInventory);

	if (!ItemMoveData.TargetInventory)
	{
		return;
	}

	Execute_ItemTransferRequest(this, ItemMoveData);
}

void UIInventoryManager::OnQuickTransferAllSameItems_Implementation(FItemMoveData ItemMoveData)
{
	ItemMoveData.TargetInventory = ResolveTargetInventory(ItemMoveData.SourceInventory);

	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_OnQuickTransferAllSameItems(ItemMoveData);
	}
	else
	{
		Handle_QuickTransferAllSameItems_Internal(ItemMoveData);
	}
}

void UIInventoryManager::Server_OnQuickTransferAllSameItems_Implementation(FItemMoveData ItemMoveData)
{
	Handle_QuickTransferAllSameItems_Internal(ItemMoveData);
}

void UIInventoryManager::Handle_QuickTransferAllSameItems_Internal(FItemMoveData InData)
{
	auto InvID = InData.SourceInventory->GetInventoryContainerID();

	auto SameItems = InData.SourceInventory->GetItemCollectionLinked()->GetAllSameItemsInContainerByItemSample(
		InvID, InData.SourceItem);
	for (auto Item : SameItems)
	{
		InData.SourceItem = Item;
		Execute_OnQuickTransferItem(this, InData);
	}
}

void UIInventoryManager::TransferItemArray_Implementation(UInventoryBase* SourceInventory,
														  UInventoryBase* TargetInventory)
{
	if (!SourceInventory || !TargetInventory)
	{
		return;
	}

	const FString SourceContainerID = SourceInventory->GetInventoryContainerID();
	TArray<UObject*> Items = SourceInventory->GetItemCollectionLinked()->GetAllItemsByContainer(SourceContainerID);

	if (Items.IsEmpty())
	{
		return;
	}

	for (UObject* Item : Items)
	{
		if (!Item)
		{
			continue;
		}

		FItemMoveData MoveData;

		MoveData.SourceItem = Item;
		MoveData.SourceInventory = SourceInventory;
		MoveData.TargetInventory = TargetInventory;

		Execute_ItemTransferRequest(this, MoveData);
	}
}

bool UIInventoryManager::CanTransferMatchingItems(UInventoryBase* SourceInventory,
												  UInventoryBase* TargetInventory) const
{
	if (!IsValid(SourceInventory) || !IsValid(TargetInventory) || SourceInventory == TargetInventory ||
		!ItemCollectionRef)
	{
		return false;
	}
	if (VendorProviderCurrent.GetObject() || SourceInventory->GetInventorySettings().bIsReferenceContainer ||
		TargetInventory->GetInventorySettings().bIsReferenceContainer)
	{
		return false;
	}
	const auto OwnedInventories = ItemCollectionRef->GetActorInventories();
	const auto Linked = ItemCollectionRef->GetLinkedInventories();
	const auto IsAccessible = [&](UInventoryBase* Inventory)
	{
		return Inventory->GetItemCollectionLinked() &&
			   (OwnedInventories.Contains(Inventory) || Inventory == Linked.ExternalInventory ||
				IsStorageInventoryAccessible(Inventory));
	};
	return IsAccessible(SourceInventory) && IsAccessible(TargetInventory);
}

void UIInventoryManager::TransferMatchingItems(UInventoryBase* SourceInventory, UInventoryBase* TargetInventory)
{
	if (!CanTransferMatchingItems(SourceInventory, TargetInventory) || !GetOwner())
	{
		return;
	}
	if (!GetOwner()->HasAuthority())
	{
		Server_TransferMatchingItems(SourceInventory, TargetInventory);
		return;
	}

	TSet<FName> TargetItemIDs;
	for (UObject* Item :
		 TargetInventory->GetItemCollectionLinked()->GetAllItemsByContainer(TargetInventory->GetInventoryContainerID()))
	{
		if (IsValid(Item) && Item->Implements<UObjectDataProvider>() &&
			IObjectDataProvider::Execute_GetQuantity(Item) > 0)
		{
			TargetItemIDs.Add(IObjectDataProvider::Execute_GetItemID(Item));
		}
	}
	if (TargetItemIDs.IsEmpty())
	{
		return;
	}

	// Snapshot before moving: each successful transfer can mutate the source collection.
	auto* SourceCollection = SourceInventory->GetItemCollectionLinked();
	const FString SourceID = SourceInventory->GetInventoryContainerID();
	const TArray<UObject*> Items = SourceCollection->GetAllItemsByContainer(SourceID);
	for (UObject* Item : Items)
	{
		if (!IsValid(Item) || !Item->Implements<UObjectDataProvider>() ||
			!TargetItemIDs.Contains(IObjectDataProvider::Execute_GetItemID(Item)))
		{
			continue;
		}
		// Reserved stacks must not be consumed by a bulk storage operation.
		const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(Item);
		if (Quantity <= 0 || IObjectDataProvider::Execute_GetFreeAmount(Item) < Quantity)
		{
			continue;
		}
		const FItemMapping* Mapping = SourceCollection->FindItemMappingByContainerName(Item, SourceID);
		if (!Mapping)
		{
			continue;
		}
		FItemMoveData Move;
		Move.SourceItem = Item;
		Move.SourceInventory = SourceInventory;
		Move.TargetInventory = TargetInventory;
		Move.SavedOrientation = Mapping->ItemOrientation;
		Move.TargetOrientation = Mapping->ItemOrientation;
		if (!Mapping->OccupiedSlots.IsEmpty() && Mapping->OccupiedSlots[0])
		{
			Move.SourceSlotID = Mapping->OccupiedSlots[0]->InventorySlotInfo.SlotGuid;
		}
		Execute_ItemTransferRequest(this, Move);
	}
}

void UIInventoryManager::Server_TransferMatchingItems_Implementation(UInventoryBase* SourceInventory,
																	 UInventoryBase* TargetInventory)
{
	TransferMatchingItems(SourceInventory, TargetInventory);
}

bool UIInventoryManager::IsTradeSessionInventory(const UInventoryBase* Inventory) const
{
	return Inventory && TradeSessionItems.Inventory == Inventory;
}

bool UIInventoryManager::WasSoldThisSession(const UInventoryBase* Inventory, const UObject* Item) const
{
	return IsTradeSessionInventory(Inventory) && IsValid(Item) && TradeSessionItems.SoldItems.Contains(Item);
}

void UIInventoryManager::OnRep_TradeSessionItems()
{
	OnTradeSessionItemsChanged.Broadcast();
}

void UIInventoryManager::ResetTradeSession(UInventoryBase* Inventory)
{
	TradeSessionItems.Reset(Inventory);
	OnRep_TradeSessionItems();
}

void UIInventoryManager::EndTradeSession()
{
	if (auto* Target = Cast<UInteractableComponent>(VendorProviderCurrent.GetObject());
		Target && TrackedInteractionTarget == Target)
	{
		HandleClearInteraction(Target, TrackedInteractionType);
		return;
	}
	if (!TradeSessionItems.Inventory && !VendorProviderCurrent.GetObject())
	{
		return;
	}
	if (IsValid(VendorProviderCurrent.GetObject()))
	{
		VendorProviderCurrent->SetTradePartnerInventory(nullptr);
		VendorProviderCurrent->SetTradePartnerItemCollection(nullptr);
	}
	VendorProviderCurrent = nullptr;
	ResetTradeSession();
	if (ItemCollectionRef && ItemCollectionRef->GetLinkedInventories().VendorInventory)
	{
		ItemCollectionRef->SetVendorInventory(nullptr);
	}
}

void UIInventoryManager::CloseTradeSession()
{
	if (bClosingSecondaryInventory || !TradeSessionItems.Inventory || !GetOwner())
	{
		return;
	}
	if (GetOwner()->HasAuthority())
	{
		EndTradeSession();
	}
	else
	{
		Server_CloseTradeSession(TradeSessionItems.SessionId);
		// Immediate local reset; the authoritative session revision arrives through replication.
		TradeSessionItems.Inventory = nullptr;
		TradeSessionItems.SoldItems.Reset();
		OnRep_TradeSessionItems();
	}
}

void UIInventoryManager::Server_CloseTradeSession_Implementation(uint32 SessionId)
{
	// A delayed close from an older window must not clear a newer trade session.
	if (SessionId == TradeSessionItems.SessionId)
	{
		EndTradeSession();
	}
}

void UIInventoryManager::VendorRequest(FItemMoveData ItemMoveData)
{
	if (ItemMoveData.SourceInventory == ItemMoveData.TargetInventory)
	{
		return;
	}

	UInventoryBase* VendorInventory = VendorProviderCurrent->GetVendorLootContainer();
	UInventoryCollectionComponent* VendorCollection =
		VendorInventory ? VendorInventory->GetItemCollectionLinked() : nullptr;
	TMap<UObject*, int32> PreviousQuantities;
	if (VendorCollection && IsTradeSessionInventory(VendorInventory))
	{
		for (UObject* Item : VendorCollection->GetAllItemsByContainer(VendorInventory->GetInventoryContainerID()))
		{
			if (IsValid(Item) && Item->Implements<UObjectDataProvider>())
			{
				PreviousQuantities.Add(Item, IObjectDataProvider::Execute_GetQuantity(Item));
			}
		}
	}
	FTradeResult TradeResult = VendorProviderCurrent->ProcessTradeRequest(ItemMoveData);
	if (VendorCollection && IsTradeSessionInventory(VendorInventory) &&
		TradeResult.OperationResult == ETradeResult::TR_Success)
	{
		TMap<UObject*, int32> CurrentQuantities;
		for (UObject* Item : VendorCollection->GetAllItemsByContainer(VendorInventory->GetInventoryContainerID()))
		{
			if (IsValid(Item) && Item->Implements<UObjectDataProvider>())
			{
				CurrentQuantities.Add(Item, IObjectDataProvider::Execute_GetQuantity(Item));
			}
		}
		TradeSessionItems.RecordTrade(TradeResult, PreviousQuantities, CurrentQuantities);
		OnRep_TradeSessionItems();
	}
	const UEnum* TradeResultEnum = StaticEnum<ETradeResult>();
	/*const FString OperationResultName = TradeResultEnum
		? TradeResultEnum->GetNameStringByValue(static_cast<int64>(TradeResult.OperationResult))
		: TEXT("Unknown");

	UE_LOG(LogTemp, Log,
		TEXT("=== Trade Result ===\n")
		TEXT("ItemsTraded: %d\n")
		TEXT("MoneySpent: %d\n")
		TEXT("MoneyReceived: %d\n")
		TEXT("OperationResult: %s\n")
		TEXT("ResultMessage: %s"),
		TradeResult.ItemsTraded,
		TradeResult.MoneySpent,
		TradeResult.MoneyReceived,
		*OperationResultName,
		*TradeResult.ResultMessage.ToString()
	);	*/

	if (ItemMoveData.SourceInventory)
	{
		ItemMoveData.SourceInventory->UpdateMoneyInfo();
		ItemMoveData.SourceInventory->UpdateWeightInfo();
	}
	if (ItemMoveData.TargetInventory)
	{
		ItemMoveData.TargetInventory->UpdateMoneyInfo();
		ItemMoveData.TargetInventory->UpdateWeightInfo();
	}
}

void UIInventoryManager::ItemTransferRequest_Implementation(FItemMoveData ItemMoveData)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_ItemTransferRequest(ItemMoveData);
	}
	else
	{
		Handle_ItemTransferRequest(ItemMoveData);
	}
}

void UIInventoryManager::Server_ItemTransferRequest_Implementation(FItemMoveData ItemMoveData)
{
	Handle_ItemTransferRequest(ItemMoveData);
}

void UIInventoryManager::Handle_ItemTransferRequest(FItemMoveData ItemMoveData)
{
	if (GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->IsApplyingSnapshot())
	{
		return;
	}
	if (!IsInventoryAccessible(ItemMoveData.TargetInventory))
	{
		return;
	}
	if (ItemMoveData.SourceInventory)
	{
		if (!IsInventoryAccessible(ItemMoveData.SourceInventory))
		{
			return;
		}
		auto* Collection = ItemMoveData.SourceInventory->GetItemCollectionLinked();
		if (!Collection || !Collection->FindItemMappingByContainerName(
							   ItemMoveData.SourceItem, ItemMoveData.SourceInventory->GetInventoryContainerID()))
		{
			return;
		}
	}

	if (VendorProviderCurrent)
	{
		if (ItemMoveData.TargetInventory == VendorProviderCurrent->GetVendorLootContainer() ||
			ItemMoveData.SourceInventory == VendorProviderCurrent->GetVendorLootContainer())
		{
			VendorRequest(ItemMoveData);
		}
		return;
	}

	ApplyItemMove(ItemMoveData);
}

void UIInventoryManager::ApplyItemMove(FItemMoveData ItemMoveData)
{
	if (!GetOwner()->HasAuthority() || !IsValid(ItemMoveData.TargetInventory))
	{
		return;
	}
	if (TryApplyEquipmentTransfer(ItemMoveData))
	{
		if (IsValid(ItemMoveData.SourceInventory))
		{
			ItemMoveData.SourceInventory->RequestToResetItemVisual(ItemMoveData.SourceItem);
		}
		NormalizeLinkedStorageTransferOwnership(ItemMoveData);
		return;
	}
	if (TryApplyLinkedStorageTransfer(ItemMoveData))
	{
		return;
	}
	FItemAddResult Result = ItemMoveData.TargetInventory->HandleAddItem(ItemMoveData, false);
	auto ActualAmountAdded = Result.ActualAmountAdded;

	bool bIsSourceInvExist = false;

	if (ItemMoveData.SourceInventory)
	{
		bIsSourceInvExist = true;
		ItemMoveData.SourceInventory->RequestToResetItemVisual(ItemMoveData.SourceItem);
	}

	const bool bNeedUIUpdate = GetNetMode() != NM_DedicatedServer;

	switch (Result.OperationResult)
	{
	case EItemAddResult::IAR_AllItemAdded:
	{
		if (ItemMoveData.SourceInventory && ItemMoveData.SourceInventory->GetItemCollectionLinked())
		{
			if (!ItemMoveData.TargetInventory->GetInventorySettings().bIsReferenceContainer)
			{
				ItemMoveData.SourceInventory->HandleRemoveItem(ItemMoveData.SourceItem, ActualAmountAdded);
			}
		}
		UnequipItemIfNeeded(ItemMoveData.SourceInventory, ItemMoveData.SourceSlotID);

		const EInventoryContextActionResult ContextResult = ValidateAndEquipTransferredItem(ItemMoveData);
		if (ContextResult == EInventoryContextActionResult::EquipmentSlotNotFound)
		{
			ItemMoveData.TargetInventory->HandleRemoveItem(ItemMoveData.SourceItem, ActualAmountAdded);

			FItemMoveData ReverseMoveData;
			ReverseMoveData.SourceInventory = ItemMoveData.TargetInventory;
			ReverseMoveData.SourceItem = ItemMoveData.SourceItem;
			ReverseMoveData.TargetInventory = ItemMoveData.SourceInventory;

			Execute_ItemTransferRequest(this, ReverseMoveData);
		}
	}
	break;
	case EItemAddResult::IAR_NoItemAdded:
		if (ItemMoveData.SourceInventory == nullptr)
		{
			const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(ItemMoveData.SourceItem);

			FItemDropData DropData(ItemMoveData.SourceItem, ItemMoveData.SourceInventory, Quantity);

			ItemDropRequest(DropData);
		}
		break;
	case EItemAddResult::IAR_PartialAmountItemAdded:
		if (Result.bIsUsedReferences)
		{
			break;
		}
		if (ItemMoveData.SourceInventory)
		{
			ItemMoveData.SourceInventory->HandleRemoveItem(ItemMoveData.SourceItem, ActualAmountAdded);
			break;
		}
		break;
	case EItemAddResult::IAR_ItemSwapped:
		if (!Result.bIsUsedReferences && ItemMoveData.SourceInventory->GetInventorySettings().bAllowItemReferencing &&
			ItemMoveData.SourceInventory != ItemMoveData.TargetInventory)
		{
			ItemMoveData.SourceInventory->HandleRemoveItem(ItemMoveData.SourceItem, ActualAmountAdded);
		}
		break;
	}

	if (ItemMoveData.SourceInventory)
	{
		ItemMoveData.SourceInventory->UpdateMoneyInfo();
		ItemMoveData.SourceInventory->UpdateWeightInfo();
	}
	if (ItemMoveData.TargetInventory)
	{
		ItemMoveData.TargetInventory->UpdateMoneyInfo();
		ItemMoveData.TargetInventory->UpdateWeightInfo();
	}
	NormalizeLinkedStorageTransferOwnership(ItemMoveData);
}

EInventoryContextActionResult UIInventoryManager::ValidateAndEquipTransferredItem(FItemMoveData& ItemMoveData)
{
	FEquipmentInventoryContext Context;
	if (!TryResolveEquipmentContext(ItemMoveData.TargetInventory, ItemMoveData.TargetSlotID, Context))
	{
		return EInventoryContextActionResult::NotApplicable;
	}
	if (!GetOwner()->HasAuthority() || !IsInventoryAccessible(ItemMoveData.TargetInventory) ||
		!IsValid(ItemMoveData.SourceItem) || !ItemMoveData.SourceItem->Implements<UObjectDataProvider>() ||
		!Context.Slot)
	{
		return EInventoryContextActionResult::InvalidSlot;
	}
	if (!Context.Equipment->DoesSlotExist(Context.SlotTag))
	{
		return EInventoryContextActionResult::EquipmentSlotNotFound;
	}
	if (Context.Equipment->IsItemEquippedInSlot(ItemMoveData.SourceItem, Context.SlotTag))
	{
		return EInventoryContextActionResult::ItemAlreadyEquipped;
	}
	if (!Context.Equipment->IsCategoryCompatibleWithSlot(
			Context.SlotTag, IObjectDataProvider::Execute_GetItemRef(ItemMoveData.SourceItem).ItemCategory))
	{
		return EInventoryContextActionResult::IncompatibleSlot;
	}
	if (Context.Equipment->IsSlotOccupied(Context.SlotTag))
	{
		return EInventoryContextActionResult::SlotOccupied;
	}
	if (Context.Collection->GetItemFromSlot(ItemMoveData.TargetSlotID,
											ItemMoveData.TargetInventory->GetInventoryContainerID()) !=
		ItemMoveData.SourceItem)
	{
		return EInventoryContextActionResult::InvalidSlot;
	}
	Context.Equipment->Server_EquipItemToSlot(Context.SlotTag, ItemMoveData.SourceItem);
	return EInventoryContextActionResult::Success;
}

void UIInventoryManager::UnequipItemIfNeeded(UInventoryBase* SourceInventory, FGuid SourceSlotID)
{
	FEquipmentInventoryContext Context;
	if (GetOwner()->HasAuthority() && IsInventoryAccessible(SourceInventory) &&
		TryResolveEquipmentContext(SourceInventory, SourceSlotID, Context) && Context.SlotTag.IsValid())
	{
		Context.Equipment->Server_UnequipItemFromSlot(Context.SlotTag);
	}
}

void UIInventoryManager::ItemSplitRequest_Implementation(UInventoryBase* TargetInventory, UObject* ItemToSplit,
														 int32 SplitAmount)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_ItemSplitRequest(TargetInventory, ItemToSplit, SplitAmount);
	}
	else
	{
		Handle_SplitItem(TargetInventory, ItemToSplit, SplitAmount);
	}
}

void UIInventoryManager::Server_ItemSplitRequest_Implementation(UInventoryBase* TargetInventory, UObject* ItemToSplit,
																int32 SplitAmount)
{
	Handle_SplitItem(TargetInventory, ItemToSplit, SplitAmount);
}

void UIInventoryManager::Handle_SplitItem(UInventoryBase* TargetInventory, UObject* ItemToSplit, int32 SplitAmount)
{
	if (!IsInventoryAccessible(TargetInventory) || !TargetInventory->GetItemCollectionLinked() ||
		!TargetInventory->GetItemCollectionLinked()->FindItemMappingByContainerName(
			ItemToSplit, TargetInventory->GetInventoryContainerID()))
	{
		return;
	}
	if (!TargetInventory || !ItemToSplit || SplitAmount <= 0)
	{
		return;
	}

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemToSplit, TEXT("Handle_SplitItem")))
	{
		return;
	}

	const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(ItemToSplit);

	if (Quantity == 1 || Quantity <= SplitAmount)
	{
		return;
	}

	EItemOrientationType FinalOrientation;

	auto EmptySlots = TargetInventory->GetAvailableSlotForItem(ItemToSplit, FinalOrientation);

	FGuid PivotID;
	if (!EmptySlots.IsEmpty())
	{
		PivotID = EmptySlots[0]->InventorySlotInfo.SlotGuid;
	}

	auto TarInvSettings = TargetInventory->GetInventorySettings();
	auto TarInvCollection = TargetInventory->GetItemCollectionLinked();

	if (TarInvSettings.MaxStackCount > 0)
	{
		auto ResultMaxStack = TarInvCollection->GetStackCountInContainer(TargetInventory->GetInventoryContainerID());
		if (ResultMaxStack + 1 > TarInvSettings.MaxStackCount)
		{
			return;
		}
	}

	UObject* NewItem = IObjectDataProvider::Execute_DuplicateItem(ItemToSplit);
	if (!NewItem ||
		!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(NewItem, TEXT("Handle_SplitItem")))
	{
		return;
	}

	IObjectDataProvider::Execute_SetQuantity(NewItem, SplitAmount);

	FItemMoveData ItemMove;
	ItemMove.SourceItem = NewItem;
	ItemMove.TargetInventory = TargetInventory;
	ItemMove.TargetSlotID = PivotID;
	ItemMove.SavedOrientation = FinalOrientation;
	ItemMove.TargetOrientation = FinalOrientation;

	TargetInventory->HandleRemoveItem(ItemToSplit, SplitAmount);
	TargetInventory->HandleAddItem(ItemMove, false);

	TargetInventory->UpdateMoneyInfo();
	TargetInventory->UpdateWeightInfo();
}

void UIInventoryManager::ItemDropRequest_Implementation(FItemDropData DropData)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_OnItemDrop(DropData);
	}
	else
	{
		HandleItemDrop(DropData);
	}
}

void UIInventoryManager::Server_OnItemDrop_Implementation(FItemDropData DropData)
{
	HandleItemDrop(DropData);
}

void UIInventoryManager::HandleItemDrop(FItemDropData DropData)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!IsValid(Pawn) || !Pawn->HasAuthority() || DropData.DropAmount <= 0 || !IsValid(DropData.ItemToDrop) ||
		!DropData.ItemToDrop->Implements<UObjectDataProvider>() || !IsInventoryAccessible(DropData.SourceInventory))
	{
		return;
	}
	UInventoryCollectionComponent* Collection = DropData.SourceInventory->GetItemCollectionLinked();
	if (!IsValid(Collection) || !IsValid(Collection->GetOwner()) || !Collection->GetOwner()->HasAuthority() ||
		!Collection->FindItemMappingByContainerName(
			DropData.ItemToDrop, DropData.SourceInventory->GetInventoryContainerID()))
	{
		return;
	}

	// Reference slots identify the item; only a physical mapping owns its quantity.
	const FItemMapping* PhysicalMapping = Collection->FindMainInventoryMappingForItem(DropData.ItemToDrop);
	UInventoryBase* PhysicalInventory = PhysicalMapping ? Collection->GetInventoryByID(PhysicalMapping->InventoryID) : nullptr;
	if (!IsInventoryAccessible(PhysicalInventory) || PhysicalInventory->GetInventorySettings().bIsReferenceContainer ||
		PhysicalInventory->GetItemCollectionLinked() != Collection)
	{
		return;
	}

	const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(DropData.ItemToDrop);
	if (Quantity <= 0 || DropData.DropAmount > Quantity)
	{
		return;
	}

	const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(DropData.ItemToDrop);

	if (!Meta.bIsDroppable)
	{
		return;
	}

	const FVector SpawnLocation = Pawn->GetActorLocation() + Pawn->GetActorForwardVector() * 50.f;
	const FRotator SpawnRotation = Pawn->GetActorRotation();
	const FDataTableRowHandle ItemRow = IObjectDataProvider::Execute_GetItemRow(DropData.ItemToDrop);
	AActor* Pickup = UInvenzayUtility::DropItem(GetWorld(), Pawn, ItemRow, DropData.DropAmount, SpawnLocation, SpawnRotation);
	if (!IsValid(Pickup))
	{
		return;
	}

	// Spawning executes actor code. Reject stale requests again before changing inventory contents.
	PhysicalMapping = Collection->FindMainInventoryMappingForItem(DropData.ItemToDrop);
	if (!IsValid(DropData.ItemToDrop) || !IsInventoryAccessible(DropData.SourceInventory) ||
		!IsInventoryAccessible(PhysicalInventory) || !PhysicalMapping ||
		PhysicalMapping->InventoryID != PhysicalInventory->GetInventoryContainerID() ||
		!Collection->FindItemMappingByContainerName(DropData.ItemToDrop, DropData.SourceInventory->GetInventoryContainerID()) ||
		IObjectDataProvider::Execute_GetQuantity(DropData.ItemToDrop) != Quantity)
	{
		Pickup->Destroy();
		return;
	}

	TArray<FString> AffectedInventories;
	for (const FItemMapping* Mapping : Collection->FindAllMappingsForItem(DropData.ItemToDrop))
	{
		if (Mapping)
		{
			AffectedInventories.AddUnique(Mapping->InventoryID);
		}
	}

	if (DropData.DropAmount == Quantity)
	{
		Collection->RemoveItemFromAllContainers(DropData.ItemToDrop);
	}
	else
	{
		PhysicalInventory->HandleRemoveItem(DropData.ItemToDrop, DropData.DropAmount);
		Collection->MarkItemAsDirty(DropData.ItemToDrop);
	}

	for (const FString& InventoryID : AffectedInventories)
	{
		if (DropData.DropAmount < Quantity)
		{
			UInventoryBase* Inventory = Collection->GetInventoryByID(InventoryID);
			if (IsValid(Inventory) && Inventory != PhysicalInventory)
			{
				Inventory->UpdateWeightInfo();
				Inventory->UpdateMoneyInfo();
			}
		}
		Collection->OnInventoryItemsChanged.Broadcast(InventoryID);
		Collection->NotifyUI_ReDraw(InventoryID);
	}
}

void UIInventoryManager::ItemDeleteRequest_Implementation(const FString& FromInventory, UObject* Item)
{
	Server_OnItemDelete(FromInventory, Item);
}

void UIInventoryManager::Server_OnItemDelete_Implementation(const FString& FromInventory, UObject* Item)
{
	if (!ItemCollectionRef)
	{
		return;
	}

	auto* Inventory = ItemCollectionRef->GetInventoryByID(FromInventory);
	if (!IsInventoryAccessible(Inventory))
	{
		return;
	}
	auto* Collection = Inventory->GetItemCollectionLinked();
	if (Collection && Collection->FindItemMappingByContainerName(Item, FromInventory))
	{
		Collection->RemoveItemFromAllContainers(Item);
	}
}

void UIInventoryManager::ItemEquipRequest_Implementation(UObject* Item)
{
	if (!ItemCollectionRef || !Item || !EquipmentComponentRef)
	{
		return;
	}

	Server_ItemEquipRequest(Item);
}

void UIInventoryManager::Server_ItemEquipRequest_Implementation(UObject* Item)
{
	if (!IsValid(Item) || !ItemCollectionRef || !EquipmentComponentRef)
	{
		return;
	}
	const auto* Mapping = ItemCollectionRef->FindMainInventoryMappingForItem(Item);
	if (!Mapping || Mapping->OccupiedSlots.IsEmpty())
	{
		return;
	}
	auto* CurrentInventory = ItemCollectionRef->GetInventoryByID(Mapping->InventoryID);
	if (!IsInventoryAccessible(CurrentInventory))
	{
		return;
	}
	UInventoryBase* EquipmentInventory = nullptr;
	UInventorySlotData* SuitableSlot = nullptr;
	UInventorySlotData* FreeSlot = nullptr;
	if (!FindSuitableEquipmentSlot(Item, EquipmentInventory, SuitableSlot, FreeSlot))
	{
		return;
	}
	auto* TargetSlot = FreeSlot ? FreeSlot : SuitableSlot;
	if (!TargetSlot)
	{
		return;
	}
	FItemMoveData Move(Item, CurrentInventory, Mapping->OccupiedSlots[0]->InventorySlotInfo.SlotGuid,
					   EquipmentInventory, TargetSlot->InventorySlotInfo.SlotGuid);
	Move.SavedOrientation = Mapping->ItemOrientation;
	Handle_ItemTransferRequest(Move);
}

void UIInventoryManager::ItemUnequipRequest_Implementation(UObject* Item)
{
	if (!ItemCollectionRef || !Item || !EquipmentComponentRef)
	{
		return;
	}

	Server_ItemUnequipRequest(Item);
}

void UIInventoryManager::Server_ItemUnequipRequest_Implementation(UObject* Item)
{
	if (!IsValid(Item) || !ItemCollectionRef || !GlobalSettings || !MainPawnInventoryRef)
	{
		return;
	}
	for (auto* Inventory : ItemCollectionRef->GetActorInventories())
	{
		if (!IsValid(Inventory) || Inventory->GetInventorySettings().InventoryTag != GlobalSettings->EquipmentInvTag)
		{
			continue;
		}
		const auto* Mapping =
			ItemCollectionRef->FindItemMappingByContainerName(Item, Inventory->GetInventoryContainerID());
		if (!Mapping || Mapping->OccupiedSlots.IsEmpty())
		{
			continue;
		}
		FItemMoveData Move(Item, Inventory, Mapping->OccupiedSlots[0]->InventorySlotInfo.SlotGuid, MainPawnInventoryRef,
						   {});
		Move.SavedOrientation = Mapping->ItemOrientation;
		Handle_ItemTransferRequest(Move);
		return;
	}
}

void UIInventoryManager::RequestUseSlot_Implementation(const FString& InvID, FGuid SlotID)
{
	Server_RequestUseSlot(InvID, SlotID);
}

void UIInventoryManager::Server_RequestUseSlot_Implementation(const FString& InvID, FGuid SlotID)
{
	if (InvID.IsEmpty())
	{
		return;
	}

	UInventoryBase* Inventory = ItemCollectionRef->GetInventoryByID(InvID);
	if (!Inventory)
	{
		return;
	}

	UInventorySlotData* Slot = Inventory->GetSlotByGuid(SlotID);
	if (!Slot)
	{
		return;
	}

	Inventory->UseSlot(Slot);
}

void UIInventoryManager::OnUseSlotInput(FString InvID, FGuid SlotID)
{
	RequestUseSlot(InvID, SlotID);
}

void UIInventoryManager::RebuildInventoryRequest_Implementation(const FString& InvID)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_RebuildInventory(InvID);
	}
	else
	{
		HandleRebuildInventory(InvID);
	}
}

void UIInventoryManager::Server_RebuildInventory_Implementation(const FString& InvID)
{
	HandleRebuildInventory(InvID);
}

void UIInventoryManager::HandleRebuildInventory(const FString& InvID)
{
	if (!ItemCollectionRef)
	{
		return;
	}

	auto FindResult = ItemCollectionRef->GetInventoryByID(InvID);
	if (!FindResult)
	{
		return;
	}

	FindResult->RebuildInventory();
}

void UIInventoryManager::InteractRequest(UInteractableComponent* TargetInteractableComponent, EInteractionType Type)
{
	if (!TargetInteractableComponent)
	{
		return;
	}
	if (GetOwner()->HasAuthority() || (OwnerPawnRef && OwnerPawnRef->IsLocallyControlled()))
	{
		Server_HandleInteract(TargetInteractableComponent, Type);
	}
}

void UIInventoryManager::TrackInventoryInteraction(UInteractableComponent* Target, EInteractionType Type)
{
	if (IsValid(TrackedInteractionTarget) && (TrackedInteractionTarget != Target || TrackedInteractionType != Type))
	{
		HandleClearInteraction(TrackedInteractionTarget.Get(), TrackedInteractionType);
	}
	if (!InteractionComponent)
	{
		InteractionComponent = GetOwner()->FindComponentByClass<UInteractionComponent>();
	}
	if (InteractionComponent)
	{
		InteractionComponent->OnStopInteract.AddUniqueDynamic(this, &ThisClass::InteractClearRequest);
		InteractionComponent->TrackActiveInteraction(Target, Type);
	}
	else
	{
		// Direct inventory API users can have no input/interaction component (e.g. AI).
		GetWorld()->GetTimerManager().SetTimer(FallbackInteractionRangeTimer, this,
											   &ThisClass::CheckInventoryInteractionRange, 0.25f, true);
	}
	TrackedInteractionTarget = Target;
	TrackedInteractionType = Type;
}

void UIInventoryManager::CheckInventoryInteractionRange()
{
	if (!UInteractionComponent::IsWithinInteractionDistance(GetOwner(), TrackedInteractionTarget.Get()))
	{
		HandleClearInteraction(TrackedInteractionTarget.Get(), TrackedInteractionType);
	}
}

void UIInventoryManager::Server_HandleInteract_Implementation(UInteractableComponent* Target, EInteractionType Type)
{
	if (!UInteractionComponent::IsWithinInteractionDistance(GetOwner(), Target) ||
		!Target->GetInteractableDataForType(Type) || !Target->TryAcquireInteraction(GetOwner()))
	{
		return;
	}

	if (StorageSession.AccessPoint && StorageSession.AccessPoint != Target)
	{
		EndStorageAccess();
	}
	auto InteractableData = Target->GetInteractableData(); // Primary determines the object's base EInteractableType.
	switch (InteractableData.DefaultInteractableType)
	{
	case EInteractableType::StorageAccess:
		OpenStorageAccess(Cast<UStorageAccessComponent>(Target));
		break;
	case EInteractableType::Pickup:
		HandlePickupInteraction(Target);
		break;
	case EInteractableType::Container:
		HandleContainerInteraction(Target);
		break;
	case EInteractableType::Vendor:
		HandleTradeInteraction(Target);
		break;
	case EInteractableType::Craft:
		HandleCraftStationInteraction(Target, Type);
		break;
	default:
		break;
	}
	if (TrackedInteractionTarget != Target &&
		(!InteractionComponent || InteractionComponent->GetActiveInteractable() != Target))
	{
		Target->ReleaseInteraction(GetOwner());
	}
}

void UIInventoryManager::HandlePickupInteraction(UInteractableComponent* Target)
{
	IPickupableass* PickupInterface = Cast<IPickupableass>(Target);
	if (!PickupInterface)
	{
		return;
	}

	if (UObject* ItemToPick = PickupInterface->GetItemData())
	{
		FItemMoveData Data;
		Data.SourceItem = ItemToPick;
		Data.TargetInventory = MainPawnInventoryRef;
		Data.SourceInventory = nullptr;

		Execute_ItemTransferRequest(this, Data);
	}
	PickupInterface->OnPickedUp();
}

void UIInventoryManager::HandleContainerInteraction(UInteractableComponent* Target)
{
	if (!GetOwner()->HasAuthority() || !ItemCollectionRef ||
		!UInteractionComponent::IsWithinInteractionDistance(GetOwner(), Target))
	{
		return;
	}
	ILootContainerProvider* LootProvider = Cast<ILootContainerProvider>(Target);
	if (!LootProvider)
	{
		return;
	}

	UInventoryBase* InventoryToDisplay = LootProvider->GetMainLootContainer();
	if (!InventoryToDisplay)
	{
		return;
	}
	auto* CharacterAccess = Cast<UCharacterInventoryAccessComponent>(Target);
	if (CharacterAccess && !CharacterAccess->TryBeginAccess(GetOwner()))
	{
		return;
	}
	TrackInventoryInteraction(Target, EInteractionType::Primary);

	Target->SetInteracting(true);

	if (CharacterAccess)
	{
		CharacterInventoryAccess = CharacterAccess;
		++CharacterInventoryRevision;
		OnRep_CharacterInventoryAccess();
		return;
	}
	SetInteractionOwnership(Target->GetOwner(), true);
	LootContainerProvider.SetObject(Target);
	LootContainerProvider.SetInterface(LootProvider);
	ItemCollectionRef->SetExternalInventory(InventoryToDisplay);
}

void UIInventoryManager::HandleTradeInteraction(UInteractableComponent* Target)
{
	if (!GetOwner()->HasAuthority() || !ItemCollectionRef ||
		!UInteractionComponent::IsWithinInteractionDistance(GetOwner(), Target))
	{
		return;
	}
	IVendorProvider* VendorProvider = Cast<IVendorProvider>(Target);
	if (!VendorProvider)
	{
		return;
	}

	auto InventoryToDisplay = VendorProvider->GetVendorLootContainer();
	if (!InventoryToDisplay)
	{
		return;
	}
	auto* CharacterAccess = Cast<UCharacterInventoryAccessComponent>(Target);
	if (CharacterAccess && !CharacterAccess->TryBeginAccess(GetOwner()))
	{
		return;
	}
	TrackInventoryInteraction(Target, EInteractionType::Primary);
	Target->SetInteracting(true);

	if (VendorProviderCurrent.GetObject() != Target || !IsTradeSessionInventory(InventoryToDisplay))
	{
		ResetTradeSession(InventoryToDisplay);
	}

	VendorProviderCurrent.SetObject(Target);
	VendorProviderCurrent.SetInterface(VendorProvider);

	FTradeContext TradeContext;
	TradeContext.TradeSettings = VendorProviderCurrent->GetTradeSettings();
	TradeContext.Vendor = Cast<AActor>(VendorProviderCurrent.GetObject());
	TradeContext.Buyer = GetOwner();

	ItemCollectionRef->SetVendorInventory(InventoryToDisplay);
	ItemCollectionRef->GetLinkedInventories().VendorInventory->SetTradeContext(TradeContext);

	VendorProviderCurrent->SetTradePartnerInventory(MainPawnInventoryRef);
	VendorProviderCurrent->SetTradePartnerItemCollection(ItemCollectionRef);
}

void UIInventoryManager::Server_SetStationAutomaticCrafting_Implementation(UCraftingComponent* Station, bool bAutomatic)
{
	if (bEndingPlay || !IsValid(GetOwner()) || !GetOwner()->HasAuthority() || !IsValid(Station) ||
		Station != ActiveCraftComponentRef || !IsValid(TrackedInteractionTarget) ||
		TrackedInteractionType != EInteractionType::Primary ||
		!TrackedInteractionTarget->IsInteractionOwnedBy(GetOwner()) ||
		!UInteractionComponent::IsWithinInteractionDistance(GetOwner(), TrackedInteractionTarget))
	{
		UE_LOG(LogTemp, Verbose,
			TEXT("Station crafting mode request rejected: player=%s, station=%s, active=%s, target=%s, type=%d, owned=%d, inRange=%d, ending=%d."),
			*GetNameSafe(GetOwner()), *GetNameSafe(Station), *GetNameSafe(ActiveCraftComponentRef.Get()),
			*GetNameSafe(TrackedInteractionTarget.Get()), static_cast<int32>(TrackedInteractionType),
			IsValid(TrackedInteractionTarget) && TrackedInteractionTarget->IsInteractionOwnedBy(GetOwner()),
			UInteractionComponent::IsWithinInteractionDistance(GetOwner(), TrackedInteractionTarget), bEndingPlay);
		return;
	}

	const ICraftProvider* Provider = Cast<ICraftProvider>(TrackedInteractionTarget);
	if (!Provider || Provider->GetCraftingComponent() != Station)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Station crafting mode request rejected: station does not match interaction."));
		return;
	}

	Station->SetAllowAutomaticCrafting(bAutomatic);
}

void UIInventoryManager::HandleCraftStationInteraction(UInteractableComponent* Target, EInteractionType Type)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !ItemCollectionRef ||
		!UInteractionComponent::IsWithinInteractionDistance(GetOwner(), Target))
	{
		return;
	}

	ICraftProvider* StationProvider = Cast<ICraftProvider>(Target);
	if (!StationProvider)
	{
		return;
	}

	UCraftingComponent* StationCraft = StationProvider->GetCraftingComponent();
	if (!StationCraft)
	{
		return;
	}
	const bool bAlreadyOperator =
		TrackedInteractionTarget == Target && TrackedInteractionType == EInteractionType::Secondary;
	TrackInventoryInteraction(Target, Type);

	SetInteractionOwnership(Target->GetOwner(), true);

	UCraftingComponent* Previous = ActiveCraftComponentRef;
	CraftProvider.SetObject(Target);
	CraftProvider.SetInterface(StationProvider);
	ActiveCraftComponentRef = StationCraft;
	ActiveCraftComponentRef->SetInteractorInventory(MainPawnInventoryRef);

	if (Type == EInteractionType::Secondary)
	{
		if (!bAlreadyOperator)
		{
			ActiveCraftComponentRef->AddOperator();
		}
		return;
	}

	ItemCollectionRef->SetCraftInventories(
		ActiveCraftComponentRef ? ActiveCraftComponentRef->GetInputInventory() : nullptr,
		ActiveCraftComponentRef ? ActiveCraftComponentRef->GetFuelInventory() : nullptr,
		ActiveCraftComponentRef ? ActiveCraftComponentRef->GetOutputInventory() : nullptr);
}

void UIInventoryManager::InteractClearRequest(UInteractableComponent* TargetInteractableComponent,
											  EInteractionType Type)
{
	if (!TargetInteractableComponent)
	{
		return;
	}
	// InteractionComponent sends its own versioned stop RPC. Do not echo another close.
	if (!GetOwner()->HasAuthority() && InteractionComponent && InteractionComponent->IsNotifyingStop())
	{
		return;
	}
	if (GetOwner()->HasAuthority())
	{
		Server_HandleClearInteract_Implementation(TargetInteractableComponent, Type);
	}
	else
	{
		Server_HandleClearInteract(TargetInteractableComponent, Type);
	}
}

void UIInventoryManager::Server_HandleClearInteract_Implementation(UInteractableComponent* Target,
																   EInteractionType Type)
{
	if (Target && (TrackedInteractionTarget != Target || TrackedInteractionType != Type))
	{
		return;
	}
	HandleClearInteraction(Target, Type);
}

void UIInventoryManager::HandleClearInteraction(UInteractableComponent* Target, EInteractionType Type)
{
	if (!Target)
	{
		Target = TrackedInteractionTarget.Get();
	}
	if (IsValid(Target) && Target->IsInteractionLockedByOther(GetOwner()))
	{
		return;
	}
	if (Target && IsValid(TrackedInteractionTarget) && TrackedInteractionTarget != Target)
	{
		return;
	}
	if (InteractionComponent && InteractionComponent->GetActiveInteractable() == Target && Target &&
		!InteractionComponent->IsNotifyingStop())
	{
		InteractionComponent->StopInteract();
		return;
	}
	if (CharacterInventoryAccess)
	{
		CharacterInventoryAccess->EndAccess(GetOwner());
		CharacterInventoryAccess = nullptr;
		++CharacterInventoryRevision;
		OnRep_CharacterInventoryAccess();
	}
	const bool bWasOperator = TrackedInteractionType == EInteractionType::Secondary;
	TrackedInteractionTarget = nullptr;
	GetWorld()->GetTimerManager().ClearTimer(FallbackInteractionRangeTimer);
	if (Target == StorageSession.AccessPoint)
	{
		EndStorageAccess();
	}
	EndTradeSession();
	if (ItemCollectionRef)
	{
		auto Linked = ItemCollectionRef->GetLinkedInventories();
		if (Linked.VendorInventory)
		{
			ItemCollectionRef->SetVendorInventory(nullptr);
		}
		if (Linked.ExternalInventory)
		{
			ItemCollectionRef->SetExternalInventory(nullptr);
		}
	}

	if (CraftProvider.GetObject())
	{
		if (bWasOperator && IsValid(ActiveCraftComponentRef))
		{
			ActiveCraftComponentRef->RemoveOperator();
		}

		if (auto* Provider = Cast<UActorComponent>(CraftProvider.GetObject()))
		{
			SetInteractionOwnership(Provider->GetOwner(), false);
		}

		CraftProvider.SetObject(nullptr);
		CraftProvider.SetInterface(nullptr);

		ActiveCraftComponentRef = nullptr;

		if (ItemCollectionRef)
		{
			ItemCollectionRef->SetCraftInventories(nullptr, nullptr, nullptr);
		}
	}
	if (auto* Provider = Cast<UActorComponent>(LootContainerProvider.GetObject()))
	{
		SetInteractionOwnership(Provider->GetOwner(), false);
	}
	auto PreviousLootProvider = LootContainerProvider;
	LootContainerProvider = nullptr;

	if (IsValid(Target))
	{
		Target->SetInteracting(false);
		Target->ReleaseInteraction(GetOwner());
	}
	if (IsValid(PreviousLootProvider.GetObject()))
	{
		PreviousLootProvider->CheckDestroyWhenEmpty();
	}
}

void UIInventoryManager::OpenSecondaryInventory(UInventoryBase* Inv, EInteractableType InteractableType)
{
	if (!Inv || !UIInvProvider || !ItemCollectionRef)
	{
		return;
	}

	UInventoryContainerWidget* SecondaryWidget = CreateInventoryWidget(Inv);
	UInventoryContainerWidget* PawnMainWidget = CreateInventoryWidget(MainPawnInventoryRef);
	if (!SecondaryWidget || !PawnMainWidget)
	{
		return;
	}

	UIInvProvider->OpenDualInventoryView(SecondaryWidget, PawnMainWidget);
}

void UIInventoryManager::CloseSecondaryInventory(EInteractableType InteractableType)
{
	if (!ItemCollectionRef || !UIInvProvider)
	{
		return;
	}

	// Replicated link removal already closes the authoritative session. Do not
	// send another close RPC, which could arrive after a new session was opened.
	TGuardValue<bool> ClosingGuard(bClosingSecondaryInventory, true);
	UIInvProvider->CloseDualInventoryView();

	switch (InteractableType)
	{
	case EInteractableType::Container:
		if (GetOwner())
		{
			Server_ClientClosedUI(InteractableType);
		}
		break;
	case EInteractableType::Vendor:
		break;
	default:
		break;
	}
}

void UIInventoryManager::HandleCraftInventoriesChanged(const FLinkedCraftInventories& Current)
{
	if (!UIInvProvider)
	{
		return;
	}

	if (Current.PrevInputInventory && Current.PrevInputInventory != Current.InputInventory)
	{
		ItemCollectionRef->UnregisterContainerWidget(Current.PrevInputInventory);
	}
	if (Current.PrevFuelInventory && Current.PrevFuelInventory != Current.FuelInventory)
	{
		ItemCollectionRef->UnregisterContainerWidget(Current.PrevFuelInventory);
	}
	if (Current.PrevOutputInventory && Current.PrevOutputInventory != Current.OutputInventory)
	{
		ItemCollectionRef->UnregisterContainerWidget(Current.PrevOutputInventory);
	}

	if (ActiveCraftComponentRef)
	{
		auto Dashboard = Cast<UCraftDashboard>(UIInvProvider->GetCraftMenuDashboard());
		if (!Dashboard)
		{
			return;
		}

		UInventoryContainerWidget* InputWidget = nullptr;
		UInventoryContainerWidget* FuelWidget = nullptr;
		UInventoryContainerWidget* OutputWidget = nullptr;

		if (Current.InputInventory)
		{
			InputWidget = CreateInventoryWidget(Current.InputInventory);
		}

		if (Current.FuelInventory && Current.FuelInventory != Current.InputInventory)
		{
			FuelWidget = CreateInventoryWidget(Current.FuelInventory);
		}

		if (Current.OutputInventory && Current.OutputInventory != Current.InputInventory &&
			Current.OutputInventory != Current.FuelInventory)
		{
			OutputWidget = CreateInventoryWidget(Current.OutputInventory);
		}

		Dashboard->SetCraftComponentPtr(ActiveCraftComponentRef);
		Dashboard->SetInventoryWidgets(InputWidget, FuelWidget, OutputWidget);

		UInventoryContainerWidget* InteractorWidget = ItemCollectionRef->GetContainerWidget(MainPawnInventoryRef);
		if (!InteractorWidget)
		{
			InteractorWidget = CreateInventoryWidget(MainPawnInventoryRef);
		}
		if (InteractorWidget)
		{
			Dashboard->SetInteractorWidget(InteractorWidget);
		}

		if (auto Choose = Cast<UCraftMenuChoose>(UIInvProvider->GetCraftChoose()))
		{
			Choose->SetCraftComponentPtr(ActiveCraftComponentRef);
		}

		UIInvProvider->OpenCraftDashboard();
	}
	else
	{
		UIInvProvider->CloseCraftMenu();

		if (auto Dashboard = Cast<UCraftDashboard>(UIInvProvider->GetCraftMenuDashboard()))
		{
			Dashboard->SetCraftComponentPtr(nullptr);
		}

		if (auto Choose = Cast<UCraftMenuChoose>(UIInvProvider->GetCraftChoose()))
		{
			Choose->SetCraftComponentPtr(nullptr);
		}

		//CachedCraftInputWidget = nullptr;
		//CachedCraftFuelWidget = nullptr;
		//CachedCraftOutputWidget = nullptr;
	}
}

void UIInventoryManager::Server_ClientClosedUI_Implementation(EInteractableType InteractableType)
{
	// Replicated removal may close an old window after a new interaction has opened.
	// The versioned InteractionComponent stop handles session cleanup.
	if (IsValid(TrackedInteractionTarget))
	{
		return;
	}
	if (LootContainerProvider.GetObject())
	{
		LootContainerProvider->CheckDestroyWhenEmpty();

		SetInteractionOwnership(Cast<AActor>(LootContainerProvider.GetObject()), false);

		LootContainerProvider.SetObject(nullptr);
		LootContainerProvider.SetInterface(nullptr);
	}
}

void UIInventoryManager::BindInteractionWidget()
{
	if (!InteractionComponent || !InteractionUIProvider)
	{
		return;
	}

	UInteractionWidget* InteractionWidget = InteractionUIProvider->GetPawnInteractionWidget();
	if (!InteractionWidget)
	{
		return;
	}

	InteractionComponent->OnBeginFocus.AddDynamic(InteractionWidget, &UInteractionWidget::OnFoundInteractable);
	InteractionComponent->OnEndFocus.AddDynamic(InteractionWidget, &UInteractionWidget::OnLostInteractable);
	InteractionComponent->OnInteractionProgress.AddDynamic(InteractionWidget, &UInteractionWidget::UpdateProgressBar);
	InteractionComponent->OnInteractionDisplayChanged.AddUniqueDynamic(
		InteractionWidget, &UInteractionWidget::OnInteractionDisplayChanged);
}

void UIInventoryManager::BindEvents()
{
	if (InteractionComponent)
	{
		InteractionComponent->OnInteract.AddUniqueDynamic(this, &UIInventoryManager::InteractRequest);
		InteractionComponent->OnStopInteract.AddUniqueDynamic(this, &UIInventoryManager::InteractClearRequest);

		BindInteractionWidget();
	}
}

void UIInventoryManager::BindInventoryEvents(UInventoryBase* Inventory)
{
	if (!Inventory)
	{
		return;
	}

	Inventory->OnSplitDelegate.RemoveDynamic(this, &UIInventoryManager::ItemSplitRequest);
	Inventory->OnSplitDelegate.AddDynamic(this, &UIInventoryManager::ItemSplitRequest);
}

void UIInventoryManager::BindWorldDropZoneEvents()
{
	if (!UIInvProvider)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIInventoryManager::InitInvWidgets: UIProvider is not set!"));
		return;
	}

	if (auto DropwWidget = UIInvProvider->GetWorldDropWidget())
	{
		DropwWidget->OnItemDroppedToWorld.AddDynamic(this, &UIInventoryManager::ItemDropRequest);
	}
}

void UIInventoryManager::OnQuickGrabPressed(const FInputActionInstance& Instance)
{
	InventoryModifierState.bIsQuickGrabModifierActive = true;
}

void UIInventoryManager::OnQuickGrabReleased(const FInputActionInstance& Instance)
{
	InventoryModifierState.bIsQuickGrabModifierActive = false;
}

void UIInventoryManager::OnGrabAllPressed(const FInputActionInstance& Instance)
{
	InventoryModifierState.bIsGrabAllSameModifierActive = true;
}

void UIInventoryManager::OnGrabAllReleased(const FInputActionInstance& Instance)
{
	InventoryModifierState.bIsGrabAllSameModifierActive = false;
}

void UIInventoryManager::RotateDraggedItem()
{
	if (UDragDropOperation* DragOp = UWidgetBlueprintLibrary::GetDragDroppingContent())
	{
		if (UItemDragDropOperation* ItemOp = Cast<UItemDragDropOperation>(DragOp))
		{
			ItemOp->RotateDraggedWidget();
		}
	}
}

void UIInventoryManager::HandleToggleInventory()
{
	if (StorageWindow)
	{
		CloseStorageAccess();
		return;
	}
	if (!UIInvProvider)
	{
		return;
	}

	UIInvProvider->ToggleInventoryLayout();
}

void UIInventoryManager::HandleToggleCraftMenu()
{
	if (StorageWindow)
	{
		CloseStorageAccess();
		return;
	}
	if (!UIInvProvider)
	{
		return;
	}
	if (CraftProvider.GetObject())
	{
		return;
	}
	if (!PawnCraftingComponentRef)
	{
		return;
	}

	UCraftingComponent* Previous = ActiveCraftComponentRef;
	ActiveCraftComponentRef = ActiveCraftComponentRef ? nullptr : PawnCraftingComponentRef;
}

void UIInventoryManager::ShowControlsHelp()
{
	auto* Pawn = Cast<APawn>(GetOwner());
	auto* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : Cast<APlayerController>(GetOwner());
	if (PC && PC->GetLocalPlayer())
	{
		PC->GetLocalPlayer()->GetSubsystem<UModalWindowManager>()->ShowControlsHelp();
	}
}

void UIInventoryManager::InitializeBindings()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!InputSubsystem)
	{
		return;
	}

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
	if (!Input)
	{
		return;
	}
	if (ControlsHelpInput.IsValid())
	{
		UInputUtility::RemoveBinding(ControlsHelpInput.Get(), ControlsHelpBindingHandle);
	}
	ControlsHelpInput = Input;
	if (GlobalSettings && GlobalSettings->ControlsHelpAction)
	{
		ControlsHelpBindingHandle = UInputUtility::BindAction(
			Input, GlobalSettings->ControlsHelpAction, ETriggerEvent::Started, this, &ThisClass::ShowControlsHelp);
	}

	if (UISettings.InventoryMappingContext)
	{
		InputSubsystem->AddMappingContext(UISettings.InventoryMappingContext, 1);
	}

	if (UISettings.ToggleInventoryAction)
	{
		Input->BindAction(UISettings.ToggleInventoryAction, ETriggerEvent::Started, this,
						  &UIInventoryManager::HandleToggleInventory);
		HandleToggleInventory();
		HandleToggleInventory();
	}
	if (UISettings.ToggleCraftAction)
	{
		Input->BindAction(UISettings.ToggleCraftAction, ETriggerEvent::Started, this,
						  &UIInventoryManager::HandleToggleCraftMenu);
	}

	if (UISettings.IA_Mod_QuickGrab)
	{
		Input->BindAction(UISettings.IA_Mod_QuickGrab, ETriggerEvent::Started, this,
						  &UIInventoryManager::OnQuickGrabPressed);
		Input->BindAction(UISettings.IA_Mod_QuickGrab, ETriggerEvent::Completed, this,
						  &UIInventoryManager::OnQuickGrabReleased);
	}
	if (UISettings.IA_Mod_GrabAllSame)
	{
		Input->BindAction(UISettings.IA_Mod_GrabAllSame, ETriggerEvent::Started, this,
						  &UIInventoryManager::OnGrabAllPressed);
		Input->BindAction(UISettings.IA_Mod_GrabAllSame, ETriggerEvent::Completed, this,
						  &UIInventoryManager::OnGrabAllReleased);
	}
	if (UISettings.IA_RotateDraggedItem)
	{
		Input->BindAction(UISettings.IA_RotateDraggedItem, ETriggerEvent::Started, this,
						  &UIInventoryManager::RotateDraggedItem);
	}

	BindInputActions();
}

void UIInventoryManager::BindInputActions()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (!Input)
	{
		return;
	}

	auto ActorInvs = ItemCollectionRef->GetActorInventories();
	if (ActorInvs.IsEmpty())
	{
		return;
	}

	for (UInventoryBase* Inventory : ActorInvs)
	{
		if (!Inventory)
		{
			continue;
		}

		USlotbasedInventory* SlotBased = Cast<USlotbasedInventory>(Inventory);
		if (!SlotBased)
		{
			continue;
		}

		if (SlotBased->GetInventorySlots().IsEmpty())
		{
			continue;
		}

		for (UInventorySlotData* SlotData : SlotBased->GetInventorySlots())
		{
			if (!SlotData)
			{
				continue;
			}
			if (!SlotData->InventorySlotInfo.UseAction)
			{
				continue;
			}

			UInputUtility::BindAction<UIInventoryManager, FString, FGuid>(
				Input, SlotData->InventorySlotInfo.UseAction.Get(), ETriggerEvent::Started, this,
				&UIInventoryManager::OnUseSlotInput, Inventory->GetInventoryContainerID(),
				SlotData->InventorySlotInfo.SlotGuid);
		}
	}
}

void UIInventoryManager::SetInteractionOwnership(AActor* TargetActor, bool bTake)
{
	if (!TargetActor || !TargetActor->HasAuthority())
	{
		return;
	}

	TargetActor->SetOwner(bTake && OwnerPawnRef ? OwnerPawnRef->GetController() : nullptr);
}

TMap<EObjectInteractionType, FModalActionConfig> UIInventoryManager::CollectAccessibleInventoryActions()
{
	TMap<EObjectInteractionType, FModalActionConfig> Result;
	auto* ContextActor = IsValid(PendingContextInv) ? PendingContextInv->GetInventoryOwnerActor() : nullptr;
	auto* ContextEquipment =
		IsValid(ContextActor) ? ContextActor->FindComponentByClass<UEquipmentComponent>() : nullptr;
	if (!GlobalSettings)
	{
		return Result;
	}
	for (const auto& Pair : GlobalSettings->InvContextModalActions)
	{
		const EObjectInteractionType Action = Pair.Key;

		bool bAllowed = true;

		switch (Action)
		{
		case EObjectInteractionType::Equip:
		{
			if (!ContextEquipment || ContextEquipment->IsItemEquipped(PendingContextItem))
			{
				bAllowed = false;
				break;
			}

			const FGameplayTag RequiredCategory =
				IObjectDataProvider::Execute_GetItemRef(PendingContextItem).ItemCategory;
			if (!RequiredCategory.IsValid() || !ContextEquipment->HasSlotForCategory(RequiredCategory))
			{
				bAllowed = false;
			}
		}
		break;
		case EObjectInteractionType::UnEquip:
		{
			if (!ContextEquipment || !ContextEquipment->IsItemEquipped(PendingContextItem))
			{
				bAllowed = false;
			}
			break;
		}
			/*case EObjectInteractionType::Sell:
			{
				ItemCollectionRef->IsItemOwnedByActor(PendingContextItem);
				break;
			}*/

		default:
			break;
		}

		if (bAllowed)
		{
			Result.Add(Pair);
		}
	}

	return Result;
}

void UIInventoryManager::OnInventoryModalResponse(FModalResult Result)
{
	if (!PendingContextItem || !PendingContextInv)
	{
		return;
	}

	switch (Result.ResultInteractionType)
	{
	case EObjectInteractionType::UseItem:
	{
		Execute_RequestUseSlot(this, PendingContextInv->GetInventoryContainerID(), PendingContextSlotGuid);
		break;
	}

	case EObjectInteractionType::Drop:
	{
		FItemDropData Data;
		Data.ItemToDrop = PendingContextItem;
		Data.SourceInventory = PendingContextInv;

		Data.DropAmount = IObjectDataProvider::Execute_GetQuantity(PendingContextItem);

		Execute_ItemDropRequest(this, Data);
		break;
	}

	case EObjectInteractionType::Destroy:
	{
		Execute_ItemDeleteRequest(this, PendingContextInv->GetInventoryContainerID(), PendingContextItem);
		break;
	}

	case EObjectInteractionType::Split:
	{
		Execute_ItemSplitRequest(this, PendingContextInv, PendingContextItem, Result.HeaderResult.SelectedAmount);
		break;
	}

	case EObjectInteractionType::Equip:
	{
		Server_EquipInventoryItem(PendingContextInv, PendingContextItem, false);
		break;
	}

	case EObjectInteractionType::UnEquip:
	{
		Server_EquipInventoryItem(PendingContextInv, PendingContextItem, true);
	}

	default:
		break;
	}

	PendingContextItem = nullptr;
	PendingContextInv = nullptr;
	PendingContextSlotGuid.Invalidate();
}

void UIInventoryManager::SubmitTrustedInventoryState(const FInventorySavedOwner& State)
{
	if (GetOwner()->HasAuthority())
	{
		Server_SubmitTrustedInventoryState_Implementation(State);
	}
	else
	{
		Server_SubmitTrustedInventoryState(State);
	}
}

void UIInventoryManager::Server_SubmitTrustedInventoryState_Implementation(const FInventorySavedOwner& State)
{
	FString Error;
	if (!GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->ApplyTrustedOwner(GetOwner(), State,
																								 Error))
	{
		UE_LOG(LogTemp, Warning, TEXT("Client inventory state rejected: %s"), *Error);
	}
}

void UIInventoryManager::QueueInventoryUIRefresh()
{
	if (!GetWorld() || bEndingPlay || GetWorld()->bIsTearingDown ||
		GetWorld()->GetTimerManager().IsTimerActive(ReplicatedInventoryUITimer))
	{
		return;
	}
	ReplicatedInventoryUITimer =
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::RefreshReplicatedInventoryUI);
}

void UIInventoryManager::RefreshReplicatedInventoryUI()
{
	if (!GetWorld() || bEndingPlay || GetWorld()->bIsTearingDown || !OwnerPawnRef ||
		!OwnerPawnRef->IsLocallyControlled() || !ItemCollectionRef || !UIInvProvider)
	{
		return;
	}
	InitInvWidgets();
	CreateWidgetsForInventories();
}
