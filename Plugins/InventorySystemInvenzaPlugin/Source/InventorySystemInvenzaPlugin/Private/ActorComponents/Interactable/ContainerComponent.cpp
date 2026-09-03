//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/ContainerComponent.h"

#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/UIInventoryManager.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Utility/InvenzayUtility.h"


class UIInventoryManager;

UContainerComponent::UContainerComponent()
{
	SetIsReplicatedByDefault(true);
}

void UContainerComponent::OnRegister()
{
	Super::OnRegister();
}

void UContainerComponent::BeginPlay()
{
	Super::BeginPlay();
	CachedMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
	
	InitializeInteractionComponent();
}

void UContainerComponent::BeginFocus()
{
	Super::BeginFocus();
	if (CachedMesh)
	{
		CachedMesh->SetRenderCustomDepth(true);
	}
}

void UContainerComponent::EndFocus()
{
	Super::EndFocus();
	if (CachedMesh)
	{
		CachedMesh->SetRenderCustomDepth(false);
	}
}

void UContainerComponent::Interact(UInteractionComponent* InteractionComponent)
{
	Super::Interact(InteractionComponent);

	if (!ItemCollectionRef) return;

	CurrentInteractionComponent = InteractionComponent;
	SetInteracting(!bIsInteracting);
}

void UContainerComponent::StopInteract(UInteractionComponent* InteractionComponent)
{
	Super::StopInteract(InteractionComponent);
	
	SetInteracting(false);
	CurrentInteractionComponent = nullptr;
}

void UContainerComponent::InitializeInteractionComponent()
{
	Super::InitializeInteractionComponent();
	
	if (auto ItemCollection = GetOwner()->FindComponentByClass<UItemCollection>())
		ItemCollectionRef = ItemCollection;
	
	UpdateInteractableData();

	InitializeInventoryStartupData();
	SetupStartingResources();
}

void UContainerComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();

	InteractableData.DefaultInteractableType = EInteractableType::Container;
	InteractableData.Action = FText::FromString(TEXT("Open"));
	InteractableData.Quantity = -1;
}

void UContainerComponent::InitializeInventoryStartupData()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (!MainLootContainerInvTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("MainLootContainerInvTag is not set!"));
		return;
	}

	for (auto Element : StartupInventories)
	{
		UInventoryBase* Inventory = UInvenzayUtility::CreateStartupInventory(
		this,
		ItemCollectionRef,
		Element,
		StartingItems);

		if (!Inventory)
		{
			return;
		}

		if (Element.Settings.InventoryTag == MainLootContainerInvTag)
		{
			MainLootInventory = Inventory;
		}
	}

	if (MainLootInventory)
	{
		MainLootInventory->OnItemRemovedDelegate.AddDynamic(this, &UContainerComponent::DestroyWhenEmpty);
	}
}

void UContainerComponent::SetupStartingResources()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	UInvenzayUtility::SetupStartingResources(this,StartingItems);
}

void UContainerComponent::DestroyWhenEmpty(FItemMapping ItemSlots, UObject* Item)
{
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		if (ItemCollectionRef->GetItemLocations().Items.IsEmpty()
		   && this->bDestroyWhenEmpty)
		{
		   CurrentInteractionComponent->StopInteract();
		   GetOwner()->K2_DestroyActor();
		}
	});
}
