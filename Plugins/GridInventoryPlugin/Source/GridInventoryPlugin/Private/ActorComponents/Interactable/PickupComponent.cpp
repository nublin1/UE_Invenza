//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/PickupComponent.h"

#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/Items/itemBase.h"
#include "Components/BoxComponent.h"
#include "Data/ItemData.h"
#include "Factory/ItemFactory.h"
#include "UI/Inventory/UInventoryWidgetBase.h"

UPickupComponent::UPickupComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPickupComponent::OnRegister()
{
	Super::OnRegister();
	UpdateInteractableData();
}

void UPickupComponent::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		InitializePickupComponent();
	});
}

void UPickupComponent::BeginFocus()
{
	if (auto StaticMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>())
	{
		StaticMesh->SetRenderCustomDepth(true);
	}
}

void UPickupComponent::EndFocus()
{
	if (auto StaticMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>())
	{
		StaticMesh->SetRenderCustomDepth(false);
	}
}

void UPickupComponent::Interact(UInteractionComponent* InteractionComponent)
{
	Super::Interact(InteractionComponent);

	TakePickup(InteractionComponent);
}

void UPickupComponent::InitializeDrop(UItemBase* ItemToDrop)
{
	ItemBase = ItemToDrop;
	if (auto StaticMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>())
	{
		StaticMesh->SetStaticMesh(ItemBase->GetItemRef().ItemAssetData.Mesh);
	}

	UpdateInteractableData();
}

void UPickupComponent::InitializePickupComponent()
{
	if (DesiredItemID.IsNone())
		return;

	ItemBase = UItemFactory::CreateItemByID(this, DesiredItemID, InitQuantity);
	if (!ItemBase)
	{
		UE_LOG(LogTemp, Error, TEXT("Item create is failed!"));
		return;
	}

	InteractableData.Quantity = ItemBase->GetQuantity();
	InteractableData.Name = ItemBase->GetItemRef().ItemTextData.Name;

	if (auto StaticMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>())
	{
		StaticMesh->SetStaticMesh(ItemBase->GetItemRef().ItemAssetData.Mesh);
	}
}

void UPickupComponent::TakePickup(const UInteractionComponent* Taker)
{
	UIInventoryManager* InventoryManager = Taker->GetOwner()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager)
		return;

	auto Inv = InventoryManager->GetMainInventory()->GetInventoryFromContainerSlot();

	if (!Inv)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainInventory is NULL!"));
		return;
	}
	if (!ItemBase)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item is NULL!"));
		return;
	}

	FItemMoveData Data;
	Data.SourceItem = ItemBase;
	Data.TargetInventory = Inv;

	FItemAddResult Result = Inv->HandleAddItem(Data);
	//UE_LOG(LogTemp, Warning, TEXT("USpecialInteractableComponent"));

	GetOwner()->Destroy();
}

void UPickupComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();
	InteractableData.DefaultInteractableType = EInteractableType::Pickup;
	InteractableData.Action = FText::FromString(TEXT("Pickup"));
}
