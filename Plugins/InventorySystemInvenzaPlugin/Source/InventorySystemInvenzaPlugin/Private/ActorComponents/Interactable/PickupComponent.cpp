//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/Interactable/PickupComponent.h"

#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Data/Items/itemBase.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Factory/ItemFactory.h"


UPickupComponent::UPickupComponent(): InitialItem()
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
	
	CachedMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>();

	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
	   InitializePickupComponent();
	});
}

void UPickupComponent::BeginFocus()
{
	if (CachedMesh)
	{
		CachedMesh->SetRenderCustomDepth(true);
	}
}

void UPickupComponent::EndFocus()
{
	if (CachedMesh)
	{
		CachedMesh->SetRenderCustomDepth(false);
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
    
	InteractableData.Quantity = ItemBase->GetQuantity();
	if (InteractableData.Name.IsEmpty())
		InteractableData.Name = ItemBase->GetItemRef().ItemTextData.DisplayName;
	
	if (CachedMesh && ItemBase->GetItemRef().ItemAssetData.Mesh)
	{
		CachedMesh->SetStaticMesh(ItemBase->GetItemRef().ItemAssetData.Mesh);
	}

	UpdateInteractableData();
}

void UPickupComponent::InitializePickupComponent()
{
	if (InitialItem.Item.RowName.IsNone())
		return;
    
	UItemBase* NewItem = UItemFactory::CreateItemByHandle(this, InitialItem.Item, InitialItem.Amount);
	if (!NewItem)
	{
		UE_LOG(LogTemp, Error, TEXT("Item creation failed for %s!"), *InitialItem.Item.RowName.ToString());
		return;
	}
    
	ItemBase = NewItem;

	InteractableData.Quantity = ItemBase->GetQuantity();
	if (InteractableData.Name.IsEmpty())
		InteractableData.Name = ItemBase->GetItemRef().ItemTextData.DisplayName;

	if (CachedMesh && ItemBase->GetItemRef().ItemAssetData.Mesh)
	{
		CachedMesh->SetStaticMesh(ItemBase->GetItemRef().ItemAssetData.Mesh);
	}
}

void UPickupComponent::TakePickup(UInteractionComponent* Taker)
{
	if (bIsPendingDestruction || !ItemBase || !Taker)
		return;

	UIInventoryManager* InventoryManager = Taker->GetInventoryManager();
	if (!InventoryManager)
		return;

	auto Inv = InventoryManager->GetInventoryByTag(InventoryManager->GetUISettings().MainInvTag);
	if (!Inv)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainInventory is NULL!"));
		return;
	}
	
	bIsPendingDestruction = true;

	FItemMoveData Data;
	Data.SourceItem = ItemBase;
	Data.TargetInventory = Inv;
	Data.SourceInventory = nullptr;
	
	InventoryManager->ItemTransferRequest(Data);

	GetOwner()->Destroy();
}

void UPickupComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();
	InteractableData.DefaultInteractableType = EInteractableType::Pickup;
	InteractableData.Action = FText::FromString(TEXT("Pickup"));
}
