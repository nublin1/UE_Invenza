//  Nublin Studio 2025 All Rights Reserved.

#include "World//AUIManagerActor.h"

#include "EnhancedInputComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "ActorComponents/Items/itemBase.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Container/InvBaseContainerWidget.h"
#include "UI/Core/CoreHUDWidget.h"
#include "UI/Interaction/InteractionWidget.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"


AUIManagerActor::AUIManagerActor(): CoreHUDWidget(nullptr), UISettings()
{
	
}

void AUIManagerActor::BeginPlay()
{
	Super::BeginPlay();
}

void AUIManagerActor::OnQuickTransferItem(FItemMoveData ItemMoveData)
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

FItemAddResult AUIManagerActor::ItemTransferRequest(FItemMoveData ItemMoveData)
{
	auto Result = ItemMoveData.TargetInventory->HandleAddItem(ItemMoveData, false);
	switch (Result.OperationResult)
	{
	case EItemAddResult::IAR_AllItemAdded:
		if (Result.bIsUsedReferences)
		{
			break;
		}
		if (ItemMoveData.SourceInventory->GetItemCollection() == ItemMoveData.TargetInventory->GetItemCollection())
		{
			ItemMoveData.SourceInventory->HandleRemoveItemFromContainer(ItemMoveData.SourceItem);
			break;
		}
		else
		{
			ItemMoveData.SourceInventory->HandleRemoveItem(ItemMoveData.SourceItem, ItemMoveData.SourceItem->GetQuantity());
			break;
		}
		break;
	case EItemAddResult::IAR_NoItemAdded:
		break;
	case EItemAddResult::IAR_PartialAmountItemAdded:
		break;
	case EItemAddResult::IAR_ItemSwapped:
		if (ItemMoveData.SourceInventory->GetIsUseReferences() && ItemMoveData.SourceInventory != ItemMoveData.TargetInventory)
		{
			ItemMoveData.SourceInventory->HandleRemoveItemFromContainer(ItemMoveData.SourceItem);
		}
		break;
	}

	return Result;
}

void AUIManagerActor::SetInteractableType(EInteractableType InteractableType)
{
	switch (InteractableType)
	{
	case EInteractableType::Container:
		CurrentInteractInvWidget = CoreHUDWidget->GetContainerInWorldWidget();
		break;
	case EInteractableType::Vendor:
		break;
	case EInteractableType::None:
		CurrentInteractInvWidget = nullptr;
		break;
	default:
		CurrentInteractInvWidget = nullptr;
		break;
	}
}

void AUIManagerActor::BindEvents(AActor* TargetActor)
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
	InteractionComponent->RegularSettings = this->RegularSettings;
	InteractionComponent->BeginFocusDelegate.AddDynamic(InteractionWidget, &UInteractionWidget::OnFoundInteractable);
	InteractionComponent->EndFocusDelegate.AddDynamic(InteractionWidget, &UInteractionWidget::OnLostInteractable);
	InteractionComponent->IteractableDataDelegate.AddDynamic(this, &AUIManagerActor::UIIteract);

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
		ItemMoveData.SourceItem = UItemBase::CreateFromDataTable(ItemCollection->ItemDataTable, Item.ItemName, Item.ItemCount);
		CoreHUDWidget->GetMainInvWidget()->GetInventoryFromContainerSlot()->HandleAddItem(ItemMoveData);
	}
}

void AUIManagerActor::UIIteract( UInteractableComponent* TargetInteractableComponent)
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

void AUIManagerActor::OnQuickGrabPressed(const FInputActionInstance& Instance)
{
	InventoryModifierState.bIsQuickGrabModifierActive = true;
}

void AUIManagerActor::OnQuickGrabReleased(const FInputActionInstance& Instance)
{
	InventoryModifierState.bIsQuickGrabModifierActive = false;
}

void AUIManagerActor::InitializeMenuBindings()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController) return;
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
	if (!Input)
		return;
	
	if (UISettings.ToggleInventoryAction)
	{
		Input->BindAction(UISettings.ToggleInventoryAction, ETriggerEvent::Started, CoreHUDWidget, &UCoreHUDWidget::ToggleInventoryMenu);
	}

	if (UISettings.IA_Mod_QuickGrab)
	{
		Input->BindAction(UISettings.IA_Mod_QuickGrab, ETriggerEvent::Started, this, &AUIManagerActor::OnQuickGrabPressed);
		Input->BindAction(UISettings.IA_Mod_QuickGrab, ETriggerEvent::Completed, this, &AUIManagerActor::OnQuickGrabReleased);
	}
}

void AUIManagerActor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
}

