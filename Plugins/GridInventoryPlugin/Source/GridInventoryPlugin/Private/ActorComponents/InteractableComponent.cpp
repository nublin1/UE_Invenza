// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/InteractableComponent.h"

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

void UInteractableComponent::UpdateInteractableData()
{
	InteractableData.DefaultInteractableType = EInteractableType::InfoOnly;
	InteractableData.Action = FText::FromString("");
}
