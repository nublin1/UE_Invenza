//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/VendorComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/TradeComponent.h"

UVendorComponent::UVendorComponent()
{
}

void UVendorComponent::BeginFocus()
{
	Super::BeginFocus();
}

void UVendorComponent::EndFocus()
{
	Super::EndFocus();
}

void UVendorComponent::Interact(UInteractionComponent* InteractionComponent)
{
	Super::Interact(InteractionComponent);
	auto Trade = GetOwner()->FindComponentByClass<UTradeComponent>();
	if (!Trade)
		return;

	if (!bIsInteract)
	{
		Trade->OpenTradeMenu(GetOwner(), InteractionComponent->GetOwner());
		bIsInteract = true;
		return;
	}
	
	bIsInteract = false;
}

void UVendorComponent::StopInteract(UInteractionComponent* InteractionComponent)
{
	Super::StopInteract(InteractionComponent);
	bIsInteract = false;
	auto Trade = GetOwner()->FindComponentByClass<UTradeComponent>();
	Trade->CloseTradeMenu();
}

void UVendorComponent::InitializeInteractionComponent()
{
	Super::InitializeInteractionComponent();
	UpdateInteractableData();
}

void UVendorComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();
	InteractableData.DefaultInteractableType = EInteractableType::Vendor;
	InteractableData.Action = FText::FromString(TEXT("Trade"));
	InteractableData.Quantity = -1;
}

