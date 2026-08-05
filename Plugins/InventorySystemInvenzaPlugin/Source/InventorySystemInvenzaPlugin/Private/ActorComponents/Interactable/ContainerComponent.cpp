//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/ContainerComponent.h"

#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Data/Inventory/InventoryBase.h"
#include "Factory/ItemFactory.h"
#include "Utility/InvenzayUtility.h"


class UIInventoryManager;

UContainerComponent::UContainerComponent()
{
	SetIsReplicatedByDefault(true);
}

void UContainerComponent::OnRegister()
{
	Super::OnRegister();

	if (AActor* Owner = GetOwner())
	{
		Owner->SetReplicates(true);
		Owner->SetReplicateMovement(true);
	}
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
	if (bIsInteracting == false)
	{
		SetInteracting(true);
	}
	else
	{
		SetInteracting(false);
	}
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
		return;
	
	if (!MainLootContainerInvTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("MainLootContainerInvTag is not set!"));
		return;
	}

	UInventoryBase* Inventory =	UInventoryBase::CreateInventoryAdvanced(GetOwner(), InventoryStartupData, GetOwner(), ItemCollectionRef);
	if (!Inventory)
		return;

	StartingItems.Add(Inventory, InventoryStartupData.StartItems);
	
	ItemCollectionRef->AddPawnInventory_Internal(Inventory);

	if (InventoryStartupData.Settings.InventoryTag == MainLootContainerInvTag)
	{
		MainLootInventory = Inventory;
	}
	
	Inventory->OnItemRemovedDelegate.AddDynamic(this, &UContainerComponent::DestroyWhenEmpty);
}

void UContainerComponent::SetupStartingResources()
{
	if (!GetOwner()->HasAuthority())
		return;

	if (StartingItems.IsEmpty())
		return;
	
	for (auto& [TargetInventory, InitItems] : StartingItems)
	{
		if (!TargetInventory 
			|| TargetInventory->GetInventoryContainerID().IsEmpty() 
			|| InitItems.IsEmpty() 
			|| !TargetInventory->GetItemCollectionLinked())
		{
			continue;
		}

		for (const auto& InitResource : InitItems)
		{
			if (InitResource.Item.RowName.IsNone()) continue;

			UItemBase* NewItemSample = UItemFactory::CreateItemByHandle(this, InitResource.Item, 1);

			UInvenzayUtility::AddItemQuantityBySample(this, TargetInventory, NewItemSample, InitResource.Amount);
		}
	}

	StartingItems.Empty();
}

void UContainerComponent::DestroyWhenEmpty(FItemMapping ItemSlots, UItemBase* Item)
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
