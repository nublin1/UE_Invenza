//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/VendorComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/TradeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "World/AUIManagerActor.h"

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
	auto ManagerActor = Cast<AUIManagerActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AUIManagerActor::StaticClass()));
	if (!Trade || !ManagerActor)
		return;

	if (!bIsInteract)
	{
		Trade->OpenTradeMenu(GetOwner(), InteractionComponent->GetOwner(), ManagerActor);
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

void UVendorComponent::OnRegister()
{
	Super::OnRegister();
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

