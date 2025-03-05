// Fill out your copyright notice in the Description page of Project Settings.


#include "World/Pickup.h"

#include "Components/BoxComponent.h"
#include "Data/ItemData.h"

// Sets default values
APickup::APickup()
{
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
	PickupMesh->SetSimulatePhysics(true);
	SetRootComponent(PickupMesh);

	BoxCollider = CreateDefaultSubobject<UBoxComponent>("BoxCollider");	
	bIsDebug? BoxCollider->SetHiddenInGame(false): BoxCollider->SetHiddenInGame(true);
	BoxCollider->AttachToComponent(PickupMesh, FAttachmentTransformRules::KeepRelativeTransform);

}

void APickup::BeginFocus()
{
	if (PickupMesh)
	{		
		PickupMesh->SetRenderCustomDepth(true);
	}
}

void APickup::EndFocus()
{
	if (PickupMesh)
	{
		PickupMesh->SetRenderCustomDepth(false);
	}
}


void APickup::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void APickup::InitializePickup()
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
