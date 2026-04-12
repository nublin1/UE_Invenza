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
#include "Kismet/GameplayStatics.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"
#include "UI/Inventory/UInventoryBaseWidget.h"


class UIInventoryManager;

UContainerComponent::UContainerComponent()
{
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
	if (bIsInteracting == false)
	{
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
	
	bIsInteracting = false;
	CurrentInteractionComponent = nullptr;
}

const TMap<FString, TObjectPtr<UInventoryBase>>& UContainerComponent::GetInventoriesToDisplay() const
{
	return Inventories;
}

void UContainerComponent::InitializeInteractionComponent()
{
	Super::InitializeInteractionComponent();
	
	if (auto ItemCollection = GetOwner()->FindComponentByClass<UItemCollection>())
		ItemCollectionRef = ItemCollection;
	
	UpdateInteractableData();

	InitializeInventoryStartupData();
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
	if (!MainLootContainerInvTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("MainLootContainerInvTag is not set!"));
		return;
	}

	UInventoryBase* Inventory =	UInventoryBase::CreateInventory(GetOwner(), InventoryStartupData);
	if (!Inventory)
		return;
	
	Inventory->InitInventory();
	
	Inventory->SetItemCollectionLink(ItemCollectionRef);
	Inventory->SetInventorySettings(InventoryStartupData.Settings);
	Inventory->SetInventoryOwnerActor(GetOwner());

	Inventories.Add(Inventory->GetInventoryContainerID(), Inventory);

	if (InventoryStartupData.Settings.InventoryTag == MainLootContainerInvTag)
	{
		MainLootInventory = Inventory;
	}

	SetupStartingResources();

	Inventory->OnItemRemovedDelegate.AddDynamic(this, &UContainerComponent::DestroyWhenEmpty);
}

void UContainerComponent::SetupStartingResources()
{
	if (Inventories.IsEmpty())
		return;
	
	for (const auto& InitResource : InventoryStartupData.StartItems)
	{
		if (InitResource.Item.RowName.IsNone()) continue;

		int32 RemainingAmount = InitResource.Amount;
		while (RemainingAmount > 0)
		{
			UItemBase* NewItem = UItemFactory::CreateItemByHandle(this, InitResource.Item, RemainingAmount);
			if (!NewItem) break;

			RemainingAmount -= NewItem->GetQuantity();

			const EItemOrientationType InitOrientation = NewItem->GetInitialItemOrientation();
                
			FItemMoveData Data;
			Data.TargetInventory  = MainLootInventory;
			Data.SourceItem       = NewItem;
			Data.SavedOrientation = InitOrientation;
			Data.TargetOrientation = InitOrientation;

			MainLootInventory->HandleAddItem(Data);
		}
	}
}

void UContainerComponent::DestroyWhenEmpty(FItemMapping ItemSlots, UItemBase* Item)
{
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		if (ItemCollectionRef->GetItemLocations().IsEmpty()
		   && this->bDestroyWhenEmpty)
		{
		   CurrentInteractionComponent->StopInteract();
		   GetOwner()->K2_DestroyActor();
		}
	});
}
