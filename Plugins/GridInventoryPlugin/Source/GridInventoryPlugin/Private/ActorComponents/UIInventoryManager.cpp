//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/UIInventoryManager.h"

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
#include "UI/Inventory/SlotbasedInventoryWidget.h"


class UEnhancedInputLocalPlayerSubsystem;

UIInventoryManager::UIInventoryManager(): CoreHUDWidget(nullptr)
{
	
}

void UIInventoryManager::BeginPlay()
{
	Super::BeginPlay();

	if (ItemDataTable)
		UItemFactory::Init(ItemDataTable);
}

void UIInventoryManager::VendorRequest(FItemMoveData ItemMoveData )
{
	FTradeRequest Req;
	Req.Vendor      = CoreHUDWidget->GetVendorInvWidget()->GetInventoryFromContainerSlot()->
		GetInventoryData().ItemCollectionLink->GetOwner()->FindComponentByClass<UTradeComponent>();
	Req.BuyerInv	= CoreHUDWidget->GetMainInvWidget()->GetInventoryFromContainerSlot();
	Req.VendorInv   = CoreHUDWidget->GetVendorInvWidget()->GetInventoryFromContainerSlot();
	Req.Item		= ItemMoveData.SourceItem;
	Req.Quantity	= ItemMoveData.SourceItem->GetQuantity();

	if (ItemMoveData.TargetInventory == CoreHUDWidget->GetVendorInvWidget()->GetInventoryFromContainerSlot())
		ETradeResult Result = UTradeService::ExecuteBuy(Req);
	else
	{
		ETradeResult Result = UTradeService::ExecuteSell(Req);
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
		CurrentInteractInvWidget->SetVisibility(ESlateVisibility::Visible);
		break;
	case EInteractableType::Vendor:
		CurrentInteractInvWidget = CoreHUDWidget->GetVendorInvWidget();
		if (auto Collection = IteractData->GetOwner()->FindComponentByClass<UItemCollection>())
			CurrentInteractInvWidget->GetInventoryFromContainerSlot()->ChangeItemCollectionLink(Collection);
		CurrentInteractInvWidget->SetVisibility(ESlateVisibility::Visible);
		break;
	case EInteractableType::None:
		if (CurrentInteractInvWidget)
		{
			CurrentInteractInvWidget->SetVisibility(ESlateVisibility::Collapsed);
			CurrentInteractInvWidget = nullptr;
		}
		break;
	default:
		if (CurrentInteractInvWidget)
		{
			CurrentInteractInvWidget->SetVisibility(ESlateVisibility::Collapsed);
			CurrentInteractInvWidget = nullptr;
		}
		break;
	}
}

void UIInventoryManager::ClearInteractableType(UInteractableComponent* IteractData)
{
	if (CurrentInteractInvWidget)
	{
		CurrentInteractInvWidget->SetVisibility(ESlateVisibility::Collapsed);
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
	for (auto Item : ItemCollection->InitItems)
	{
		FItemMoveData ItemMoveData;
		ItemMoveData.SourceItem = UItemFactory::CreateItemByID(this, Item.ItemName, Item.ItemCount);
		CoreHUDWidget->GetMainInvWidget()->GetInventoryFromContainerSlot()->HandleAddItem(ItemMoveData);
	}

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

	if (auto* PickupComp = Cast<UPickupComponent>(TargetInteractableComponent))
	{
		auto Inv = CoreHUDWidget->GetMainInvWidget()->GetInventoryFromContainerSlot();
		auto Item = PickupComp->GetItemBase();

		if (!Inv)
		{
			UE_LOG(LogTemp, Warning, TEXT("MainInventory is NULL!"));
			return;
		}
		if (!Item)
		{
			UE_LOG(LogTemp, Warning, TEXT("Item is NULL!"));
			return;
		}

		FItemMoveData Data;
		Data.SourceItem = Item;
		Data.TargetInventory = Inv;
		
		FItemAddResult Result = Inv->HandleAddItem(Data);
		//UE_LOG(LogTemp, Warning, TEXT("USpecialInteractableComponent"));
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

	if (UISettings.GameplayMappingContext)
	{
		InputSubsystem->AddMappingContext(UISettings.InventoryMappingContext, 2);
	}
	
	if (UISettings.ToggleInventoryAction)
	{
		Input->BindAction(UISettings.ToggleInventoryAction, ETriggerEvent::Started, CoreHUDWidget, &UCoreHUDWidget::ToggleInventoryLayout);
	}

	if (UISettings.ToggleEquipmentAction)
	{
		Input->BindAction(UISettings.ToggleEquipmentAction, ETriggerEvent::Started, CoreHUDWidget, &UCoreHUDWidget::ToggleEquipmentLayout);
	}

	if (UISettings.IA_Mod_QuickGrab)
	{
		Input->BindAction(UISettings.IA_Mod_QuickGrab, ETriggerEvent::Started, this, &UIInventoryManager::OnQuickGrabPressed);
		Input->BindAction(UISettings.IA_Mod_QuickGrab, ETriggerEvent::Completed, this, &UIInventoryManager::OnQuickGrabReleased);
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

