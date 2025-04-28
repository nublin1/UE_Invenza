//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/InteractableContainerComponent.h"

#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/Items/itemBase.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Factory/ItemFactory.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Inventory/UInventoryWidgetBase.h"


class UIInventoryManager;

UInteractableContainerComponent::UInteractableContainerComponent()
{
}

void UInteractableContainerComponent::BeginFocus()
{
	Super::BeginFocus();
	if (auto StaticMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>())
	{
		StaticMesh->SetRenderCustomDepth(true);
	}
}

void UInteractableContainerComponent::EndFocus()
{
	Super::EndFocus();
	if (auto StaticMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>())
	{
		StaticMesh->SetRenderCustomDepth(false);
	}
}

void UInteractableContainerComponent::Interact(UInteractionComponent* InteractionComponent)
{
	Super::Interact(InteractionComponent);

	if (!ItemCollection) return;
	InitializeItemCollection();	

	FindContainerWidget();
	if (!InventoryWidget) return;

	CurrentInteractionComponent = InteractionComponent;
	InventoryWidget->OnVisibilityChanged.AddDynamic(this, &UInteractableContainerComponent::ContainerWidgetVisibilityChanged);
	
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

void UInteractableContainerComponent::StopInteract(UInteractionComponent* InteractionComponent)
{
	Super::StopInteract(InteractionComponent);

	InventoryWidget->OnVisibilityChanged.RemoveDynamic(this, &UInteractableContainerComponent::ContainerWidgetVisibilityChanged);
	ContainerWidget=nullptr;
	bIsInteracting = false;
	CurrentInteractionComponent = nullptr;
}

void UInteractableContainerComponent::OnRegister()
{
	Super::OnRegister();
	InitializeInteractionComponent();
}

void UInteractableContainerComponent::ContainerWidgetVisibilityChanged(ESlateVisibility NewVisibility)
{
	if (NewVisibility != ESlateVisibility::Visible)
	{
		CurrentInteractionComponent->StopInteract();
	}
}

void UInteractableContainerComponent::InitializeInteractionComponent()
{
	Super::InitializeInteractionComponent();
	ItemCollection = GetOwner()->FindComponentByClass<UItemCollection>();
	UpdateInteractableData();
}

void UInteractableContainerComponent::InitializeItemCollection() 
{
	FindContainerWidget();
	if (!InventoryWidget ||! ContainerWidget) return;
	InventoryWidget->SetItemCollection(ItemCollection);

	if (ItemCollection->InitItems.IsEmpty())
		return;
	
	for (const auto& Item : ItemCollection->InitItems)
	{
		FItemMoveData ItemMoveData;
		ItemMoveData.SourceItem =  UItemFactory::CreateItemByID(this, Item.ItemName, Item.ItemCount);
		if (ItemMoveData.SourceItem)
		{
			InventoryWidget->HandleAddItem(ItemMoveData);
		}
	}
	
	ItemCollection->InitItems.Empty();
}

void UInteractableContainerComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();

	InteractableData.DefaultInteractableType = EInteractableType::Container;
	InteractableData.Action = FText::FromString(TEXT("Open"));
	InteractableData.Quantity = -1;
}

void UInteractableContainerComponent::FindContainerWidget()
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
