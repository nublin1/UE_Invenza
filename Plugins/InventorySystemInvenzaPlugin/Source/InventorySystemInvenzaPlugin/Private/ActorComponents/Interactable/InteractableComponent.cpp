//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/InteractableComponent.h"

#include "Net/UnrealNetwork.h"

UInteractableComponent::UInteractableComponent()
{
	SetIsReplicatedByDefault(true);
}

void UInteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UInteractableComponent, bIsInteracting);
}

// Add default functionality here for any IInteractionInterface functions that are not pure virtual.
void UInteractableComponent::BeginFocus()
{
}

void UInteractableComponent::EndFocus()
{
}

void UInteractableComponent::BeginInteract(UInteractionComponent* InteractionComponent)
{
}

void UInteractableComponent::EndInteract(UInteractionComponent* InteractionComponent)
{
}

void UInteractableComponent::Interact(UInteractionComponent* InteractionComponent)
{
}

void UInteractableComponent::StopInteract(UInteractionComponent* InteractionComponent)
{
}

void UInteractableComponent::SetInteracting(bool NewState)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	bIsInteracting = NewState;
}

void UInteractableComponent::InitializeInteractionComponent()
{
}

void UInteractableComponent::UpdateInteractableData()
{
	InteractableData.DefaultInteractableType = EInteractableType::InfoOnly;
	InteractableData.Action = FText::FromString("");
}
