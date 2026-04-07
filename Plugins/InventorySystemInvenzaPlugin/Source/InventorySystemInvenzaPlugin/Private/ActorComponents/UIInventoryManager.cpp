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
#include "ActorComponents/Trade/TradeComponent.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"
#include "ActorComponents/Interactable/PickupComponent.h"
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
#include "Service/TradeService.h"
#include "UI/Inventory/Container/InvBaseContainerWidget.h"
#include "UI/Interaction/InteractionWidget.h"
#include "UI/Inventory/InventorySlot.h"
#include "UI/Inventory/ListInventoryWidget.h"
#include "Data/Trade/TradeTypes.h"
#include "UI/ModalWidgets/ModalTradeWidget.h"

class UEnhancedInputLocalPlayerSubsystem;

UIInventoryManager::UIInventoryManager()
{
	
}

void UIInventoryManager::BeginPlay()
{
	Super::BeginPlay();
	InitializeInventoryManager();
}

void UIInventoryManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
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
	InitWidgets();
	CreateWidgetsForInventories();

	InitializeBindings();
	BindEvents();
	SetupStartingResources();
}

void UIInventoryManager::CreateInventories()
{
	for (FInventoryStartupData& StartupData : StartupInventories)
	{
		UInventoryBase* Inventory =	UInventoryBase::CreateInventory(this, StartupData);
		if (!Inventory)
			continue;

		Inventory->InitInventory();		
		Inventory->SetItemCollectionLink(ItemCollectionRef);
		
		StartingItems.Add(Inventory, StartupData.StartItems);

		Inventories.Add(Inventory->GetInventoryContainerID(), Inventory);
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
	if (!UIInvProvider)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIInventoryManager::InitWidgets: UIProvider is not set!"));
		return false;
	}

	auto InvSettings = InvToLink->GetInventorySettings();
	
	auto InvContainer =  UInvenzaWidgetFactory::CreateInventoryWidget(
		UGameplayStatics::GetPlayerController(GetWorld(), 0),
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

void UIInventoryManager::OpenTradeModal(bool bIsSaleOperation, UItemBase* OperationalItem)
{
	/*auto TradeComponent = CurrentInteractInvWidget->GetInventoryFromContainerSlot()->
		GetInventoryData().ItemCollectionLink->GetOwner()->FindComponentByClass<UTradeComponent>();
	if (!TradeComponent)
		return;

	if (!ModalTradeWidget)
	{
		ModalTradeWidget = CreateWidget<UModalTradeWidget>(GetWorld()->GetFirstPlayerController(),
					UISettings.ModalTradeWidgetClass);
		ModalTradeWidget->SetAnchorsInViewport(FAnchors(0.5f, 0.5f));
		ModalTradeWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
		ModalTradeWidget->AddToViewport();

		FVector2D ViewportSize;
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);
		}
		//const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
		const FVector2D ViewportCenter = ViewportSize * 0.5f;
		const FVector2D Center = ViewportCenter;
		
		ModalTradeWidget->SetPositionInViewport(Center, true);
	}

	if (!ModalTradeWidget)
		return;

	FText OperationalText;
	float PriceFactor = 1.0f;
	int MaxAmount = 1.0f;
	FItemMetaData ItemData = OperationalItem->GetItemRef();
	if (bIsSaleOperation)
	{
		OperationalText = FText::FromString("Buy");
		PriceFactor = TradeComponent->GetTradeSettings().SellPriceFactor;
		if (TradeComponent->GetTradeSettings().RemoveItemAfterPurchase)
			MaxAmount = OperationalItem->GetQuantity();
		else
			MaxAmount = ItemData.ItemNumeraticData.MaxStackSize;
	}
	else
	{
		OperationalText = FText::FromString("Sell");
		PriceFactor = TradeComponent->GetTradeSettings().BuyPriceFactor;
		MaxAmount = ItemData.ItemNumeraticData.MaxStackSize;
	}
	
	FModalTradeData TradeData (OperationalText,
					MaxAmount,
					ItemData.ItemTextData.NameID,
					ItemData.ItemTradeData.BasePrice * PriceFactor);

	ModalTradeWidget->InitializeTradeWidget(TradeData);
	ModalTradeWidget->SetVisibility(ESlateVisibility::Visible);

	ModalTradeWidget->ConfirmCallback = [this, bIsSaleOperation, OperationalItem](int32 Quantity)
	{
		UE_LOG(LogTemp, Log, TEXT("Trade confirmed: %d items"), Quantity);
		
		FTradeRequest Req;
		Req.Vendor				= CoreHUDWidget->GetVendorInvWidget()->GetInventoryFromContainerSlot()->
			GetInventoryData().ItemCollectionLink->GetOwner()->FindComponentByClass<UTradeComponent>();
		Req.BuyerContainer		= CoreHUDWidget->GetMainInvWidget();
		Req.VendorContainer		= CoreHUDWidget->GetVendorInvWidget();
		Req.Item				= OperationalItem;
		Req.Quantity			= Quantity;
		Req.bIsSaleOperation	= bIsSaleOperation;
		
			
		auto Result = VendorRequest(Req);
		if (Result == ETradeResult::Success)
			ModalTradeWidget->SetVisibility(ESlateVisibility::Collapsed);
	};
	ModalTradeWidget->CancelCallback = [this]()
	{
		ModalTradeWidget->SetVisibility(ESlateVisibility::Collapsed);
	};*/
}

void UIInventoryManager::SetupStartingResources()
{
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

			int32 RemainingAmount = InitResource.Amount;
			while (RemainingAmount > 0)
			{
				UItemBase* NewItem = UItemFactory::CreateItemByHandle(this, InitResource.Item, RemainingAmount);
				if (!NewItem) break;

				RemainingAmount -= NewItem->GetQuantity();

				const EItemOrientationType InitOrientation = NewItem->GetInitialItemOrientation();
                
				FItemMoveData Data;
				Data.TargetInventory  = TargetInventory;
				Data.SourceItem       = NewItem;
				Data.SavedOrientation = InitOrientation;
				Data.TargetOrientation = InitOrientation;

				ItemTransferRequest(Data);
			}
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
		if (!SlotData->LinkedEquipmentSlot.IsValid()) continue;
		
		EquipmentComponentRef->EquipItemToSlot(SlotData->LinkedEquipmentSlot, Item);
		return;
	}
}

void UIInventoryManager::OnItemRemovedFromInventory(FItemMapping ItemSlots, UItemBase* Item)
{
	if (!Item || !EquipmentComponentRef) return;

	for (UInventorySlotData* SlotData : ItemSlots.OccupiedSlots)
	{
		if (!SlotData) continue;
		if (!SlotData->LinkedEquipmentSlot.IsValid()) continue;

		EquipmentComponentRef->UnequipItemFromSlot(SlotData->LinkedEquipmentSlot);
		return;
	}
}

void UIInventoryManager::VendorRequest(FItemMoveData ItemMoveData )
{
	if (IVendorProvider* Vendor = Cast<IVendorProvider>(VendorProviderCurrent.GetObject()))
	{
		FTradeResult TradeResult = Vendor->ProcessTradeRequest(ItemMoveData);

		if (TradeResult.OperationResult == ETradeResult::TR_Success)
		{
			NotifyTradeSuccess(TradeResult);
		}
		else
		{
			NotifyTradeFailed(TradeResult.ResultMessage);
		}
	}
}

void UIInventoryManager::OnQuickTransferItem(FItemMoveData ItemMoveData)
{
	/*if (!CurrentInteractInvWidget)
		return;

	if (ItemMoveData.SourceInventory == GetMainInventory()->GetInventoryFromContainerSlot())
	{
		ItemMoveData.TargetInventory = CurrentInteractInvWidget->GetInventoryFromContainerSlot();
		ItemTransferRequest(ItemMoveData);
		return;
	}

	ItemMoveData.TargetInventory = GetMainInventory()->GetInventoryFromContainerSlot();
	ItemTransferRequest(ItemMoveData);*/
}

void UIInventoryManager::ItemTransferRequest(FItemMoveData ItemMoveData)
{
	if (VendorProviderCurrent && ItemMoveData.TargetInventory->GetInventorySettings().InventoryType == EInventoryType::VendorInventory)
	{
		VendorRequest(ItemMoveData);
		return;
	}
	
	auto Result = ItemMoveData.TargetInventory->HandleAddItem(ItemMoveData, false);
	auto ActualAmountAdded = Result.ActualAmountAdded;
	UE_LOG(LogTemp, Log, TEXT("InventoryManager::ItemTransferRequest. Is ResultMessage: %s"), *Result.ResultMessage.ToString());

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
	for (auto Element : Inventories)
	{
		if (Element.Value->GetInventorySettings().InventoryTag == Tag)
			return Element.Value;
	}

	return nullptr;
}

UInventoryBase* UIInventoryManager::GetInventoryByID(FString ContainerID)
{
	if (ContainerID.IsEmpty())
		return nullptr;
	
	if (const TObjectPtr<UInventoryBase>* Found = Inventories.Find(ContainerID))
	{
		return *Found;
	}

	return nullptr;
}

void UIInventoryManager::HandleInteract(UInteractableComponent* TargetInteractableComponent)
{
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
			OpenExternalInventory(InventoryToDisplay);
		}

		return;
	}

	if (auto VendorProvider = Cast<IVendorProvider>(TargetInteractableComponent))
	{
		if (auto InventoryToDisplay = VendorProvider->GetVendorLootContainer())
		{
			VendorProviderCurrent.SetObject(TargetInteractableComponent);
			VendorProviderCurrent.SetInterface(VendorProvider);
			OpenExternalInventory(InventoryToDisplay);

			VendorProvider->SetTradePartnerInventory(MainPawnInventory);
			VendorProvider->SetTradePartnerItemCollection(ItemCollectionRef);
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

		InventorContainerWidgetMap.Remove(ExternalInventory);
		ExternalInventory = nullptr;
	}

	if (auto VendorProvider = Cast<IVendorProvider>(TargetInteractableComponent))
	{
		auto FindResult = InventorContainerWidgetMap.Find(ExternalInventory);
		if (!FindResult)
			return;
		
		UIInvProvider->RemovePawnInvContainer(FindResult->Get());

		InventorContainerWidgetMap.Remove(ExternalInventory);
		ExternalInventory = nullptr;
		
		VendorProviderCurrent = nullptr;
	}
}

void UIInventoryManager::OpenExternalInventory(UInventoryBase* Inv)
{
	ExternalInventory = Inv;
	CreateWidget(ExternalInventory);
	
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
	
	/*if (UISettings.ToggleInventoryAction)
	{
		Input->BindAction(UISettings.ToggleInventoryAction, ETriggerEvent::Started, CoreHUDWidget.Get(), &UCoreHUDWidget::ToggleInventoryLayout);
	}

	if (UISettings.ToggleEquipmentAction)
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

	if (Inventories.IsEmpty())
		return;
	
	for (auto& Pair : Inventories)
	{
		UInventoryBase* Inventory = Pair.Value;
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

			if (!SlotData->UseAction)
				continue;

			Input->BindAction(
				SlotData->UseAction,
				ETriggerEvent::Started,
				Inventory,
				&UInventoryBase::UseSlot,
				SlotData);
		}
	}
}

bool UUInventoryWidgetBase::HandleTradeModalOpening(UItemBase* Item)
{
	if (!Item) return false;

	if (Item->GetItemRef().ItemCategory == EItemCategory::Money) return false;
	
	UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	
	/*if (InventoryManager->GetCurrentInteractInvWidget()
			&& InventoryManager->GetCurrentInteractInvWidget()->GetInventoryType() == EInventoryType::VendorInventory)
	{
		/*if (InventoryData.ItemCollectionLink->GetOwner() == Cast<APawn>(GetOwningPlayer()->GetPawn()))
		{
			InventoryManager->OpenTradeModal(false, Item);
			return true;
		}#1#
				
		InventoryManager->OpenTradeModal(true, Item);
		return true;
	}*/
	return false;
}

