// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/PickupComponent.h"

#include "Data/ItemData.h"

UPickupComponent::UPickupComponent(): ItemQuantity(0)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UPickupComponent::BeginFocus()
{
	if (PickupMesh)
	{		
		PickupMesh->SetRenderCustomDepth(true);
	}
}

void UPickupComponent::EndFocus()
{
	if (PickupMesh)
	{
		PickupMesh->SetRenderCustomDepth(false);
	}
}

void UPickupComponent::Interact(UInteractionComponent* InteractionComponent)
{
	Super::Interact(InteractionComponent);

	TakePickup(InteractionComponent);
}

void UPickupComponent::InitializePickup()
{
	if (!ItemDataTable || DesiredItemID.IsNone())
		return;
	
	FItemData* ItemData = ItemDataTable->FindRow<FItemData>(DesiredItemID, DesiredItemID.ToString());

	if (!ItemData)
		return;

	ItemRef = ItemData->ItemMetaData;

	if (ItemQuantity <= 0)
	{
		ItemQuantity = 1;
	}
}

void UPickupComponent::TakePickup(const UInteractionComponent* Taker)
{
	GetOwner()->Destroy();
}

void UPickupComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();
	
}
