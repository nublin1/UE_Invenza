﻿//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/InteractableComponent.h"

UInteractableComponent::UInteractableComponent()
{
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

void UInteractableComponent::InitializeInteractionComponent()
{
}

void UInteractableComponent::UpdateInteractableData()
{
	InteractableData.DefaultInteractableType = EInteractableType::InfoOnly;
	InteractableData.Action = FText::FromString("");
}
