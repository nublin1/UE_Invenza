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
#include "Factory/ItemFactory.h"
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
#include "Utility/InventoryUtility.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

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

	UInteractionComponent* InteractionComp = GetOwner()->FindComponentByClass<UInteractionComponent>();
	if (InteractionComp)
	{
		InteractionComponent = InteractionComp;
		//InteractionComponent->SetInventoryManager(this);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InteractionComponent is not valid!"));
	}

	CreateInventories();

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn && OwnerPawn->IsLocallyControlled())
	{
		InitWidgets();
		CreateWidgetsForInventories();
		InitializeBindings();
		BindEvents();
	}
	
	SetupStartingResources();

	OnInitializationCompleteDelegate.Broadcast();
}

void UIInventoryManager::CreateInventories()
{
	if (!GetOwner()->HasAuthority())
		return;
	
	for (FInventoryStartupData& StartupData : StartupInventories)
	{
		UInventoryBase* Inventory =	UInventoryBase::CreateInventoryAdvanced(GetOwner(), StartupData, GetOwner(), ItemCollectionRef);
		if (!Inventory)
			continue;
		
		StartingItems.Add(Inventory, StartupData.StartItems);

		ItemCollectionRef->AddPawnInventory_Internal(Inventory);
		BindInventoryEvents(Inventory);

		if (StartupData.Settings.InventoryTag == UISettings.MainInvTag)
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
		if (InventorContainerWidgetMap.Contains(Inventory))
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
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) return false;
	
	if (!UIInvProvider)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIInventoryManager::InitWidgets: UIProvider is not set!"));
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
	InvWidget->InitializeInventoryWidgetWithSettings(InvSettings);

	InvContainer->InitializeInventoryBindings();
	
	InvWidget->BindDelegated();

	InvWidget->OnItemDroppedDelegate.AddDynamic(this, &UIInventoryManager::ItemTransferRequest);

	UIInvProvider->AddPawnInvContainerWidget(InvContainer);

	InventorContainerWidgetMap.Add(InvToLink, InvContainer);
	
	return true;
}

void UIInventoryManager::InitWidgets()
{
	if (!UIInvProvider)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIInventoryManager::InitWidgets: UIProvider is not set!"));
		return;
	}

	auto AllPawnContInvs = UIInvProvider->GetAllPawnInvContainers();
	if (AllPawnContInvs.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("UIInventoryManager::InitWidgets: PawnInvs is empty!"));
		return;
	}
	
	for (auto ContInvs : AllPawnContInvs)
	{
		if (!ContInvs || !ContInvs->GetInventoryWidgetFromContainerSlot())
			continue;

		auto InvWidget = ContInvs->GetInventoryWidgetFromContainerSlot();

		if (!InvWidget->TargetInventoryTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("UIInventoryManager::InitWidgets: TargetInventoryTag is not set!"));
			continue;
		};
		

		auto TarInv = GetInventoryByTag(InvWidget->TargetInventoryTag);
		if (!TarInv)
			continue;

		if (USlotbasedInventoryWidget* SlotBased = Cast<USlotbasedInventoryWidget>(InvWidget))
		{
			SlotBased->SetInventoryBaseRef(TarInv);			
			InvWidget->InitializeInventoryWidget();			
			
			SlotBased->SetUISettings(UISettings);
			SlotBased->BindDelegated();
		}
		else if (UListInventoryWidget* ListBased = Cast<UListInventoryWidget>(InvWidget))
		{
			InvWidget->InitializeInventoryWidget();
			
			auto TarListInv = Cast<UListInventory>(TarInv);
			if (!TarListInv)
				continue;

			//TarListInv->SetEntryClass(TarListInv->GetInventorySettings().EntryClass);
			
			ListBased->SetInventoryBaseRef(TarListInv);
			ListBased->SetUISettings(UISettings);
			ListBased->BindDelegated();
		}

		InvWidget->OnItemDroppedDelegate.AddDynamic(this, &UIInventoryManager::ItemTransferRequest);

		InventorContainerWidgetMap.Add(TarInv, ContInvs);
	}

	for (auto ContainerBase : UIInvProvider->GetAllPawnInvContainers())
	{
		ContainerBase->InitializeInventoryBindings();
	}
}

void UIInventoryManager::SetupStartingResources()
{
	if (!GetOwner()->HasAuthority())
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

			UItemBase* NewItemSample = UItemFactory::CreateItemByHandle(this, InitResource.Item, 1);

			UInventoryUtility::AddItemQuantity(this, TargetInventory, NewItemSample, InitResource.Amount);
		}
	}

	StartingItems.Empty();
}

void UIInventoryManager::OnItemAddedToInventory(FItemMapping& ItemSlots, UItemBase* Item)
{
	if (!Item || !EquipmentComponentRef) return;

	for (UInventorySlotData* SlotData : ItemSlots.OccupiedSlots)
	{
		if (!SlotData) continue;
		if (!SlotData->InventorySlotInfo.LinkedEquipmentSlot.IsValid()) continue;
		
		EquipmentComponentRef->EquipItemToSlot(SlotData->InventorySlotInfo.LinkedEquipmentSlot, Item);
		return;
	}
}

void UIInventoryManager::OnItemRemovedFromInventory(FItemMapping ItemSlots, UItemBase* Item)
{
	if (!Item || !EquipmentComponentRef) return;

	for (UInventorySlotData* SlotData : ItemSlots.OccupiedSlots)
	{
		if (!SlotData) continue;
		if (!SlotData->InventorySlotInfo.LinkedEquipmentSlot.IsValid()) continue;

		EquipmentComponentRef->UnequipItemFromSlot(SlotData->InventorySlotInfo.LinkedEquipmentSlot);
		return;
	}
}

UInventoryBase* UIInventoryManager::ResolveTargetInventory(UInventoryBase* SourceInventory) const
{
	if (!SourceInventory)
		return nullptr;
	
	if (VendorInventory)
	{
		return (SourceInventory == VendorInventory)
			? MainPawnInventory
			: VendorInventory;
	}
	
	if (ExternalInventory)
	{
		return (SourceInventory == ExternalInventory)
			? MainPawnInventory
			: ExternalInventory;
	}
	
	return MainPawnInventory;
}

void UIInventoryManager::OnQuickTransferItem_Implementation(FItemMoveData InData)
{
	if (!MainPawnInventory)
		return;
	
	FItemMoveData ItemMoveData = InData;

	if (ItemMoveData.SourceInventory == ItemMoveData.TargetInventory)
		return;
	
	ItemMoveData.TargetInventory = ResolveTargetInventory(ItemMoveData.SourceInventory);

	if (!ItemMoveData.TargetInventory)
		return;

	ItemTransferRequest(ItemMoveData);
}

void UIInventoryManager::OnQuickTransferAllSameItems_Implementation(FItemMoveData ItemMoveData)
{
	auto InvID = ItemMoveData.SourceInventory->GetInventoryContainerID();
	
	auto SameItems = ItemCollectionRef->GetAllSameItemsInContainer(InvID, ItemMoveData.SourceItem);
	for (auto Item : SameItems)
	{
		ItemMoveData.SourceItem = Item;
		OnQuickTransferItem(ItemMoveData);
	}
}

void UIInventoryManager::VendorRequest(FItemMoveData ItemMoveData )
{
	if (ItemMoveData.SourceInventory == ItemMoveData.TargetInventory)
		return;
	
	FTradeResult TradeResult = VendorProviderCurrent->ProcessTradeRequest(ItemMoveData);
		
}

void UIInventoryManager::ItemTransferRequest(FItemMoveData ItemMoveData)
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

	if (ItemMoveData.SourceInventory)
		ItemMoveData.SourceInventory->RequestToResetItemVisual(ItemMoveData.SourceItem);
	
	switch (Result.OperationResult)
	{
	case EItemAddResult::IAR_AllItemAdded:
		if (ItemMoveData.SourceInventory && ItemMoveData.SourceInventory->GetItemCollectionLinked())
		{
			ItemMoveData.SourceInventory->HandleRemoveItem(ItemMoveData.SourceItem, ActualAmountAdded);
			break;
		}
		break;
	case EItemAddResult::IAR_NoItemAdded:
		if (ItemMoveData.SourceInventory == nullptr)
			ItemDropRequest(ItemMoveData.SourceItem);
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

void UIInventoryManager::ItemDropRequest(UItemBase* ItemToDrop)
{
	if (auto Pawn = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPawn())
	{
		/*auto Interaction = Pawn->FindComponentByClass<UInteractionComponent>();
		if (!Interaction) break;*/

		ItemToDrop->DropItem(GetWorld());
	}
}

UInventoryBase* UIInventoryManager::GetInventoryByTag(const FGameplayTag& Tag)
{
	for (auto Element : ItemCollectionRef->GetActorInventories())
	{
		if (Element->GetInventorySettings().InventoryTag == Tag)
			return Element;
	}

	return nullptr;
}

UInventoryBase* UIInventoryManager::GetInventoryByID(FString ContainerID)
{
	if (ContainerID.IsEmpty())
		return nullptr;

	for (auto Element : ItemCollectionRef->GetActorInventories())
	{
		if (Element->GetInventorySettings().InventoryID == ContainerID)
			return Element;
	}

	return nullptr;
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
		if (UItemBase* ItemToPick = PickupInterface->GetItemData())
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

	if (auto VendorComponent = Cast<UVendorComponent>(TargetInteractableComponent))
	{
		if (auto InventoryToDisplay = VendorComponent->GetVendorLootContainer())
		{
			VendorProviderCurrent = VendorComponent;
			VendorInventory = InventoryToDisplay;

			FTradeContext TradeContext;
			TradeContext.TradeSettings = VendorComponent->GetTradeSettings();
			TradeContext.Vendor = VendorComponent->GetOwner();
			TradeContext.Buyer = this->GetOwner();

			VendorInventory->SetTradeContext(TradeContext);

			OpenVendorInventory(InventoryToDisplay);
			
			VendorComponent->SetTradePartnerInventory(MainPawnInventory);
			VendorComponent->SetTradePartnerItemCollection(ItemCollectionRef);
		}
	}
}

void UIInventoryManager::HandleClearInteraction(UInteractableComponent* TargetInteractableComponent)
{
	if (ILootContainerProvider* LootProvider = Cast<ILootContainerProvider>(TargetInteractableComponent))
	{
		if (!ExternalInventory)
			return;

		auto FindResult = InventorContainerWidgetMap.Find(ExternalInventory);
		if (!FindResult)
			return;
		
		UIInvProvider->RemovePawnInvContainer(FindResult->Get());
		CloseExternalInventory(ExternalInventory);
		LootProvider = nullptr;
	}

	if (auto VendorProvider = Cast<IVendorProvider>(TargetInteractableComponent))
	{
		auto FindResult = InventorContainerWidgetMap.Find(ExternalInventory);
		if (!FindResult)
			return;
		
		UIInvProvider->RemovePawnInvContainer(FindResult->Get());
		CloseExternalInventory(ExternalInventory);
		
		VendorProviderCurrent = nullptr;
	}
}

void UIInventoryManager::OpenVendorInventory(UInventoryBase* Inv)
{
	VendorInventory = Inv;
	CreateWidget(VendorInventory);
	HandleToggleInventory();
}

void UIInventoryManager::CloseVendorInventory(UInventoryBase* Inv)
{
	InventorContainerWidgetMap.Remove(VendorInventory);
	VendorInventory = nullptr;
	HandleToggleInventory();
}

void UIInventoryManager::OpenExternalInventory(UInventoryBase* Inv)
{
	ExternalInventory = Inv;
	CreateWidget(ExternalInventory);
	HandleToggleInventory();
}

void UIInventoryManager::CloseExternalInventory(UInventoryBase* Inv)
{
	InventorContainerWidgetMap.Remove(ExternalInventory);
	ExternalInventory = nullptr;
	HandleToggleInventory();
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
		InteractionComponent->OnInteract.AddDynamic(this, &UIInventoryManager::HandleInteract);
		InteractionComponent->OnStopInteract.AddDynamic(this, &UIInventoryManager::HandleClearInteraction);
		
		BindInteractionWidget();
		
	}		
}

void UIInventoryManager::BindInventoryEvents(UInventoryBase* Inventory)
{
	if (!Inventory) return;
	Inventory->OnAddItemDelegate.AddDynamic(this, &UIInventoryManager::OnItemAddedToInventory);
	Inventory->OnItemRemovedDelegate.AddDynamic(this, &UIInventoryManager::OnItemRemovedFromInventory);
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
	
	if (InteractionComponent && ExternalInventory)
	{
		/*APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (!PC)
			return;

		if (PC->bShowMouseCursor)
			InteractionComponent->StopInteract();

		return;*/
	}
	
	UIInvProvider->ToggleInventoryLayout();
	
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

	/*if (UISettings.ToggleEquipmentAction)
	{
		Input->BindAction(UISettings.ToggleEquipmentAction, ETriggerEvent::Started, CoreHUDWidget.Get(), &UCoreHUDWidget::ToggleEquipmentLayout);
	}*/

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
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController) return;

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
	if (!Input) return;

	auto ActorInvs = ItemCollectionRef->GetActorInventories();

	if (ActorInvs.IsEmpty())
		return;
	
	for (auto& Inv : ActorInvs)
	{
		UInventoryBase* Inventory = Inv;
		if (!Inventory)
			continue;

		USlotbasedInventory* SlotBased = Cast<USlotbasedInventory>(Inventory);
		if (!SlotBased)
			continue;

		if (SlotBased->GetInventorySlots().IsEmpty()) continue;
		for (UInventorySlotData* SlotData : SlotBased->GetInventorySlots())
		{
			if (!SlotData)
				continue;

			if (!SlotData->InventorySlotInfo.UseAction)
				continue;

			Input->BindAction(
				SlotData->InventorySlotInfo.UseAction.Get(),
				ETriggerEvent::Started,
				Inventory,
				&UInventoryBase::UseSlot,
				SlotData);
		}
	}
}