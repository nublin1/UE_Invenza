//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/PickupComponent.h"

#include "ActorComponents/Items/itemBase.h"
#include "Components/BoxComponent.h"
#include "Data/ItemData.h"

UPickupComponent::UPickupComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPickupComponent::OnRegister()
{
	Super::OnRegister();
	InitializePickupComponent();
	UpdateInteractableData();
}

void UPickupComponent::BeginPlay()
{
	Super::BeginPlay();
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
	if (!ItemDataTable || DesiredItemID.IsNone())
		return;
	
	FItemData* ItemData = ItemDataTable->FindRow<FItemData>(DesiredItemID, DesiredItemID.ToString());

	if (!ItemData)
		return;

	ItemBase = NewObject<UItemBase>();
	ItemBase->SetItemRef(ItemData->ItemMetaData);
	if (InitQuantity>ItemData->ItemMetaData.ItemNumeraticData.MaxStackSize )
	{
		InitQuantity = ItemData->ItemMetaData.ItemNumeraticData.MaxStackSize;
	}
	ItemBase->SetQuantity(InitQuantity);
	InteractableData.Quantity = InitQuantity;
	InteractableData.Name = ItemBase->GetItemRef().ItemTextData.Name;

	if (auto StaticMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>())
	{
		StaticMesh->SetStaticMesh(ItemData->ItemMetaData.ItemAssetData.Mesh);
	}
	
}

void UPickupComponent::TakePickup(const UInteractionComponent* Taker)
{
	GetOwner()->Destroy();
}

void UPickupComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();
	InteractableData.DefaultInteractableType = EInteractableType::Pickup;
	InteractableData.Action =  FText::FromString(TEXT("Pickup"));
	
}
