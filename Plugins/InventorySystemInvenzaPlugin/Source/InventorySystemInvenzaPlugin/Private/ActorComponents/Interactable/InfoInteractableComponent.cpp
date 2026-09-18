// Nublin Studio 2026 All Rights Reserved.


#include "ActorComponents/Interactable/InfoInteractableComponent.h"

#include "ActorComponents/InteractionComponent.h"
#include "Subsystems/ModalWindowManager.h"

UInfoInteractableComponent::UInfoInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SetIsReplicatedByDefault(true);

	FInteractableData Data;
	Data.DefaultInteractableType = EInteractableType::InfoOnly;
	Data.Name = FText::FromString(TEXT("Information"));
	Data.Action = FText::FromString(TEXT("Read"));
	Data.Quantity = -1;
	Data.InteractableDuration = 0.0f;
	Data.bHoldToInteract = false;

	InteractableDataMap.Add(EInteractionType::Primary, Data);
}

void UInfoInteractableComponent::HandleInteract(UInteractionComponent* InteractionComponent)
{
	Super::HandleInteract(InteractionComponent);

	if (!IsValid(InteractionComponent) || bModalOpen)
	{
		return;
	}
	
	AActor* Interactor = InteractionComponent->GetOwner();
	APlayerController* PlayerController = Cast<APlayerController>(Interactor);

	if (!PlayerController)
	{
		if (const APawn* Pawn = Cast<APawn>(Interactor))
		{
			PlayerController = Cast<APlayerController>(Pawn->GetController());
		}
	}

	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UModalWindowManager* ModalManager =	LocalPlayer->GetSubsystem<UModalWindowManager>();
	if (!ModalManager)
	{
		return;
	}
	
	FModalHeaderData HeaderData(EModalHeaderType::SimpleText,InformationText);
	
	TMap<EObjectInteractionType, FModalActionConfig> Actions;
	Actions.Add(EObjectInteractionType::Ok, FModalActionConfig());

	FModalResultDelegate ResultDelegate;
	ResultDelegate.BindDynamic(this, &UInfoInteractableComponent::HandleModalResult);

	CurrentInteractionComponent = InteractionComponent;
	bModalOpen = true;

	ModalManager->OpenModalFlow(
		this,
		HeaderData,
		EModalFooterType::Notification,
		Actions,
		ResultDelegate);
}

void UInfoInteractableComponent::HandleModalResult(FModalResult Result)
{
	bModalOpen = false;
	
	UInteractionComponent* Interactor = CurrentInteractionComponent;
	CurrentInteractionComponent = nullptr;

	if (IsValid(Interactor))
	{
		Interactor->StopInteract();
	}
}

void UInfoInteractableComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();
}