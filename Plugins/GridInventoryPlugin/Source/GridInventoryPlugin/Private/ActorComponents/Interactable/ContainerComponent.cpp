//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/ContainerComponent.h"

#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"
#include "UI/Inventory/UInventoryWidgetBase.h"


class UIInventoryManager;

UContainerComponent::UContainerComponent()
{
}

void UContainerComponent::BeginFocus()
{
	Super::BeginFocus();
	if (auto StaticMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>())
	{
		StaticMesh->SetRenderCustomDepth(true);
	}
}

void UContainerComponent::EndFocus()
{
	Super::EndFocus();
	if (auto StaticMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>())
	{
		StaticMesh->SetRenderCustomDepth(false);
	}
}

void UContainerComponent::Interact(UInteractionComponent* InteractionComponent)
{
	Super::Interact(InteractionComponent);

	if (!ItemCollection) return;
	
	FindContainerWidget();
	InitializeItemCollection();	
	
	if (!InventoryWidget) return;

	CurrentInteractionComponent = InteractionComponent;
	InventoryWidget->OnVisibilityChanged.AddDynamic(this, &UContainerComponent::ContainerWidgetVisibilityChanged);
	InventoryWidget->OnPostRemoveItemDelegate.AddDynamic(this, &UContainerComponent::DestroyWhenEmpty);
	
	if (bIsInteracting == false)
	{
		InventoryWidget->ReDrawAllItems();
		bIsInteracting = true;
	}
	else
	{
		bIsInteracting = false;
	}
}

void UContainerComponent::StopInteract(UInteractionComponent* InteractionComponent)
{
	Super::StopInteract(InteractionComponent);

	if (InventoryWidget)
	{
		InventoryWidget->OnVisibilityChanged.RemoveDynamic(this, &UContainerComponent::ContainerWidgetVisibilityChanged);
		InventoryWidget->OnPostRemoveItemDelegate.RemoveDynamic(this, &UContainerComponent::DestroyWhenEmpty);
	}
	ContainerWidget=nullptr;
	bIsInteracting = false;
	CurrentInteractionComponent = nullptr;
}

void UContainerComponent::OnRegister()
{
	Super::OnRegister();
	InitializeInteractionComponent();
}

void UContainerComponent::ContainerWidgetVisibilityChanged(ESlateVisibility NewVisibility)
{
	if (NewVisibility != ESlateVisibility::Visible)
	{
		CurrentInteractionComponent->StopInteract();
	}
}

void UContainerComponent::InitializeInteractionComponent()
{
	Super::InitializeInteractionComponent();
	ItemCollection = GetOwner()->FindComponentByClass<UItemCollection>();
	UpdateInteractableData();
}

void UContainerComponent::InitializeItemCollection() 
{
	if (!ContainerWidget) return;

	InventoryWidget = ContainerWidget->GetInventoryFromContainerSlot();
	if (!InventoryWidget) return;

	if (InventorySize != FVector2D::ZeroVector)
	{
		if (auto SlotBased = Cast<USlotbasedInventoryWidget>(InventoryWidget))
		{
			SlotBased->RebuildSlots(InventorySize.X, InventorySize.Y);
		}
	}
	
	InventoryWidget->SetItemCollection(ItemCollection);
	InventoryWidget->InitItemsInItemsCollection();
}

void UContainerComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();

	InteractableData.DefaultInteractableType = EInteractableType::Container;
	InteractableData.Action = FText::FromString(TEXT("Open"));
	InteractableData.Quantity = -1;
	
}

void UContainerComponent::FindContainerWidget()
{
	auto* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
		return;

	UIInventoryManager* InventoryManager = PlayerPawn->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager)
		return;

	auto* CoreHUDWidget = InventoryManager->GetCoreHUDWidget();
	if (!CoreHUDWidget)
		return;

	auto* FoundContainerWidget = CoreHUDWidget->GetContainerInWorldWidget();
	if (!FoundContainerWidget)
		return;

	auto* FoundInventoryWidget = FoundContainerWidget->GetInventoryFromContainerSlot();
	if (!FoundInventoryWidget)
		return;

	ContainerWidget = FoundContainerWidget;
	InventoryWidget = FoundInventoryWidget;
}

void UContainerComponent::DestroyWhenEmpty()
{
	if (ItemCollection->GetItemLocations().IsEmpty()
		&& this->bDestroyWhenEmpty)
	{
		CurrentInteractionComponent->StopInteract();
		GetOwner()->K2_DestroyActor();
	}
}
