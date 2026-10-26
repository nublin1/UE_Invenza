//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/InteractableComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "GameFramework/Actor.h"

#include "Net/UnrealNetwork.h"

UInteractableComponent::UInteractableComponent()
{
	SetIsReplicatedByDefault(true);
}

UInteractableComponent* UInteractableComponent::GetInteractionLockComponent() const
{
	return IsValid(GetOwner()) ? GetOwner()->FindComponentByClass<UInteractableComponent>() : nullptr;
}

bool UInteractableComponent::IsInteractionLockedByOther(const AActor* Interactor) const
{
	const UInteractableComponent* Lock = GetInteractionLockComponent();
	return Lock && Lock->ExclusiveInteractor.IsValid() && Lock->ExclusiveInteractor.Get() != Interactor;
}

bool UInteractableComponent::IsInteractionOwnedBy(const AActor* Interactor) const
{
	const UInteractableComponent* Lock = GetInteractionLockComponent();
	return Interactor && Lock && Lock->ExclusiveInteractor.Get() == Interactor;
}

bool UInteractableComponent::TryAcquireInteraction(AActor* Interactor)
{
	if (!IsValid(GetOwner()) || !GetOwner()->HasAuthority() || GetOwner()->IsActorBeingDestroyed() ||
		!IsValid(Interactor) || Interactor->IsActorBeingDestroyed() || Interactor->GetWorld() != GetWorld())
	{
		return false;
	}
	UInteractableComponent* Lock = GetInteractionLockComponent();
	if (!Lock)
	{
		return false;
	}
	if (IsInteractionLockedByOther(Interactor))
	{
		UE_LOG(LogTemp, Log, TEXT("Interaction rejected: %s is already in use by %s; requester: %s."),
			*GetNameSafe(GetOwner()), *GetNameSafe(Lock->ExclusiveInteractor.Get()), *GetNameSafe(Interactor));
		return false;
	}
	Lock->ExclusiveInteractor = Interactor;
	Interactor->OnEndPlay.AddUniqueDynamic(Lock, &UInteractableComponent::HandleExclusiveInteractorEndPlay);
	return true;
}

void UInteractableComponent::ReleaseInteraction(AActor* Interactor)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	UInteractableComponent* Lock = GetInteractionLockComponent();
	if (!Lock || Lock->ExclusiveInteractor.Get() != Interactor)
	{
		return;
	}
	if (IsValid(Interactor))
	{
		Interactor->OnEndPlay.RemoveDynamic(Lock, &UInteractableComponent::HandleExclusiveInteractorEndPlay);
	}
	Lock->ExclusiveInteractor.Reset();
}

void UInteractableComponent::HandleExclusiveInteractorEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	ReleaseInteraction(Actor);
}

void UInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AActor* Interactor = ExclusiveInteractor.Get())
	{
		if (UInteractionComponent* Interaction = Interactor->FindComponentByClass<UInteractionComponent>())
		{
			if (Interaction->GetActiveInteractable() && Interaction->GetActiveInteractable()->GetOwner() == GetOwner())
			{
				Interaction->StopInteract();
			}
		}
		ReleaseInteraction(Interactor);
	}
	Super::EndPlay(EndPlayReason);
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

FText UInteractableComponent::GetInteractionActionText(EInteractionType Type, bool bActiveForInteractor) const
{
	const FInteractableData* Data = GetInteractableDataForType(Type);
	if (!Data)
	{
		return FText::GetEmpty();
	}

	return bActiveForInteractor && !Data->ActiveAction.IsEmpty() ? Data->ActiveAction : Data->Action;
}

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
	{
		return;
	}

	FInteractableData PrimaryData;
	PrimaryData.DefaultInteractableType = EInteractableType::InfoOnly;
	PrimaryData.Action = FText::GetEmpty();
	PrimaryData.Quantity = -1;
	InteractableDataMap.Add(EInteractionType::Primary, PrimaryData);
}
