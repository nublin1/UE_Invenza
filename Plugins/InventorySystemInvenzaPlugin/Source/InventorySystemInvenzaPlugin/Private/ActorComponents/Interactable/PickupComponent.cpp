//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/Interactable/PickupComponent.h"

#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Data/Items/itemBase.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/ActorChannel.h"
#include "Factory/ItemFactory.h"
#include "Net/UnrealNetwork.h"
#include "Utility/InterfaceUtils.h"


UPickupComponent::UPickupComponent(): InitialItem()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void UPickupComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPickupComponent, ItemBase);
}

bool UPickupComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (ItemBase)
		bWroteSomething |= Channel->ReplicateSubobject(ItemBase, *Bunch, *RepFlags);

	return bWroteSomething;
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
		UpdateInteractableData();
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
}

void UPickupComponent::InitializeDrop(FInitItemsEntry ItemToDrop)
{
	InitialItem = ItemToDrop;

	InitializePickupComponent();

	UpdateInteractableData();
}

UObject* UPickupComponent::GetItemData()
{
	if (bIsPendingDestruction)
		return nullptr;

	bIsPendingDestruction = true;
	
	return ItemBase;
}

void UPickupComponent::OnPickedUp()
{
	GetOwner()->Destroy();
}

void UPickupComponent::InitializePickupComponent()
{
	if (!GetOwner()->HasAuthority()) return;
	
	if (InitialItem.Item.RowName.IsNone())
		return;
    
	UObject* NewItem = UItemFactory::CreateItemByHandle(this, InitialItem.Item, InitialItem.Amount);
	if (!NewItem)
	{
		UE_LOG(LogTemp, Error, TEXT("Item creation failed for %s!"), *InitialItem.Item.RowName.ToString());
		return;
	}
    
	ItemBase = NewItem;

	const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(ItemBase);

	if (CachedMesh && Meta.ItemAssetData.Mesh)
	{
		CachedMesh->SetStaticMesh(Meta.ItemAssetData.Mesh);
	}
}

void UPickupComponent::OnRep_ItemBase()
{
	UpdateInteractableData();
}

void UPickupComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();
	InteractableData.DefaultInteractableType = EInteractableType::Pickup;
	InteractableData.Action = FText::FromString(TEXT("Pickup"));

	if (ItemBase &&
		UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemBase, TEXT("UpdateInteractableData")))
	{
		InteractableData.Quantity = IObjectDataProvider::Execute_GetQuantity(ItemBase);

		if (InteractableData.Name.IsEmpty())
		{
			const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(ItemBase);
			InteractableData.Name = Meta.ItemTextData.DisplayName;
		}
	}
}
