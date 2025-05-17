//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/UIInventoryManager.h"

#include "DelayAction.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/TradeComponent.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "ActorComponents/Items/itemBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Factory/ItemFactory.h"
#include "Kismet/GameplayStatics.h"
#include "Service/TradeService.h"
#include "UI/Container/InvBaseContainerWidget.h"
#include "UI/Core/CoreHUDWidget.h"
#include "UI/Interaction/InteractionWidget.h"
#include "UI/Inventory/InventorySlot.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"
#include "UI/ModalWidgets/ModalTradeWidget.h"


class UEnhancedInputLocalPlayerSubsystem;

UIInventoryManager::UIInventoryManager(): CoreHUDWidget(nullptr)
{
	
}

void UIInventoryManager::OpenTradeModal(bool bIsSaleOperation, UItemBase* Operationalitem)
{
	auto TradeComponent = CurrentInteractInvWidget->GetInventoryFromContainerSlot()->
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
		
		const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
		const FVector2D ViewportCenter = ViewportSize * 0.5f;
		const FVector2D Center = ViewportCenter;
		
		ModalTradeWidget->SetPositionInViewport(Center, true);
	}

	if (!ModalTradeWidget)
		return;

	FText OperationalText;
	float PriceFactor = 1.0f;
	int MaxAmount = 1.0f;
	FItemMetaData ItemData = Operationalitem->GetItemRef();
	if (bIsSaleOperation)
	{
		OperationalText = FText::FromString("Sale");
		PriceFactor = TradeComponent->GetTradeSettings().SellPriceFactor;
		if (TradeComponent->GetTradeSettings().RemoveItemAfterPurchase)
			MaxAmount = Operationalitem->GetQuantity();
		else
			MaxAmount = ItemData.ItemNumeraticData.MaxStackSize;
	}
	else
	{
		OperationalText = FText::FromString("Buy");
		PriceFactor = TradeComponent->GetTradeSettings().BuyPriceFactor;
		MaxAmount = ItemData.ItemNumeraticData.MaxStackSize;
	}
	
	FModalTradeData TradeData (OperationalText,
					MaxAmount,
					ItemData.ItemTextData.Name,
					ItemData.ItemNumeraticData.BasePrice * PriceFactor);

	ModalTradeWidget->InitializeTradeWidget(TradeData);
	ModalTradeWidget->SetVisibility(ESlateVisibility::Visible);

	ModalTradeWidget->ConfirmCallback = [this, bIsSaleOperation, Operationalitem](int32 Quantity)
	{
		UE_LOG(LogTemp, Log, TEXT("Trade confirmed: %d items"), Quantity);
		FItemMoveData ItemMoveData;
		ItemMoveData.SourceItem = Operationalitem;
		if (bIsSaleOperation)
		{
			ItemMoveData.TargetInventory = CurrentInteractInvWidget->GetInventoryFromContainerSlot();
			ItemMoveData.SourceInventory = GetMainInventory()->GetInventoryFromContainerSlot();
		}
		else
		{
			ItemMoveData.SourceInventory = CurrentInteractInvWidget->GetInventoryFromContainerSlot();
			ItemMoveData.TargetInventory = GetMainInventory()->GetInventoryFromContainerSlot();
		}
			
		auto Result = VendorRequest(ItemMoveData);
		if (Result == ETradeResult::Success)
			ModalTradeWidget->SetVisibility(ESlateVisibility::Collapsed);
		
			
	};
	ModalTradeWidget->CancelCallback = [this]()
	{
		ModalTradeWidget->SetVisibility(ESlateVisibility::Collapsed);
	};
}

void UIInventoryManager::BeginPlay()
{
	Super::BeginPlay();

	if (ItemDataTable)
		UItemFactory::Init(ItemDataTable);
}

ETradeResult UIInventoryManager::VendorRequest(FItemMoveData ItemMoveData )
{
	FTradeRequest Req;
	Req.Vendor				= CoreHUDWidget->GetVendorInvWidget()->GetInventoryFromContainerSlot()->
		GetInventoryData().ItemCollectionLink->GetOwner()->FindComponentByClass<UTradeComponent>();
	Req.BuyerContainer		= CoreHUDWidget->GetMainInvWidget();
	Req.VendorContainer		= CoreHUDWidget->GetVendorInvWidget();
	Req.Item				= ItemMoveData.SourceItem;
	Req.Quantity			= ItemMoveData.SourceItem->GetQuantity();
	
	if (ItemMoveData.TargetInventory == CoreHUDWidget->GetVendorInvWidget()->GetInventoryFromContainerSlot())
	{
		ETradeResult Result = UTradeService::ExecuteBuy(Req);
		return Result;
	}
	else
	{
		ETradeResult Result = UTradeService::ExecuteSell(Req);
		return Result;
	}
}

void UIInventoryManager::OnQuickTransferItem(FItemMoveData ItemMoveData)
{
	if (!CurrentInteractInvWidget)
		return;

	if (ItemMoveData.SourceInventory == GetMainInventory()->GetInventoryFromContainerSlot())
	{
		ItemMoveData.TargetInventory = CurrentInteractInvWidget->GetInventoryFromContainerSlot();
		ItemTransferRequest(ItemMoveData);
		return;
	}

	ItemMoveData.TargetInventory = GetMainInventory()->GetInventoryFromContainerSlot();
	ItemTransferRequest(ItemMoveData);
}

void UIInventoryManager::ItemTransferRequest(FItemMoveData ItemMoveData)
{
	if (CoreHUDWidget->GetVendorInvWidget())
	{
		if (ItemMoveData.TargetInventory == CoreHUDWidget->GetVendorInvWidget()->GetInventoryFromContainerSlot()
		|| (ItemMoveData.SourceInventory && ItemMoveData.SourceInventory == CoreHUDWidget->GetVendorInvWidget()->GetInventoryFromContainerSlot()))
		{
			VendorRequest(ItemMoveData);
			return;
		}
	}
	
	auto Result = ItemMoveData.TargetInventory->HandleAddItem(ItemMoveData, false);
	UE_LOG(LogTemp, Log, TEXT("Is ResultMessage: %s"), *Result.ResultMessage.ToString());
	switch (Result.OperationResult)
	{
	case EItemAddResult::IAR_AllItemAdded:
		if (Result.bIsUsedReferences)
		{
			break;
		}
		if (ItemMoveData.SourceInventory && ItemMoveData.SourceInventory->GetInventoryData().ItemCollectionLink
			== ItemMoveData.TargetInventory->GetInventoryData().ItemCollectionLink)
		{
			ItemMoveData.SourceInventory->HandleRemoveItemFromContainer(ItemMoveData.SourceItem);
			break;
		}
		else if (ItemMoveData.SourceInventory)
		{
			ItemMoveData.SourceInventory->HandleRemoveItem(ItemMoveData.SourceItem, ItemMoveData.SourceItem->GetQuantity());
			break;
		}
		break;
	case EItemAddResult::IAR_NoItemAdded:
		if (CoreHUDWidget->GetVendorInvWidget())
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
		}
		
		
		break;
	case EItemAddResult::IAR_PartialAmountItemAdded:
		break;
	case EItemAddResult::IAR_ItemSwapped:
		if (!Result.bIsUsedReferences
			&& ItemMoveData.SourceInventory->GetInventorySettings().bCanReferenceItems
			&& ItemMoveData.SourceInventory != ItemMoveData.TargetInventory)
		{
			ItemMoveData.SourceInventory->HandleRemoveItemFromContainer(ItemMoveData.SourceItem);
		}
		break;
	}
}

void UIInventoryManager::SetInteractableType(UInteractableComponent* IteractData)
{
	switch (IteractData->InteractableData.DefaultInteractableType)
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
	}
}

void UIInventoryManager::ClearInteractableType(UInteractableComponent* IteractData)
{
	if (CurrentInteractInvWidget)
	{
		CoreHUDWidget->ToggleWidget(CurrentInteractInvWidget);
		CurrentInteractInvWidget = nullptr;
	}
}

void UIInventoryManager::BindEvents(AActor* TargetActor)
{
	if (!TargetActor) return;
	
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
	
	InteractionComponent->IteractableDataDelegate.AddDynamic(this, &UIInventoryManager::UIIteract);

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
		
}

void UIInventoryManager::UIIteract( UInteractableComponent* TargetInteractableComponent)
{
	if (!TargetInteractableComponent) return;

	
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
		Input->BindAction(UISettings.ToggleInventoryAction, ETriggerEvent::Started, CoreHUDWidget.Get(), &UCoreHUDWidget::ToggleInventoryLayout);
	}

	if (UISettings.ToggleEquipmentAction)
	{
		Input->BindAction(UISettings.ToggleEquipmentAction, ETriggerEvent::Started, CoreHUDWidget.Get(), &UCoreHUDWidget::ToggleEquipmentLayout);
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

	InitializeInvSlotsBindings();
}

void UIInventoryManager::InitializeInvSlotsBindings()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController) return;

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
	if (!Input) return;
	
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UInvBaseContainerWidget::StaticClass(), false);
	for (const auto Widget : FoundWidgets)
	{
		auto Inventory = Cast<UInvBaseContainerWidget>(Widget)->GetInventoryFromContainerSlot();
		if (!Inventory) continue;

		auto InvSlots = Inventory->GetInventoryData().InventorySlots;
		if (InvSlots.IsEmpty()) continue;

		for (const auto Slot : InvSlots)
		{
			if (Slot->GetUseAction())
				Input->BindAction(Slot->GetUseAction(), ETriggerEvent::Started, Inventory, &UUInventoryWidgetBase::UseSlot, Slot);
		}
	}
}

void UIInventoryManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
}

