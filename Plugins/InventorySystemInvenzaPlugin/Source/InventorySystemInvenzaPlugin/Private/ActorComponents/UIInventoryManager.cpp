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
#include "Data/Items/itemBase.h"
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
	InitializeInventoryManager();
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

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn && OwnerPawn->IsLocallyControlled())
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
	
	auto GlobalSettings = UInvenzayUtility::GetInvenzaGlobalSettings(GetWorld());
	
	for (FInventoryStartupData& StartupData : StartupInventories)
	{
		UInventoryBase* Inventory =	UInventoryBase::CreateInventoryAdvanced(GetOwner(), StartupData, GetOwner(), ItemCollectionRef);
		if (!Inventory)
			continue;
		
		StartingItems.Add(Inventory, StartupData.StartItems);

		ItemCollectionRef->AddPawnInventory_Internal(Inventory);
		BindInventoryEvents(Inventory);

		if (StartupData.Settings.InventoryTag == GlobalSettings->MainInvTag)
		{
			MainPawnInventory = Inventory;
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

		if (!CreateWidget(Inventory))
			continue;
	}
}

bool UIInventoryManager::CreateWidget(UInventoryBase* InvToLink)
{
	if (!InvToLink)
		return false;
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) return false;
	
	if (!UIInvProvider)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIInventoryManager::InitInvWidgets: UIProvider is not set!"));
		return false;
	}

	auto InvSettings = InvToLink->GetInventorySettings();

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return false;
	
	auto InvContainer =  UInvenzaWidgetFactory::CreateInventoryWidget(
		PC,
		InvSettings.ContainerWidgetClass,
		InvSettings.InventoryWidgetClass,
		nullptr);

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

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
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

void UIInventoryManager::SetupStartingResources()
{
	if (!GetOwner()->HasAuthority())
		return;

	if (StartingItems.IsEmpty())
		return;
	
	for (auto& [TargetInventory, InitItems] : StartingItems)
	{
		if (!TargetInventory 
			|| TargetInventory->GetInventoryContainerID().IsEmpty() 
			|| InitItems.IsEmpty() 
			|| !TargetInventory->GetItemCollectionLinked())
		{
			continue;
		}

		for (const auto& InitResource : InitItems)
		{
			if (InitResource.Item.RowName.IsNone()) continue;
			//UObject* NewItemSample = UItemFactory::CreateItemByHandle(this, InitResource.Item, 1);

			UInvenzayUtility::AddItemQuantity(this, TargetInventory,InitResource);
		}
	}

	StartingItems.Empty();
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
	auto AllowedActions = UInvenzayUtility::CollectAccessibleObjectActions(GetWorld(), Item);
	if (AllowedActions.Num() == 0) return;

	UModalWindowManager* ModalManager = nullptr;
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		 ModalManager =	PC->GetLocalPlayer()->GetSubsystem<UModalWindowManager>();
	}
	
	if (!ModalManager)
		return;
	
	PendingContextItem = Item;
	PendingContextInv = ItemCollectionRef->GetInventoryByID(FromInventory);
	PendingContextSlotGuid = SlotGuid;

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

void UIInventoryManager::OnItemAddedToInventory(FItemMapping& ItemSlots, UObject* Item)
{
	if (!Item || !EquipmentComponentRef) return;

	for (UInventorySlotData* SlotData : ItemSlots.OccupiedSlots)
	{
		if (!SlotData) continue;
		if (!SlotData->InventorySlotInfo.LinkedEquipmentSlot.IsValid()) continue;
		
		EquipmentComponentRef->Server_EquipItemToSlot(SlotData->InventorySlotInfo.LinkedEquipmentSlot, Item);
		return;
	}
}

void UIInventoryManager::OnItemRemovedFromInventory(FItemMapping ItemSlots, UObject* Item)
{
	if (!Item || !EquipmentComponentRef) return;

	for (UInventorySlotData* SlotData : ItemSlots.OccupiedSlots)
	{
		if (!SlotData) continue;
		if (!SlotData->InventorySlotInfo.LinkedEquipmentSlot.IsValid()) continue;

		EquipmentComponentRef->Server_UnequipItemFromSlot(SlotData->InventorySlotInfo.LinkedEquipmentSlot);
		return;
	}
}

UInventoryBase* UIInventoryManager::ResolveTargetInventory(UInventoryBase* SourceInventory)
{
	if (!SourceInventory)
		return nullptr;
	
	if (ItemCollectionRef->GetLinkedInventories().VendorInventory)
	{
		return (SourceInventory == ItemCollectionRef->GetLinkedInventories().VendorInventory)
			?(MainPawnInventory.Get())
			: ItemCollectionRef->GetLinkedInventories().VendorInventory.Get();
	}
	
	if (ItemCollectionRef->GetLinkedInventories().ExternalInventory)
	{
		return (SourceInventory == ItemCollectionRef->GetLinkedInventories().ExternalInventory)
			? (MainPawnInventory.Get())
			: ItemCollectionRef->GetLinkedInventories().ExternalInventory.Get();
	}
	
	return MainPawnInventory;
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
	if (!MainPawnInventory)
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

void UIInventoryManager::VendorRequest(FItemMoveData ItemMoveData )
{
	if (ItemMoveData.SourceInventory == ItemMoveData.TargetInventory)
		return;
	
	FTradeResult TradeResult = VendorProviderCurrent->ProcessTradeRequest(ItemMoveData);
		
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
		if (ItemMoveData.SourceInventory && ItemMoveData.SourceInventory->GetItemCollectionLinked())
		{
			if (!ItemMoveData.TargetInventory->GetInventorySettings().bIsReferenceContainer)
			{
				ItemMoveData.SourceInventory->HandleRemoveItem(ItemMoveData.SourceItem, ActualAmountAdded);
			}
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

		const FDataTableRowHandle ItemRow =
			IObjectDataProvider::Execute_GetItemRow(DropData.ItemToDrop);

		UInvenzayUtility::DropItem(
			GetWorld(),
			Pawn,
			ItemRow,
			DropData.DropAmount,
			SpawnLocation,
			SpawnRotation
		);

		DropData.SourceInventory->HandleRemoveItem(
			DropData.ItemToDrop,
			DropData.DropAmount
		);
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
	if (!ItemCollectionRef)
		return;
	
	if (!Item)
		return;
	
	if (!EquipmentComponentRef)
		return;
	
	auto GlobalSettings = UInvenzayUtility::GetInvenzaGlobalSettings(GetWorld());
	
	auto EquipmentInventories = ItemCollectionRef->GetAllInventoriesByTag(GlobalSettings->EquipmentInvTag);
	if (EquipmentInventories.IsEmpty())
		return;
	
	auto CurrentInv = ItemCollectionRef->FindMainInventoryForItem(Item);
	if (CurrentInv == nullptr)
		return;
	
	const FGameplayTag RequiredEquipmentSlot =
	   IObjectDataProvider::Execute_GetItemRef(Item).ItemCategory;
	UInventorySlotData* FirstSuitableSlot = nullptr;
	UInventorySlotData* FirstFreeSuitableSlot = nullptr;
	
	UInventoryBase* TargetEquipmentInventory = nullptr;
	
	for (auto* EquipmentInventory : EquipmentInventories)
	{
		if (!EquipmentInventory)
			continue;

		const auto LinkedEquipmentSlots = EquipmentInventory->GetSlotsWithLinkedEquipment();

		for (const FGuid& SlotID : LinkedEquipmentSlots)
		{
			UInventorySlotData* Slot = EquipmentInventory->GetSlotByGuid(SlotID);

			if (!Slot)
				continue;

			const FGameplayTag& EquipmentSlotTag =
				Slot->InventorySlotInfo.LinkedEquipmentSlot;

			// The slot must be compatible with the item.
			if (!EquipmentSlotTag.IsValid())
				continue;

			if (!RequiredEquipmentSlot.IsValid())
				continue;

			if (!EquipmentSlotTag.MatchesTagExact(RequiredEquipmentSlot))
				continue;
			
			if (!EquipmentComponentRef->DoesSlotExist(EquipmentSlotTag))
				continue;

			// Remember the first suitable slot.
			if (!FirstSuitableSlot)
			{
				FirstSuitableSlot = Slot;
				TargetEquipmentInventory = EquipmentInventory;
			}

			// Check whether the slot is free.
			const bool bIsFree =
				EquipmentInventory->bIsSlotEmpty(
					Slot,
					TArray<UInventorySlotData*>()
				);

			if (bIsFree)
			{
				FirstFreeSuitableSlot = Slot;
				TargetEquipmentInventory = EquipmentInventory;
				break;
			}
		}

		if (FirstFreeSuitableSlot)
			break;
	}
	
	// No compatible equipment slot exists.
	if (!FirstSuitableSlot || !TargetEquipmentInventory)
		return;

	FItemMoveData MoveData;

	MoveData.SourceInventory = CurrentInv;
	MoveData.SourceItem = Item;
	MoveData.TargetInventory = TargetEquipmentInventory;

	// Prefer a free compatible slot.
	if (FirstFreeSuitableSlot)
	{
		MoveData.TargetSlotID = FirstFreeSuitableSlot->InventorySlotInfo.SlotGuid;
		Execute_ItemTransferRequest(this, MoveData);
		return;
	}
	else
	{
		// No free slot. Use the first compatible slot
		MoveData.TargetSlotID = FirstSuitableSlot->InventorySlotInfo.SlotGuid;
		
		auto ItemInTarSlot = ItemCollectionRef->GetItemFromSlot(FirstSuitableSlot->InventorySlotInfo.SlotGuid, TargetEquipmentInventory->GetInventoryContainerID());
		
		if (TargetEquipmentInventory->GetInventorySettings().bIsReferenceContainer)
		{
			const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(ItemInTarSlot);

			TargetEquipmentInventory->HandleRemoveItem(ItemInTarSlot, Quantity);
			Execute_ItemTransferRequest(this, MoveData);
			return;
		}
		else
		{
			const FDataTableRowHandle EquippedItemRow =	IObjectDataProvider::Execute_GetItemRow(ItemInTarSlot);
			const int32 EquippedItemQuantity = IObjectDataProvider::Execute_GetQuantity(ItemInTarSlot);
			
			TargetEquipmentInventory->HandleRemoveItem(ItemInTarSlot, EquippedItemQuantity);
			Execute_ItemTransferRequest(this, MoveData);
			
			FInitItemsEntry ReturnedItemEntry;
			ReturnedItemEntry.Item = EquippedItemRow ;
			ReturnedItemEntry.Amount = EquippedItemQuantity ;
			UInvenzayUtility::AddItemQuantity(this, CurrentInv, ReturnedItemEntry);
		}
	}
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

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn && OwnerPawn->IsLocallyControlled())
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
		
	if (IPickupableass* PickupInterface = Cast<IPickupableass>(Target))
	{
		if (UObject* ItemToPick = PickupInterface->GetItemData())
		{
			FItemMoveData Data;
			Data.SourceItem = ItemToPick;
			Data.TargetInventory = MainPawnInventory;
			Data.SourceInventory = nullptr;
			
			Execute_ItemTransferRequest(this, Data);
		}
		PickupInterface->OnPickedUp();
		return;
	}

	if (ILootContainerProvider* LootProvider = Cast<ILootContainerProvider>(Target))
	{
		if (auto InventoryToDisplay = LootProvider->GetMainLootContainer())
		{
			ItemCollectionRef->SetExternalInventory(InventoryToDisplay);
		}
		if (GetOwner()->HasAuthority())
		{
			HandleInteract(Target);
		}
		return;
	}

	if (IVendorProvider* VendorProvider = Cast<IVendorProvider>(Target))
	{
		if (auto InventoryToDisplay = VendorProvider->GetVendorLootContainer())
		{
			ItemCollectionRef->SetVendorInventory(InventoryToDisplay);

			VendorProviderCurrent.SetObject(Target);
			VendorProviderCurrent.SetInterface(VendorProvider);
			
			FTradeContext TradeContext;
			TradeContext.TradeSettings = VendorProviderCurrent->GetTradeSettings();
			TradeContext.Vendor = Cast<AActor>(VendorProviderCurrent.GetObject());
			TradeContext.Buyer = this->GetOwner();

			ItemCollectionRef->GetLinkedInventories().VendorInventory->SetTradeContext(TradeContext);
			
			VendorProviderCurrent->SetTradePartnerInventory(MainPawnInventory);
			VendorProviderCurrent->SetTradePartnerItemCollection(ItemCollectionRef);
		}
		if (GetOwner()->HasAuthority())
		{
			HandleInteract(Target);
		}
		return;
	}
}

void UIInventoryManager::HandleInteract(UInteractableComponent* TargetInteractableComponent)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	bool bIsPlayer = OwnerPawn && OwnerPawn->IsLocallyControlled();

	if (!bIsPlayer)
	{
		return;
	}

	if (IPickupableass* PickupInterface = Cast<IPickupableass>(TargetInteractableComponent))
	{
		if (UObject* ItemToPick = PickupInterface->GetItemData())
		{
			FItemMoveData Data;
			Data.SourceItem = ItemToPick;
			Data.TargetInventory = MainPawnInventory;
			Data.SourceInventory = nullptr;
	
			ItemTransferRequest(Data);
		}
		PickupInterface->OnPickedUp();

		return;
	}

	if (ILootContainerProvider* LootProvider = Cast<ILootContainerProvider>(TargetInteractableComponent))
	{
		if (auto InventoryToDisplay = LootProvider->GetMainLootContainer())
		{
			LootContainerProvider.SetObject(TargetInteractableComponent);
			LootContainerProvider.SetInterface(LootProvider);
			OpenExternalInventory(InventoryToDisplay);
		}

		return;
	}

	if (IVendorProvider* VendorProvider = Cast<IVendorProvider>(TargetInteractableComponent))
	{
		if (auto InventoryToDisplay = VendorProviderCurrent->GetVendorLootContainer())
		{
			OpenVendorInventory(InventoryToDisplay);
		}
	}
}

void UIInventoryManager::InteractClearRequest(UInteractableComponent* TargetInteractableComponent)
{
	if (!TargetInteractableComponent) return;

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (GetOwner()->HasAuthority())
	{
		HandleClearInteraction(TargetInteractableComponent);
	}
	else {
		Server_HandleClearInteract(TargetInteractableComponent);
	}
}

void UIInventoryManager::Server_HandleClearInteract_Implementation(UInteractableComponent* Target)
{
	if (!ItemCollectionRef) return;
	
	if (ItemCollectionRef->GetLinkedInventories().ExternalInventory)
	{
		ItemCollectionRef->SetExternalInventory(nullptr);
	}
	
	if (ItemCollectionRef->GetLinkedInventories().VendorInventory)
	{
		ItemCollectionRef->SetVendorInventory(nullptr);
	}
}

void UIInventoryManager::HandleClearInteraction(UInteractableComponent* TargetInteractableComponent)
{
	if (ItemCollectionRef->GetLinkedInventories().ExternalInventory)
	{
		auto FindResult = ItemCollectionRef->GetContainerWidget(ItemCollectionRef->GetLinkedInventories().ExternalInventory);
		if (!FindResult)
			return;
		
		CloseExternalInventory(ItemCollectionRef->GetLinkedInventories().ExternalInventory);
		return;
	}
	
	if (auto VendorProvider = Cast<IVendorProvider>(TargetInteractableComponent))
	{
		auto FindResult = ItemCollectionRef->GetContainerWidget(ItemCollectionRef->GetLinkedInventories().VendorInventory);
		if (!FindResult)
			return;
		
		UIInvProvider->RemovePawnInvContainer(FindResult);
		CloseExternalInventory(ItemCollectionRef->GetLinkedInventories().VendorInventory);
		
		VendorProviderCurrent = nullptr;
	}
}

void UIInventoryManager::OpenVendorInventory(UInventoryBase* Inv)
{
	CreateWidget(Inv);
	HandleToggleInventory();
}

void UIInventoryManager::CloseVendorInventory(UInventoryBase* Inv)
{
	auto FindResult = ItemCollectionRef->GetContainerWidget(Inv);
	if (!FindResult)
		return;

	UIInvProvider->RemovePawnInvContainer(FindResult);
	
	ItemCollectionRef->UnregisterContainerWidget(Inv);
	ItemCollectionRef->SetVendorInventory(nullptr);
	HandleToggleInventory();
	VendorProviderCurrent = nullptr;
}

void UIInventoryManager::OpenExternalInventory(UInventoryBase* Inv)
{
	CreateWidget(Inv);
	HandleToggleInventory();
}

void UIInventoryManager::CloseExternalInventory(UInventoryBase* Inv)
{
	auto FindResult = ItemCollectionRef->GetContainerWidget(Inv);
	if (!FindResult)
		return;

	UIInvProvider->RemovePawnInvContainer(FindResult);
	
	ItemCollectionRef->UnregisterContainerWidget(Inv);
	ItemCollectionRef->SetExternalInventory(nullptr);
	HandleToggleInventory();

	LootContainerProvider.SetObject(nullptr);
	LootContainerProvider.SetInterface(nullptr);
}

void UIInventoryManager::BindInteractionWidget()
{
	if (!InteractionComponent)
		return;

	if (!InteractionUIProvider)
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
	Inventory->OnAddItemDelegate.RemoveDynamic(this, &UIInventoryManager::OnItemAddedToInventory);
	Inventory->OnAddItemDelegate.AddDynamic(this, &UIInventoryManager::OnItemAddedToInventory);

	Inventory->OnItemRemovedDelegate.RemoveDynamic(this, &UIInventoryManager::OnItemRemovedFromInventory);
	Inventory->OnItemRemovedDelegate.AddDynamic(this, &UIInventoryManager::OnItemRemovedFromInventory);

	Inventory->OnSplitDelegate.RemoveDynamic(this, &UIInventoryManager::ItemSplitRequest);
	Inventory->OnSplitDelegate.AddDynamic(this, &UIInventoryManager::ItemSplitRequest);
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

void UIInventoryManager::CollectAccessibleInventoryActions()
{
	TMap<EObjectInteractionType, FModalActionConfig> Result;
	
	auto GlobalSettings = UInvenzayUtility::GetInvenzaGlobalSettings(this);
	
	for (const auto& Pair : GlobalSettings->InvContextModalActions)
	{
		const EObjectInteractionType Action = Pair.Key;
		
		bool bAllowed = true;
		
		switch (Action)
		{
		case EObjectInteractionType::Equip:
			{
				if (!EquipmentComponentRef || EquipmentComponentRef->IsItemEquipped(PendingContextItem))
					bAllowed = false;
				break;
			}
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
			
		}

	default:
		break;
	}

	PendingContextItem = nullptr;
	PendingContextInv = nullptr;
	PendingContextSlotGuid.Invalidate();
}
