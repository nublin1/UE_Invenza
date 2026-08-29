//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/UIInventoryManager.h"

#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Crafting/CraftingComponent.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "ActorComponents/Interactable/VendorComponent.h"
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
	Super::BeginPlay();
	
	OwnerPawnRef = Cast<APawn>(GetOwner());
	
	GlobalSettings = UInvenzayUtility::GetInvenzaGlobalSettings(GetWorld());
	
	FTimerHandle InitTimerHandle;

	GetWorld()->GetTimerManager().SetTimer(InitTimerHandle, FTimerDelegate::CreateLambda([this]()
	{
		InitializeInventoryManager();
	}), 0.2f, false);
}

void UIInventoryManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
}

void UIInventoryManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UIInventoryManager, InventoryWidgetInitMap);
	DOREPLIFETIME(UIInventoryManager, LootContainerProvider);
	DOREPLIFETIME(UIInventoryManager, VendorProviderCurrent);
}

void UIInventoryManager::InitializeInventoryManager()
{	
	auto Collection = GetOwner()->FindComponentByClass<UItemCollection>();
	if (!Collection)
		return;

	ItemCollectionRef = Collection;

	if (auto EquipmentComponent = GetOwner()->FindComponentByClass<UEquipmentComponent>())
	{
		EquipmentComponentRef = EquipmentComponent;
	}

	if (auto CraftingComponent = GetOwner()->FindComponentByClass<UCraftingComponent>())
	{
		CraftingComponentRef = CraftingComponent;
		
	}

	if (UInteractionComponent* InteractionComp = GetOwner()->FindComponentByClass<UInteractionComponent>())
	{
		InteractionComponent = InteractionComp;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeInventoryManager: InteractionComponent NOT FOUND on %s"),
		   *GetOwner()->GetName());
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
}

void UIInventoryManager::CreateInventories()
{
	if (!GetOwner()->HasAuthority())
		return;
	
	for (FInventoryStartupData& StartupData : StartupInventories)
	{
		UInventoryBase* Inventory = UInvenzayUtility::CreateStartupInventory(
			this,
			ItemCollectionRef,
			StartupData,
			StartingItems);

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
		return;

	for (auto& Inventory : InventoryWidgetInitMap)
	{
		if (ItemCollectionRef->HasContainerWidget(Inventory))
		{
			UE_LOG(LogTemp, Warning, TEXT("Widget for inventory %s already exists"), *GetNameSafe(Inventory));
			continue;
		}

		if (!CreateInventoryWidget(Inventory))
			continue;
	}
}

bool UIInventoryManager::CreateInventoryWidget(UInventoryBase* InvToLink)
{
	if (!InvToLink)
		return false;
	
	if (!OwnerPawnRef || !OwnerPawnRef->IsLocallyControlled()) return false;
	
	if (!UIInvProvider)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIInventoryManager::InitInvWidgets: UIProvider is not set!"));
		return false;
	}

	auto InvSettings = InvToLink->GetInventorySettings();

	APlayerController* PC = Cast<APlayerController>(OwnerPawnRef->GetController());
	if (!PC) return false;
	
	auto InvContainer = UInvenzaWidgetFactory::CreateConteinerInventoryWidget(
		PC,
		InvSettings.ContainerWidgetClass,
		InvSettings.InventoryWidgetClass,
		InvSettings.OperationPanelWidgetClass);

	if (!InvContainer || !InvToLink)
		return false;
	
	auto InvWidget = InvContainer->GetInventoryWidgetFromContainerSlot();
	InvWidget->SetInventoryBaseRef(InvToLink);
	InvWidget->SetUISettings(UISettings);
	InvWidget->InitializeInventoryWidgetWithSettings();

	InvContainer->InitializeInventoryBindings();

	InvWidget->OnItemDroppedDelegate.AddDynamic(this, &UIInventoryManager::ItemTransferRequest);

	UIInvProvider->AddPawnInvContainerWidget(InvContainer);

	ItemCollectionRef->RegisterContainerWidget(InvToLink, InvContainer);
	
	return true;
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
			continue;

		auto InvWidget = ContInvs->GetInventoryWidgetFromContainerSlot();

		if (!InvWidget->TargetInventoryTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("UIInventoryManager::InitInvWidgets: TargetInventoryTag is not set!"));
			continue;
		};
		

		auto TarInv = ItemCollectionRef->GetInventoryByTag(InvWidget->TargetInventoryTag);
		if (!TarInv)
			continue;

		if (USlotbasedInventoryWidget* SlotBased = Cast<USlotbasedInventoryWidget>(InvWidget))
		{
			SlotBased->SetInventoryBaseRef(TarInv);			
			SlotBased->SetUISettings(UISettings);
			InvWidget->InitializeInventoryWidget();

			ItemCollectionRef->Server_SetSlotBasedInventoryWidgetInitData(TarInv->GetInventoryContainerID(), SlotBased->CollectInitSlotsDataFromWidget());
			
			Execute_RebuildInventoryRequest(this, TarInv->GetInventoryContainerID());
			
		}
		else if (UListInventoryWidget* ListBased = Cast<UListInventoryWidget>(InvWidget))
		{
			InvWidget->InitializeInventoryWidget();
			
			auto TarListInv = Cast<UListInventory>(TarInv);
			if (!TarListInv)
				continue;
			
			ListBased->SetInventoryBaseRef(TarListInv);
			ListBased->SetUISettings(UISettings);
			ListBased->BindDelegated();
		}

		InvWidget->OnItemDroppedDelegate.AddDynamic(this, &UIInventoryManager::ItemTransferRequest);

		ItemCollectionRef->RegisterContainerWidget(TarInv, ContInvs);
	}

	for (auto ContainerBase : UIInvProvider->GetAllPawnInvContainers())
	{
		ContainerBase->InitializeInventoryBindings();
	}
}

void UIInventoryManager::InitCraftWidgets()
{
	if (!CraftingComponentRef || !UIInvProvider)
		return;
	
	APlayerController* PC = Cast<APlayerController>(OwnerPawnRef->GetController());
	if (!PC) return;

	if (UISettings.bCreateCraftWidgetsDynamically)
	{
		auto DashboardWidget = UInvenzaWidgetFactory::CreateInvenzaWidget(PC, UISettings.CraftDashboardClass );
		if (!DashboardWidget) return;

		UIInvProvider->AddPawnCraftDashboardWidget(DashboardWidget);

		auto CraftChooseWidget = UInvenzaWidgetFactory::CreateInvenzaWidget(PC, UISettings.CraftMenuChooseClass);
		if (!CraftChooseWidget) return;

		UIInvProvider->AddPawnCraftDashboardWidget(CraftChooseWidget);
	}

	UIInvProvider->BindCraftWidgets();
}

void UIInventoryManager::SetupStartingResources()
{
	if (!GetOwner()->HasAuthority())
		return;

	UInvenzayUtility::SetupStartingResources(this, StartingItems);
}

void UIInventoryManager::SetupAdditionalComponents()
{
	if (!CraftingComponentRef || !UIInvProvider)
		return;
	
	CraftingComponentRef->SetInputInventory(ItemCollectionRef->GetInventoryByTag(CraftingComponentRef->GetConfig().InputInventoryTag));
	CraftingComponentRef->SetOutputInventory(ItemCollectionRef->GetInventoryByTag(CraftingComponentRef->GetConfig().OutputInventoryTag));
	
	CraftingComponentRef->RequestInitCraftingComponent();
	
	if (auto CraftMenuDashboard = Cast<UCraftDashboard>(UIInvProvider->GetCraftMenuDashboard()))
	{
		CraftMenuDashboard->SetCraftComponentPtr(CraftingComponentRef);
	}

	if (auto CraftMenuChoose = Cast<UCraftMenuChoose>(UIInvProvider->GetCraftChoose()))
	{
		CraftMenuChoose->SetCraftComponentPtr(CraftingComponentRef);
	}
}

void UIInventoryManager::ItemContextMenuRequest_Implementation(const FString& FromInventory, FGuid SlotGuid, UObject* Item)
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
		 ModalManager =	PC->GetLocalPlayer()->GetSubsystem<UModalWindowManager>();
	}
	if (!ModalManager)
		return;

	FModalResultDelegate ResponseDelegate;
	ResponseDelegate.BindDynamic(this, &UIInventoryManager::OnInventoryModalResponse);
	
	FModalHeaderData HeaderData (EModalHeaderType::None, FText());
	ModalManager->OpenModalFlow(
		PendingContextItem,
		HeaderData,
		EModalFooterType::Selection,
		AllowedActions,
		ResponseDelegate
	);
}

UInventoryBase* UIInventoryManager::ResolveTargetInventory(UInventoryBase* SourceInventory)
{
	if (!SourceInventory)
		return nullptr;
	
	if (ItemCollectionRef->GetLinkedInventories().VendorInventory)
	{
		return (SourceInventory == ItemCollectionRef->GetLinkedInventories().VendorInventory)
			?(MainPawnInventoryRef.Get())
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
	UInventorySlotData*& OutSuitableSlot, UInventorySlotData*& OutFreeSlot)
{
	OutEquipmentInventory = nullptr;
	OutSuitableSlot = nullptr;
	OutFreeSlot = nullptr;

	if (!ItemCollectionRef || !Item || !EquipmentComponentRef || !GlobalSettings)
		return false;

	const FGameplayTag RequiredCategory = IObjectDataProvider::Execute_GetItemRef(Item).ItemCategory;
	if (!RequiredCategory.IsValid())
		return false;

	for (UInventoryBase* EquipmentInventory : ItemCollectionRef->GetAllInventoriesByTag(GlobalSettings->EquipmentInvTag))
	{
		if (!EquipmentInventory)
			continue;

		for (const FGuid& SlotID : EquipmentInventory->GetSlotsWithLinkedEquipment())
		{
			UInventorySlotData* Slot = EquipmentInventory->GetSlotByGuid(SlotID);
			if (!Slot)
				continue;

			const FGameplayTag& AllowedCategory = Slot->InventorySlotInfo.AllowedCategory;
			const FGameplayTag& EquipmentSlotTag = Slot->InventorySlotInfo.LinkedEquipmentSlot;

			if (!AllowedCategory.IsValid()
				|| !EquipmentSlotTag.IsValid()
				|| !AllowedCategory.MatchesTagExact(RequiredCategory)
				|| !EquipmentComponentRef->DoesSlotExist(EquipmentSlotTag)
				|| !EquipmentComponentRef->CanEquipItemToSlot(Item, EquipmentSlotTag))
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
		return;
	
	FItemMoveData ItemMoveData = InData;

	if (ItemMoveData.SourceInventory == ItemMoveData.TargetInventory)
		return;
	
	ItemMoveData.TargetInventory = ResolveTargetInventory(ItemMoveData.SourceInventory);

	if (!ItemMoveData.TargetInventory)
		return;

	Execute_ItemTransferRequest(this,ItemMoveData);
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
	
	auto SameItems = InData.SourceInventory->GetItemCollectionLinked()->GetAllSameItemsInContainerByItemSample(InvID, InData.SourceItem);
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
		return;

	const FString SourceContainerID = SourceInventory->GetInventoryContainerID();
	TArray<UObject*> Items = SourceInventory->GetItemCollectionLinked()->GetAllItemsByContainer(SourceContainerID);

	if (Items.IsEmpty())
		return;

	for (UObject* Item : Items)
	{
		if (!Item)
			continue;

		FItemMoveData MoveData;

		MoveData.SourceItem = Item;
		MoveData.SourceInventory = SourceInventory;
		MoveData.TargetInventory = TargetInventory;

		Execute_ItemTransferRequest(this,MoveData);
	}
	
}

void UIInventoryManager::VendorRequest(FItemMoveData ItemMoveData )
{
	if (ItemMoveData.SourceInventory == ItemMoveData.TargetInventory)
		return;
	
	FTradeResult TradeResult = VendorProviderCurrent->ProcessTradeRequest(ItemMoveData);
	const UEnum* TradeResultEnum = StaticEnum<ETradeResult>();
	const FString OperationResultName = TradeResultEnum
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
	);	
	
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
	if (VendorProviderCurrent)
	{
		if (ItemMoveData.TargetInventory == VendorProviderCurrent->GetVendorLootContainer()
			|| ItemMoveData.SourceInventory == VendorProviderCurrent->GetVendorLootContainer())
			VendorRequest(ItemMoveData);
		return;
	}
	
	FItemAddResult Result = ItemMoveData.TargetInventory->HandleAddItem(ItemMoveData, false);
	auto ActualAmountAdded = Result.ActualAmountAdded;
	//UE_LOG(LogTemp, Log, TEXT("InventoryManager::ItemTransferRequest. Is ResultMessage: %s"), *Result.ResultMessage.ToString());

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
			// EQUIPMENT INVENTORY
			UnequipItemIfNeeded(ItemMoveData.SourceInventory, ItemMoveData.SourceSlotID);
			
			const EInventoryContextActionResult ContextResult =	ValidateAndEquipTransferredItem(ItemMoveData);
			if (ContextResult == EInventoryContextActionResult::EquipmentSlotNotFound)
			{
				// Equipment validation failed.
				// The item has already been transferred, so move it back to the source inventory

				ItemMoveData.TargetInventory->HandleRemoveItem(ItemMoveData.SourceItem, ActualAmountAdded);
				
				FItemMoveData ReverseMoveData;
				ReverseMoveData.SourceInventory = ItemMoveData.TargetInventory;
				ReverseMoveData.SourceItem = ItemMoveData.SourceItem;
				ReverseMoveData.TargetInventory = ItemMoveData.SourceInventory;

				Execute_ItemTransferRequest(this,ReverseMoveData);
				
			}
		
			/*if (bNeedUIUpdate)
			{
				ItemCollectionRef->NotifyUI_ItemChanged(
					ItemMoveData.SourceItem,
					ItemMoveData.TargetInventory->GetInventoryContainerID(),
					EInventoryActionType::Added);
	
				if (bIsSourceInvExist)
				{
					ItemCollectionRef->NotifyUI_ItemChanged(
					ItemMoveData.SourceItem,
					ItemMoveData.SourceInventory->GetInventoryContainerID(),
					EInventoryActionType::Removed);
				}
			}*/
		}
		break;
	case EItemAddResult::IAR_NoItemAdded:
		if (ItemMoveData.SourceInventory == nullptr)
		{
			const int32 Quantity =
				IObjectDataProvider::Execute_GetQuantity(ItemMoveData.SourceItem);

			FItemDropData DropData(
				ItemMoveData.SourceItem,
				ItemMoveData.SourceInventory,
				Quantity
			);

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
		if (!Result.bIsUsedReferences
			&& ItemMoveData.SourceInventory->GetInventorySettings().bAllowItemReferencing
			&& ItemMoveData.SourceInventory != ItemMoveData.TargetInventory)
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
}

EInventoryContextActionResult UIInventoryManager::ValidateAndEquipTransferredItem(FItemMoveData& ItemMoveData)
{
	if (!EquipmentComponentRef || !ItemCollectionRef || !GlobalSettings || !ItemMoveData.TargetInventory)
		return EInventoryContextActionResult::NotApplicable;

	const auto EquipmentInventories =
		ItemCollectionRef->GetAllInventoriesByTag(
			GlobalSettings->EquipmentInvTag);

	if (EquipmentInventories.IsEmpty() || !EquipmentInventories.Contains(ItemMoveData.TargetInventory))
		return EInventoryContextActionResult::NotApplicable;

	if (!ItemMoveData.SourceItem || !ItemMoveData.TargetSlotID.IsValid())
		return EInventoryContextActionResult::InvalidSlot;

	UInventorySlotData* TargetSlot = ItemMoveData.TargetInventory->GetSlotByGuid(ItemMoveData.TargetSlotID);
	if (!TargetSlot)
		return EInventoryContextActionResult::InvalidSlot;

	const FGameplayTag EquipmentSlotTag = TargetSlot->InventorySlotInfo.LinkedEquipmentSlot;

	if (!EquipmentSlotTag.IsValid())
		return EInventoryContextActionResult::InvalidSlot;

	if (!EquipmentComponentRef->DoesSlotExist(
		EquipmentSlotTag))
	{
		return EInventoryContextActionResult::EquipmentSlotNotFound;
	}

	if (EquipmentComponentRef->IsItemEquippedInSlot(
		ItemMoveData.SourceItem,
		EquipmentSlotTag))
	{
		return EInventoryContextActionResult::ItemAlreadyEquipped;
	}

	const FGameplayTag ItemCategory =
		IObjectDataProvider::Execute_GetItemRef(
			ItemMoveData.SourceItem).ItemCategory;

	if (!EquipmentComponentRef->IsCategoryCompatibleWithSlot(
		EquipmentSlotTag,
		ItemCategory))
	{
		return EInventoryContextActionResult::IncompatibleSlot;
	}

	if (EquipmentComponentRef->IsSlotOccupied(
		EquipmentSlotTag))
	{
		return EInventoryContextActionResult::SlotOccupied;
	}

	EquipmentComponentRef->Server_EquipItemToSlot(
		EquipmentSlotTag,
		ItemMoveData.SourceItem);

	return EInventoryContextActionResult::Success;
}

void UIInventoryManager::UnequipItemIfNeeded(UInventoryBase* SourceInventory, FGuid SourceSlotID)
{
	if (!SourceInventory || !SourceSlotID.IsValid())
		return;
	if (!EquipmentComponentRef || !ItemCollectionRef || !GlobalSettings)
		return;

	const auto EquipmentInventories =	ItemCollectionRef->GetAllInventoriesByTag(GlobalSettings->EquipmentInvTag);
	if (!EquipmentInventories.Contains(SourceInventory))
		return;
	
	UInventorySlotData* Slot = SourceInventory->GetSlotByGuid(SourceSlotID);
	if (!Slot)
		return;

	EquipmentComponentRef->Server_UnequipItemFromSlot(Slot->InventorySlotInfo.LinkedEquipmentSlot);
}

void UIInventoryManager::ItemSplitRequest_Implementation(UInventoryBase* TargetInventory, UObject* ItemToSplit, int32 SplitAmount)
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

void UIInventoryManager::Server_ItemSplitRequest_Implementation(UInventoryBase* TargetInventory, UObject* ItemToSplit, int32 SplitAmount)
{
	Handle_SplitItem(TargetInventory, ItemToSplit, SplitAmount);
}

void UIInventoryManager::Handle_SplitItem(UInventoryBase* TargetInventory, UObject* ItemToSplit, int32 SplitAmount)
{
	if (!TargetInventory || !ItemToSplit || SplitAmount <= 0)
		return;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemToSplit, TEXT("Handle_SplitItem")))
		return;

	const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(ItemToSplit);

	if (Quantity == 1 || Quantity <= SplitAmount)
		return;

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
			return;
	}

	UObject* NewItem = IObjectDataProvider::Execute_DuplicateItem(ItemToSplit);
	if (!NewItem ||
		!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(NewItem, TEXT("Handle_SplitItem")))
		return;

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
	if (DropData.DropAmount <= 0 || !DropData.ItemToDrop)
		return;

	const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(DropData.ItemToDrop);

	if (!Meta.bIsDroppable)
		return;

	if (auto Pawn = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPawn())
	{
		FVector SpawnLocation = Pawn->GetActorLocation() + Pawn->GetActorForwardVector() * 50.f;
		FRotator SpawnRotation = Pawn->GetActorRotation();

		const FDataTableRowHandle ItemRow =	IObjectDataProvider::Execute_GetItemRow(DropData.ItemToDrop);

		UInvenzayUtility::DropItem(
			GetWorld(),
			Pawn,
			ItemRow,
			DropData.DropAmount,
			SpawnLocation,
			SpawnRotation
		);
		
		ItemDeleteRequest_Implementation(DropData.SourceInventory->GetInventoryContainerID(), DropData.ItemToDrop);
	}
}

void UIInventoryManager::ItemDeleteRequest_Implementation(const FString& FromInventory, UObject* Item)
{
	Server_OnItemDelete(FromInventory, Item);
}

void UIInventoryManager::Server_OnItemDelete_Implementation(const FString& FromInventory, UObject* Item)
{
	if (!ItemCollectionRef)
		return;
	
	ItemCollectionRef->RemoveItemFromAllContainers(Item);
}

void UIInventoryManager::ItemEquipRequest_Implementation(UObject* Item)
{
	if (!ItemCollectionRef || !Item || !EquipmentComponentRef)
		return;
	
	Server_ItemEquipRequest(Item);
}

void UIInventoryManager::Server_ItemEquipRequest_Implementation(UObject* Item)
{
	auto MapData = ItemCollectionRef->FindMainInventoryMappingForItem(Item);
	if (!MapData)
		return;
	
	UInventoryBase* CurrentInv = ItemCollectionRef->GetInventoryByID(MapData->InventoryID);
	if (!CurrentInv)
		return;

	UInventoryBase* TargetEquipmentInventory = nullptr;
	UInventorySlotData* FirstSuitableSlot = nullptr;
	UInventorySlotData* FirstFreeSuitableSlot = nullptr;

	if (!FindSuitableEquipmentSlot(	Item,TargetEquipmentInventory,FirstSuitableSlot,FirstFreeSuitableSlot))
	{
		return;
	}

	UInventorySlotData* TargetSlot = FirstFreeSuitableSlot ? FirstFreeSuitableSlot : FirstSuitableSlot;
	if (!TargetSlot)
		return;

	const FGuid TargetSlotID = TargetSlot->InventorySlotInfo.SlotGuid;
	const FGameplayTag EquipmentSlotTag = TargetSlot->InventorySlotInfo.LinkedEquipmentSlot;

	if (!EquipmentSlotTag.IsValid())
		return;

	FItemMoveData MoveData;
	MoveData.SourceInventory = CurrentInv;
	MoveData.SourceItem = Item;
	MoveData.TargetInventory = TargetEquipmentInventory;
	MoveData.TargetSlotID = TargetSlotID;

	if (FirstFreeSuitableSlot)
	{
		Execute_ItemTransferRequest(this, MoveData);
		return;
	}

	UObject* ItemInTargetSlot = ItemCollectionRef->GetItemFromSlot(
		TargetSlotID,
		TargetEquipmentInventory->GetInventoryContainerID());

	if (!ItemInTargetSlot)
		return;

	const int32 EquippedItemQuantity = IObjectDataProvider::Execute_GetQuantity(ItemInTargetSlot);
	if (TargetEquipmentInventory->GetInventorySettings().bIsReferenceContainer)
	{
		TargetEquipmentInventory->HandleRemoveItem(
			ItemInTargetSlot,
			EquippedItemQuantity);

		Execute_ItemTransferRequest(this, MoveData);

		return;
	}

	const FDataTableRowHandle EquippedItemRow =	IObjectDataProvider::Execute_GetItemRow(ItemInTargetSlot);
	TargetEquipmentInventory->HandleRemoveItem(ItemInTargetSlot, EquippedItemQuantity);

	Execute_ItemTransferRequest(this, MoveData);

	FInitItemsEntry ReturnedItemEntry;
	ReturnedItemEntry.Item = EquippedItemRow;
	ReturnedItemEntry.Amount = EquippedItemQuantity;

	UInvenzayUtility::AddItemQuantity(
		this,
		CurrentInv,
		ReturnedItemEntry);
}

void UIInventoryManager::ItemUnequipRequest_Implementation(UObject* Item)
{
	if (!ItemCollectionRef || !Item || !EquipmentComponentRef)
		return;
	
	Server_ItemUnequipRequest(Item);
}

void UIInventoryManager::Server_ItemUnequipRequest_Implementation(UObject* Item)
{
	if (!Item)
		return;
	
	if (!MainPawnInventoryRef)
		return;
	
	auto MapData = ItemCollectionRef->FindMainInventoryMappingForItem(Item);
	if (!MapData)
		return;
	
	UInventoryBase* CurrentInv = ItemCollectionRef->GetInventoryByID(MapData->InventoryID);
	if (!CurrentInv)
		return;
	
	FItemMoveData MoveData;

	MoveData.SourceInventory = CurrentInv;
	MoveData.SourceItem = Item;
	MoveData.SourceSlotID = MapData->OccupiedSlots[0]->InventorySlotInfo.SlotGuid;
	MoveData.TargetInventory = MainPawnInventoryRef;
	
	Execute_ItemTransferRequest(this, MoveData);
}

void UIInventoryManager::RequestUseSlot_Implementation( const FString& InvID, FGuid SlotID)
{
	Server_RequestUseSlot(InvID, SlotID);
}

void UIInventoryManager::Server_RequestUseSlot_Implementation(const FString& InvID, FGuid SlotID)
{
	if (InvID.IsEmpty())
		return;
	
	UInventoryBase* Inventory = ItemCollectionRef->GetInventoryByID(InvID);
	if (!Inventory)
		return;

	UInventorySlotData* Slot = Inventory->GetSlotByGuid(SlotID);
	if (!Slot)
		return;
	
	Inventory->UseSlot(Slot);
}

void UIInventoryManager::OnUseSlotInput(FString InvID, FGuid SlotID)
{
	RequestUseSlot(InvID, SlotID);
}

void UIInventoryManager::RebuildInventoryRequest_Implementation(const FString& InvID)
{
	if (!GetOwner()->HasAuthority())
		Server_RebuildInventory(InvID);
	else
		HandleRebuildInventory(InvID);
}

void UIInventoryManager::Server_RebuildInventory_Implementation(const FString& InvID)
{
	HandleRebuildInventory(InvID);
}

void UIInventoryManager::HandleRebuildInventory(const FString& InvID)
{
	if (!ItemCollectionRef)
		return;
	
	auto FindResult = ItemCollectionRef->GetInventoryByID(InvID);
	if (!FindResult)
		return;

	FindResult->RebuildInventory();
}

void UIInventoryManager::InteractRequest(UInteractableComponent* TargetInteractableComponent)
{
	if (!TargetInteractableComponent) return;
	
	if (OwnerPawnRef && OwnerPawnRef->IsLocallyControlled())
	{
		Server_HandleInteract(TargetInteractableComponent);
	}
}

void UIInventoryManager::Server_HandleInteract_Implementation(UInteractableComponent* Target)
{
	if (!Target)
	{
		return;
	}
	
	auto InteractableData = Target->GetInteractableData();
	switch (InteractableData.DefaultInteractableType)
	{
	case EInteractableType::Pickup:
		{
			HandlePickupInteraction(Target);
			break;
		}
	case EInteractableType::Container:
		{
			HandleContainerInteraction(Target);
			break;
		}
	case EInteractableType::Vendor:
		{
			HandleTradeInteraction(Target);
			break;
		}
	default: 
		break;
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
	ILootContainerProvider* LootProvider = Cast<ILootContainerProvider>(Target);
	if (!LootProvider)
	{
		return;
	}

	UInventoryBase* InventoryToDisplay =	LootProvider->GetMainLootContainer();
	if (!InventoryToDisplay)
	{
		return;
	}
	
	ItemCollectionRef->SetExternalInventory(InventoryToDisplay);

	LootContainerProvider.SetObject(Target->GetOwner());
	LootContainerProvider.SetInterface(LootProvider);

	OpenExternalInventory(InventoryToDisplay);
}

void UIInventoryManager::HandleTradeInteraction(UInteractableComponent* Target)
{
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
	
	ItemCollectionRef->SetVendorInventory(InventoryToDisplay);

	VendorProviderCurrent.SetObject(Target);
	VendorProviderCurrent.SetInterface(VendorProvider);
			
	FTradeContext TradeContext;
	TradeContext.TradeSettings = VendorProviderCurrent->GetTradeSettings();
	TradeContext.Vendor = Cast<AActor>(VendorProviderCurrent.GetObject());
	TradeContext.Buyer = this->GetOwner();

	ItemCollectionRef->GetLinkedInventories().VendorInventory->SetTradeContext(TradeContext);
			
	VendorProviderCurrent->SetTradePartnerInventory(MainPawnInventoryRef);
	VendorProviderCurrent->SetTradePartnerItemCollection(ItemCollectionRef);
		
	OpenVendorInventory(InventoryToDisplay);
}

void UIInventoryManager::InteractClearRequest(UInteractableComponent* TargetInteractableComponent)
{
	if (!TargetInteractableComponent) return;
	if (GetOwner()->HasAuthority())
	{
		Server_HandleClearInteract_Implementation(TargetInteractableComponent);
	}
	else
	{
		Server_HandleClearInteract(TargetInteractableComponent);
	}
}

void UIInventoryManager::Server_HandleClearInteract_Implementation(UInteractableComponent* Target)
{
	HandleClearInteraction(Target);
}

void UIInventoryManager::HandleClearInteraction(UInteractableComponent* TargetInteractableComponent)
{
	if (!ItemCollectionRef)
	{
		return;
	}

	auto LinkedInventories = ItemCollectionRef->GetLinkedInventories();
	if (LinkedInventories.VendorInventory)
	{
		CloseVendorInventory(LinkedInventories.VendorInventory);
		return;
	}

	if (LinkedInventories.ExternalInventory)
	{
		CloseExternalInventory(LinkedInventories.ExternalInventory);
		return;
	}
}

void UIInventoryManager::OpenVendorInventory(UInventoryBase* Inv)
{
	CreateInventoryWidget(Inv);
	HandleToggleInventory();
}

void UIInventoryManager::CloseVendorInventory(UInventoryBase* Inv)
{
	if (!Inv || !ItemCollectionRef)
	{
		return;
	}
	
	auto FindResult = ItemCollectionRef->GetContainerWidget(Inv);
	if (FindResult)
	{
		UIInvProvider->RemovePawnInvContainer(FindResult);
		ItemCollectionRef->UnregisterContainerWidget(Inv);
	}
	
	ItemCollectionRef->SetVendorInventory(nullptr);
	HandleToggleInventory();
	VendorProviderCurrent = nullptr;
}

void UIInventoryManager::OpenExternalInventory(UInventoryBase* Inv)
{
	CreateInventoryWidget(Inv);
	HandleToggleInventory();
}

void UIInventoryManager::CloseExternalInventory(UInventoryBase* Inv)
{
	if (!Inv || !ItemCollectionRef)
	{
		return;
	}
	
	auto FindResult = ItemCollectionRef->GetContainerWidget(Inv);
	if (FindResult)
	{
		UIInvProvider->RemovePawnInvContainer(FindResult);
		ItemCollectionRef->UnregisterContainerWidget(Inv);
	}
	
	ItemCollectionRef->SetExternalInventory(nullptr);
	HandleToggleInventory();

	LootContainerProvider.SetObject(nullptr);
	LootContainerProvider.SetInterface(nullptr);
}

void UIInventoryManager::BindInteractionWidget()
{
	if (!InteractionComponent || !InteractionUIProvider)
		return;

	UInteractionWidget* InteractionWidget = InteractionUIProvider->GetPawnInteractionWidget();
	if (!InteractionWidget)
		return;
	
	InteractionComponent->OnBeginFocus.AddDynamic(InteractionWidget, &UInteractionWidget::OnFoundInteractable);
	InteractionComponent->OnEndFocus.AddDynamic(InteractionWidget, &UInteractionWidget::OnLostInteractable);
	InteractionComponent->OnInteractionProgress.AddDynamic(InteractionWidget, &UInteractionWidget::UpdateProgressBar);
}

void UIInventoryManager::BindEvents()
{
	if (InteractionComponent)
	{
		InteractionComponent->OnInteract.AddDynamic(this, &UIInventoryManager::InteractRequest);
		InteractionComponent->OnStopInteract.AddDynamic(this, &UIInventoryManager::InteractClearRequest);
		
		BindInteractionWidget();
		
	}		
}

void UIInventoryManager::BindInventoryEvents(UInventoryBase* Inventory)
{
	if (!Inventory) return;

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
	if (!UIInvProvider)
		return;
	
	UIInvProvider->ToggleInventoryLayout();
}

void UIInventoryManager::HandleToggleCraftMenu()
{
	if (!UIInvProvider)
		return;

	UIInvProvider->ToggleCraftMenuLayout();
}

void UIInventoryManager::InitializeBindings()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController) return;

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!InputSubsystem) return;

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
	if (!Input) return;
	
	if (UISettings.InventoryMappingContext)
	{
		InputSubsystem->AddMappingContext(UISettings.InventoryMappingContext, 1);
	}
	
	if (UISettings.ToggleInventoryAction)
	{
		Input->BindAction(UISettings.ToggleInventoryAction, ETriggerEvent::Started, this, &UIInventoryManager::HandleToggleInventory);
		HandleToggleInventory();
		HandleToggleInventory();
	}
	if (UISettings.ToggleCraftAction)
	{
		Input->BindAction(UISettings.ToggleCraftAction, ETriggerEvent::Started, this, &UIInventoryManager::HandleToggleCraftMenu);
	}
	

	if (UISettings.IA_Mod_QuickGrab)
	{
		Input->BindAction(UISettings.IA_Mod_QuickGrab, ETriggerEvent::Started, this, &UIInventoryManager::OnQuickGrabPressed);
		Input->BindAction(UISettings.IA_Mod_QuickGrab, ETriggerEvent::Completed, this, &UIInventoryManager::OnQuickGrabReleased);
	}
	if (UISettings.IA_Mod_GrabAllSame)
	{
		Input->BindAction(UISettings.IA_Mod_GrabAllSame, ETriggerEvent::Started, this, &UIInventoryManager::OnGrabAllPressed);
		Input->BindAction(UISettings.IA_Mod_GrabAllSame, ETriggerEvent::Completed, this, &UIInventoryManager::OnGrabAllReleased);
	}
	if (UISettings.IA_RotateDraggedItem)
	{
		Input->BindAction(UISettings.IA_RotateDraggedItem, ETriggerEvent::Started, this, &UIInventoryManager::RotateDraggedItem);
	}

	BindInputActions();
}

void UIInventoryManager::BindInputActions()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (!Input) return;

	auto ActorInvs = ItemCollectionRef->GetActorInventories();
	if (ActorInvs.IsEmpty()) return;

	for (UInventoryBase* Inventory : ActorInvs)
	{
		if (!Inventory) continue;

		USlotbasedInventory* SlotBased = Cast<USlotbasedInventory>(Inventory);
		if (!SlotBased) continue;

		if (SlotBased->GetInventorySlots().IsEmpty()) continue;

		for (UInventorySlotData* SlotData : SlotBased->GetInventorySlots())
		{
			if (!SlotData) continue;
			if (!SlotData->InventorySlotInfo.UseAction) continue;
			
			UInputUtility::BindAction<UIInventoryManager, FString, FGuid>(
				Input,
				SlotData->InventorySlotInfo.UseAction.Get(),
				ETriggerEvent::Started,
				this,
				&UIInventoryManager::OnUseSlotInput,
				Inventory->GetInventoryContainerID(),
				SlotData->InventorySlotInfo.SlotGuid
			);
		}
	}
}

void UIInventoryManager::ValidateInventoryContextActions()
{
	
}

TMap<EObjectInteractionType, FModalActionConfig> UIInventoryManager::CollectAccessibleInventoryActions()
{
	TMap<EObjectInteractionType, FModalActionConfig> Result;
	
	for (const auto& Pair : GlobalSettings->InvContextModalActions)
	{
		const EObjectInteractionType Action = Pair.Key;
		
		bool bAllowed = true;
		
		switch (Action)
		{
		case EObjectInteractionType::Equip:
			{
				if (!EquipmentComponentRef || EquipmentComponentRef->IsItemEquipped(PendingContextItem))
				{
					bAllowed = false;
					break;
				}

				const FGameplayTag RequiredCategory = IObjectDataProvider::Execute_GetItemRef(PendingContextItem).ItemCategory;
				if (!RequiredCategory.IsValid()
					|| !EquipmentComponentRef->HasSlotForCategory(RequiredCategory))
				{
					bAllowed = false;
				}
			}
			break;
		case EObjectInteractionType::UnEquip:
			{
				if (!EquipmentComponentRef || !EquipmentComponentRef->IsItemEquipped(PendingContextItem))
					bAllowed = false;
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
		return;

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
			Execute_ItemEquipRequest(this, PendingContextItem);
			break;
		}
		
	case EObjectInteractionType::UnEquip:
		{
			Execute_ItemUnequipRequest(this, PendingContextItem);
		}

	default:
		break;
	}

	PendingContextItem = nullptr;
	PendingContextInv = nullptr;
	PendingContextSlotGuid.Invalidate();
}
