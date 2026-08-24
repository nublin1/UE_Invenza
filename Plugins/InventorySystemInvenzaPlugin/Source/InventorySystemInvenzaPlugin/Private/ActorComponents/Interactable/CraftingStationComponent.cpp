// Nublin Studio 2026 All Rights Reserved.


#include "ActorComponents/Interactable/CraftingStationComponent.h"

#include "ActorComponents/Crafting/CraftingComponent.h"


UCraftingStationComponent::UCraftingStationComponent()
{
	SetIsReplicatedByDefault(true);
}

void UCraftingStationComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UCraftingStationComponent::Interact(UInteractionComponent* InteractionComponent)
{
	Super::Interact(InteractionComponent);

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

void UCraftingStationComponent::StopInteract(UInteractionComponent* InteractionComponent)
{
	Super::StopInteract(InteractionComponent);
	
	SetInteracting(false);
	CurrentInteractionComponent = nullptr;
}

bool UCraftingStationComponent::InitializeCraftingStation()
{
	CraftingComponent = nullptr;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	CraftingComponent =	OwnerActor->FindComponentByClass<UCraftingComponent>();
	if (!CraftingComponent)
	{
		UE_LOG(LogTemp,	Error, TEXT("[%s] Actor '%s' has UCraftingStationComponent, but no UCraftingComponent was found."),
			*GetName(),	*OwnerActor->GetName());

		return false;
	}

	return true;
}

void UCraftingStationComponent::InitializeInteractionComponent()
{
	InitializeCraftingStation();
}

void UCraftingStationComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();

	InteractableData.DefaultInteractableType = EInteractableType::Craft;
	InteractableData.Action = FText::FromString(TEXT("Open Craft"));
	InteractableData.Quantity = -1;
}
