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
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "Factory/ItemFactory.h"
#include "Interface/Inventory/InvUIProvider.h"
#include "Kismet/GameplayStatics.h"
#include "Service/TradeService.h"
#include "UI/Inventory/Container/InvBaseContainerWidget.h"
#include "UI/Interaction/InteractionWidget.h"
#include "UI/Inventory/InventorySlot.h"
#include "UI/Inventory/ListInventoryWidget.h"
#include "UI/ModalWidgets/ModalTradeWidget.h"

class UEnhancedInputLocalPlayerSubsystem;

UIInventoryManager::UIInventoryManager()
{
	
}

void UIInventoryManager::BeginPlay()
{
	Super::BeginPlay();

	auto Collection = GetOwner()->FindComponentByClass<UItemCollection>();
	if (!Collection)
		return;

	ItemCollectionRef = Collection;

	CreateInventories();
	InitWidgets();

	InitializeBindings();
}

void UIInventoryManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
}

void UIInventoryManager::CreateInventories()
{
	for (const FInventoryStartupData& StartupData : StartupInventories)
	{
		UInventoryBase* Inventory =
			NewObject<UInventoryBase>(this, StartupData.InventoryClass);

		if (!Inventory)
			continue;

		Inventory->SetInventorySettings(StartupData.Settings);
		Inventory->SetInitialItems(StartupData.StartItems);
		Inventory->SetItemCollectionLink(ItemCollectionRef);

		Inventories.Add(StartupData.Settings.InventoryTag, Inventory);
		BindInventoryEvents(Inventory);

		if (StartupData.Settings.InventoryTag == UISettings.MainInvTag)
		{
			MainPawnInventory = Inventory;
		}
	}
}

void UIInventoryManager::InitWidgets()
{
	if (!UIProvider)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIInventoryManager::InitWidgets: UIProvider is not set!"));
		return;
	}
	
	for (auto WidgetBase : UIProvider->GetAllPawnInventories())
	{
		if (!WidgetBase)
			continue;

		WidgetBase->InitializeInventoryWidget();

		auto TarInv = GetInventoryByTag(WidgetBase->TargetInventoryTag);
		if (!TarInv)
			continue;

		if (USlotbasedInventoryWidget* SlotBased = Cast<USlotbasedInventoryWidget>(WidgetBase))
		{
			auto TarSlotInv = Cast<USlotbasedInventory>(TarInv);
			if (!TarSlotInv)
				continue;
			
			auto SizeInv = SlotBased->GetNumberRowsAndColumns();
			
			TarSlotInv->SetInventorySize(SizeInv);
			TarSlotInv->SetInvSlots(SlotBased->GetSlotData());
			
			SlotBased->SetUISettings(UISettings);
			SlotBased->SetInventoryBaseRef(TarInv);			
			SlotBased->BindDelegated();
		}
		else if (UListInventoryWidget* ListBased = Cast<UListInventoryWidget>(WidgetBase))
		{
			ListBased->SetInventoryBaseRef(TarInv);
			ListBased->SetUISettings(UISettings);
			ListBased->BindDelegated();
		}

		TarInv->SetupStartingResources();

		WidgetBase->OnItemDroppedDelegate.AddDynamic(this, &UIInventoryManager::ItemTransferRequest);
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

ETradeResult UIInventoryManager::VendorRequest(const FTradeRequest TradeRequest )
{
	if (TradeRequest.bIsSaleOperation)
	{
		ETradeResult Result = UTradeService::ExecuteSell(TradeRequest);
		return Result;
	}
	else
	{
		ETradeResult Result = UTradeService::ExecuteBuy(TradeRequest);
		return Result;
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
	/*if (CoreHUDWidget->GetVendorInvWidget())
	{
		if (ItemMoveData.TargetInventory == CoreHUDWidget->GetVendorInvWidget()->GetInventoryFromContainerSlot()
		|| (ItemMoveData.SourceInventory && ItemMoveData.SourceInventory == CoreHUDWidget->GetVendorInvWidget()->GetInventoryFromContainerSlot()))
		{
			bool IsSale = (ItemMoveData.TargetInventory != CoreHUDWidget->GetVendorInvWidget()->GetInventoryFromContainerSlot());
			
			FTradeRequest Req(CoreHUDWidget->GetVendorInvWidget()->GetInventoryFromContainerSlot()->
				GetInventoryData().ItemCollectionLink->GetOwner()->FindComponentByClass<UTradeComponent>(),
				CoreHUDWidget->GetVendorInvWidget(),
				GetMainInventory(),
				ItemMoveData.SourceItem,
				ItemMoveData.TargetSlot,
				ItemMoveData.SourceItem->GetQuantity(), IsSale);
			
			VendorRequest(Req);
			return;
		}
	}*/
	
	auto Result = ItemMoveData.TargetInventory->HandleAddItem(ItemMoveData, false);
	UE_LOG(LogTemp, Log, TEXT("InventoryManager::ItemTransferRequest. Is ResultMessage: %s"), *Result.ResultMessage.ToString());
	switch (Result.OperationResult)
	{
	case EItemAddResult::IAR_AllItemAdded:
		if (Result.bIsUsedReferences)
		{
			break;
		}
		if (ItemMoveData.SourceInventory && ItemMoveData.SourceInventory->GetItemCollectionLinked()
			== ItemMoveData.TargetInventory->GetItemCollectionLinked())
		{
			ItemMoveData.SourceInventory->HandleRemoveItem(ItemMoveData.SourceItem, ItemMoveData.SourceItem->GetQuantity());
			break;
		}
		break;
	case EItemAddResult::IAR_NoItemAdded:
		/*if (CoreHUDWidget->GetVendorInvWidget())
		{
			if (!ItemMoveData.SourceInventory || ItemMoveData.SourceInventory == CoreHUDWidget->GetVendorInvWidget()->GetInventoryFromContainerSlot())
			{
				if (auto Pawn = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPawn())
				{
					auto Interaction = Pawn->FindComponentByClass<UInteractionComponent>();
					if (!Interaction) break;

					ItemMoveData.SourceItem->DropItem(GetWorld());
				}
				if (ItemMoveData.SourceInventory && ItemMoveData.SourceInventory == CoreHUDWidget->GetVendorInvWidget()->GetInventoryFromContainerSlot())
				{
					ItemMoveData.SourceInventory->HandleRemoveItemFromContainer(ItemMoveData.SourceItem);
				}
			}
		}*/
		break;
	case EItemAddResult::IAR_PartialAmountItemAdded:
		if (Result.bIsUsedReferences)
		{
			break;
		}
		if (ItemMoveData.SourceInventory)
		{
			ItemMoveData.SourceInventory->HandleRemoveItem(ItemMoveData.SourceItem, Result.ActualAmountAdded);
			break;
		}
		break;
	case EItemAddResult::IAR_ItemSwapped:
		if (!Result.bIsUsedReferences
			&& ItemMoveData.SourceInventory->GetInventorySettings().bAllowItemReferencing
			&& ItemMoveData.SourceInventory != ItemMoveData.TargetInventory)
		{
			ItemMoveData.SourceInventory->HandleRemoveItem(ItemMoveData.SourceItem, ItemMoveData.SourceItem->GetQuantity());
		}
		break;
	}
}

UInventoryBase* UIInventoryManager::GetInventoryByTag(const FGameplayTag& Tag)
{
	if (const TObjectPtr<UInventoryBase>* Found = Inventories.Find(Tag))
	{
		return *Found;
	}

	return nullptr;
}

UInventoryBase* UIInventoryManager::GetInventoryByID(FString ContainerID)
{
	if (ContainerID.IsEmpty())
		return nullptr;
	
	for (auto Element : Inventories)
	{
		if (Element.Value->GetInventoryContainerID() == ContainerID)
			return Element.Value;
	}

	return nullptr;
}

void UIInventoryManager::SetInteractableType(UInteractableComponent* IteractData)
{
	/*switch (IteractData->InteractableData.DefaultInteractableType)
	{
	case EInteractableType::Container:
		CurrentInteractInvWidget = CoreHUDWidget->GetContainerInWorldWidget();
		if (!CurrentInteractInvWidget)
			break;
		CoreHUDWidget->ToggleWidget(CurrentInteractInvWidget);
		break;
	case EInteractableType::Vendor:
		CurrentInteractInvWidget = CoreHUDWidget->GetVendorInvWidget();
		if (!CurrentInteractInvWidget)
			break;
		if (auto Collection = IteractData->GetOwner()->FindComponentByClass<UItemCollection>())
		{
			CurrentInteractInvWidget->GetInventoryFromContainerSlot()->SetItemCollection(Collection);
			CurrentInteractInvWidget->GetInventoryFromContainerSlot()->InitItemsInItemsCollection();
		}
		CoreHUDWidget->ToggleWidget(CurrentInteractInvWidget);
		GetMainInventory()->GetInventoryFromContainerSlot()->ReDrawAllItems();
		break;
	case EInteractableType::None:
		if (CurrentInteractInvWidget)
		{
			CoreHUDWidget->ToggleWidget(CurrentInteractInvWidget);
			CurrentInteractInvWidget = nullptr;
		}
		break;
	default:
		if (CurrentInteractInvWidget)
		{
			CoreHUDWidget->ToggleWidget(CurrentInteractInvWidget);
			CurrentInteractInvWidget = nullptr;
		}
		break;
	}*/
}

void UIInventoryManager::ClearInteractableType(UInteractableComponent* IteractData)
{
	/*if (CurrentInteractInvWidget)
	{
		CoreHUDWidget->ToggleWidget(CurrentInteractInvWidget);
		CurrentInteractInvWidget = nullptr;
	}*/
}

void UIInventoryManager::BindEvents()
{
		
	/*	
	UInteractionComponent* InteractionComponent = TargetActor->FindComponentByClass<UInteractionComponent>();

	if (!InteractionComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("InteractionComponent is not valid!"));
		return;
	}

	auto InteractionWidget = CoreHUDWidget->GetInteractionWidget();

	if (!IsValid(InteractionWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("InteractionWidget is not valid or pending kill!"));
		return;
	}
	
	//
	InteractionComponent->RegularSettings = this->UISettings;
	InteractionComponent->BeginFocusDelegate.AddDynamic(InteractionWidget, &UInteractionWidget::OnFoundInteractable);
	InteractionComponent->EndFocusDelegate.AddDynamic(InteractionWidget, &UInteractionWidget::OnLostInteractable);
	InteractionComponent->OnInteractionProgress.AddDynamic(InteractionWidget, &UInteractionWidget::UpdateProgressBar);
	
	InteractionComponent->IteractableDataDelegate.AddDynamic(this, &UIInventoryManager::SetInteractableType);
	InteractionComponent->StopIteractDelegate.AddDynamic(this, &UIInventoryManager::ClearInteractableType);


	UItemCollection* ItemCollection = TargetActor->FindComponentByClass<UItemCollection>();
	if (!ItemCollection) return;
		
	if (!CoreHUDWidget->GetMainInvWidget())
	{
		UE_LOG(LogTemp, Warning, TEXT("MainInv is not Found!"));
		return;
	}
	CoreHUDWidget->GetMainInvWidget()->GetInventoryFromContainerSlot()->SetItemCollection(ItemCollection);
	CoreHUDWidget->GetMainInvWidget()->GetInventoryFromContainerSlot()->InitItemsInItemsCollection();

	CoreHUDWidget->GetMainInvWidget()->GetInventoryFromContainerSlot()->OnItemDroppedDelegate.AddDynamic(this, &UIInventoryManager::ItemTransferRequest);
	if (auto ContainerInWorldWidget = CoreHUDWidget->GetContainerInWorldWidget())
		ContainerInWorldWidget->GetInventoryFromContainerSlot()->OnItemDroppedDelegate.AddDynamic(this, &UIInventoryManager::ItemTransferRequest);
	if (auto VendorInvWidget = CoreHUDWidget->GetVendorInvWidget())
		VendorInvWidget->GetInventoryFromContainerSlot()->OnItemDroppedDelegate.AddDynamic(this, &UIInventoryManager::ItemTransferRequest);
	if (auto HotbarInvWidget = CoreHUDWidget->GetHotbarInvWidget())
		HotbarInvWidget->GetInventoryFromContainerSlot()->OnItemDroppedDelegate.AddDynamic(this, &UIInventoryManager::ItemTransferRequest);
	if (auto EquipInvWidget = CoreHUDWidget->GetEquipmentInvWidget())
		EquipInvWidget->GetInventoryFromContainerSlot()->OnItemDroppedDelegate.AddDynamic(this, &UIInventoryManager::ItemTransferRequest);
		*/
		
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

		if (SlotBased->GetInvSlots().IsEmpty()) continue;
		for (UInventorySlotData* SlotData : SlotBased->GetInvSlots())
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

