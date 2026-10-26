//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/ContainerComponent.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "TimerManager.h"

#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Net/UnrealNetwork.h"
#include "Utility/InvenzayUtility.h"

class UIInventoryManager;

UContainerComponent::UContainerComponent()
{
	SetIsReplicatedByDefault(true);
}

void UContainerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UContainerComponent, MainLootInventory);
}

void UContainerComponent::OnRegister()
{
	Super::OnRegister();
}

void UContainerComponent::OnRep_MainLootInventory()
{
	if (auto* Collection = GetOwner()->FindComponentByClass<UInventoryCollectionComponent>())
	{
		Collection->OnActorInventoriesChanged.Broadcast();
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

void UContainerComponent::HandleInteract(UInteractionComponent* InteractionComponent)
{
	Super::HandleInteract(InteractionComponent);

	if (!ItemCollectionRef)
	{
		return;
	}

	CurrentInteractionComponent = InteractionComponent;
	SetInteracting(!bIsInteracting);
}

void UContainerComponent::HandleStopInteract(UInteractionComponent* InteractionComponent, EInteractionType Type)
{
	Super::HandleStopInteract(InteractionComponent, Type);
	SetInteracting(false);
	CurrentInteractionComponent = nullptr;
}

void UContainerComponent::CheckDestroyWhenEmpty()
{
	if (!bDestroyWhenEmpty)
	{
		return;
	}

	if (!MainLootInventory)
	{
		return;
	}

	auto Items = MainLootInventory->GetItemCollectionLinked()->GetAllItemsByContainer(
		MainLootInventory->GetInventoryContainerID());
	if (Items.IsEmpty())
	{
		Server_DestroyWhenEmpty();
	}
}

void UContainerComponent::InitializeInteractionComponent()
{
	Super::InitializeInteractionComponent();

	if (auto ItemCollection = GetOwner()->FindComponentByClass<UInventoryCollectionComponent>())
	{
		ItemCollectionRef = ItemCollection;
	}

	UpdateInteractableData();

	InitializeInventoryStartupData();
	SetupStartingResources();
}

void UContainerComponent::UpdateInteractableData()
{

	if (InteractableDataMap.Contains(EInteractionType::Primary))
	{
		return;
	}

	FInteractableData PrimaryData;
	PrimaryData.DefaultInteractableType = EInteractableType::Container;
	PrimaryData.Action = GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ContainerComponent.Literal0"));
	PrimaryData.ActiveAction =
		GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ContainerComponent.Literal1"));
	PrimaryData.Quantity = -1;
	PrimaryData.bHoldToInteract = false;

	InteractableDataMap.Add(EInteractionType::Primary, PrimaryData);

	Super::UpdateInteractableData();
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
		UInventoryBase* Inventory =
			UInvenzayUtility::CreateStartupInventory(this, ItemCollectionRef, Element, StartingItems);

		if (!Inventory)
		{
			return;
		}

		if (Element.Settings.InventoryTag == MainLootContainerInvTag)
		{
			MainLootInventory = Inventory;
		}
	}
}

void UContainerComponent::SetupStartingResources()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	UInvenzayUtility::SetupStartingResources(this, StartingItems);
}

void UContainerComponent::Server_DestroyWhenEmpty_Implementation()
{
	if (ItemCollectionRef)
	{
		ItemCollectionRef->SetExternalInventory(nullptr);
	}

	TWeakObjectPtr<UContainerComponent> WeakThis(this);

	GetWorld()->GetTimerManager().SetTimerForNextTick(
		[WeakThis]()
		{
			if (!WeakThis.IsValid())
			{
				return;
			}
			UContainerComponent* Self = WeakThis.Get();

			if (AActor* Owner = Self->GetOwner())
			{
				Owner->K2_DestroyActor();
			}
		});
}
