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

const FInteractableData& UInteractableComponent::GetInteractableData() const
{
	static FInteractableData Empty;
	const FInteractableData* Found = InteractableDataMap.Find(EInteractionType::Primary);
	return Found ? *Found : Empty;
}

// Add default functionality here for any IInteractionInterface functions that are not pure virtual.
void UInteractableComponent::BeginFocus()
{
}

void UInteractableComponent::EndFocus()
{
}

void UInteractableComponent::BeginInteract(UInteractionComponent* InteractionComponent, EInteractionType Type)
{
}

void UInteractableComponent::EndInteract(UInteractionComponent* InteractionComponent, EInteractionType Type)
{
}

void UInteractableComponent::HandleInteract(UInteractionComponent* InteractionComponent)
{
	
}

void UInteractableComponent::HandleStopInteract(UInteractionComponent* InteractionComponent, EInteractionType Type)
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
	if (InteractableDataMap.Contains(EInteractionType::Primary))
		return;
	
	FInteractableData PrimaryData;
	PrimaryData.DefaultInteractableType = EInteractableType::InfoOnly;
	PrimaryData.Action = FText::FromString(TEXT(""));
	PrimaryData.Quantity = -1;
	InteractableDataMap.Add(EInteractionType::Primary, PrimaryData);
}
