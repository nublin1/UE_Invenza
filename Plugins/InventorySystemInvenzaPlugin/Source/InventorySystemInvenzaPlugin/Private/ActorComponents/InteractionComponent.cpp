//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/InteractionComponent.h"

#include "EnhancedInputComponent.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "TimerManager.h"      
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"


UInteractionComponent::UInteractionComponent(): TraceChannel(), TargetInteractableComponent(nullptr),
                                                CurrentInteractableComponent(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	InteractionCheckInterval = 0.1f;
	InteractionCheckDistance = 500.0f;

	ComponentTags.Add(FName("InteractionComponent"));
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle InitTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(InitTimerHandle, [this]
	{
		InitInteractionComponent();
	}, 0.3f, false);
}

void UInteractionComponent::InitInteractionComponent()
{
	auto PlayerCharacter = Cast<ACharacter>(GetOwner());
	if (!PlayerCharacter)
		return;

	CameraComponent = PlayerCharacter->FindComponentByClass<UCameraComponent>();
	
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent);
	if (!Input)
		return;

	Input->BindAction(InteractAction, ETriggerEvent::Started, this, &UInteractionComponent::BeginInteract);
	Input->BindAction(InteractAction, ETriggerEvent::Completed, this, &UInteractionComponent::EndInteract);
}

void UInteractionComponent::PerformInteractionCheck()
{
	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !CameraComponent)
	{
		return;
	}

	FVector TraceStart = CameraComponent->GetComponentLocation();
	FVector TraceEnd = TraceStart + CameraComponent->GetForwardVector() * InteractionCheckDistance;

	if (FVector::DotProduct(CameraComponent->GetForwardVector(), CameraComponent->GetComponentRotation().Vector()) <= 0)
	{
		NotFoundInteractable();
		return;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	FHitResult TraceHit;

	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 2.0f, 0, 2.0f);
	if (GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, TraceChannel, QueryParams))
	{
		AActor* HitActor = TraceHit.GetActor();

		if (HitActor)
		{
			UInteractableComponent* InteractableComp = HitActor->FindComponentByClass<UInteractableComponent>();
			
			if (InteractableComp && InteractableComp != InteractionData.CurrentInteractable)
			{
				FoundInteractable(HitActor, InteractableComp);
			}
			return;
		}
	}

	NotFoundInteractable();
}

void UInteractionComponent::FoundInteractable(AActor* NewInteractable, UInteractableComponent* NewInteractableComp )
{
	if (InteractionData.CurrentInteractable)
	{
		if (InteractionData.CurrentInteractable != InteractionData.LastInteractable)
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Interaction);
			InteractionData.LastInteractable = InteractionData.CurrentInteractable;
		}

		TargetInteractableComponent = InteractionData.CurrentInteractable;
		TargetInteractableComponent->EndFocus();
	}

	InteractionData.CurrentInteractable = NewInteractableComp;
	TargetInteractableComponent = NewInteractableComp;

	TargetInteractableComponent->BeginFocus();
	if (BeginFocusDelegate.IsBound())
	{
		BeginFocusDelegate.Broadcast(TargetInteractableComponent->InteractableData);
	}
}

void UInteractionComponent::NotFoundInteractable()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Interaction);
	
	if (InteractionData.CurrentInteractable)
	{
		if (IsValid(TargetInteractableComponent))
		{
			TargetInteractableComponent->EndFocus();
		}

		if (EndFocusDelegate.IsBound() && InteractionData.CurrentInteractable)
		{
			EndFocusDelegate.Broadcast(InteractionData.CurrentInteractable->InteractableData);
		}
		InteractionData.CurrentInteractable = nullptr;		
		
		InteractionData.LastInteractable = InteractionData.CurrentInteractable;
		TargetInteractableComponent = nullptr;
	}
}

void UInteractionComponent::BeginInteract()
{
	// verify nothing has changed with the iteractable state since beginning interaction
	PerformInteractionCheck();

	if (!InteractionData.CurrentInteractable)
	{
		return;
	}

	if (IsValid(TargetInteractableComponent))
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_Interaction))
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Interaction);
			return;
		}
		
		if (CurrentInteractableComponent && CurrentInteractableComponent == TargetInteractableComponent)
		{
			StopInteract();
			return;
		}
		else
		{
			StopInteract();
			TargetInteractableComponent->BeginInteract(this);
		}
		
		if (FMath::IsNearlyZero(TargetInteractableComponent->InteractableData.InteractableDuration, 0.1f))
		{
			Interact();
		}
		else
		{
			InteractionStartTime = GetWorld()->GetTimeSeconds();
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_Interaction,
											this, &UInteractionComponent::Interact,
											TargetInteractableComponent->InteractableData.InteractableDuration,
											false);
		}
	}
}

void UInteractionComponent::EndInteract()
{
	if (IsValid(TargetInteractableComponent) && TargetInteractableComponent->InteractableData.bHoldToInteract)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Interaction);	
		
		TargetInteractableComponent->EndInteract(this);
		EndIteractNotify();
		return;
		
	}
	
}

void UInteractionComponent::Interact()
{	
	CurrentInteractableComponent = TargetInteractableComponent;
	
	if (IsValid(TargetInteractableComponent))
	{
		TargetInteractableComponent->Interact(this);
		IteractNotify();
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_Interaction))
	{
		EndInteract();
	}
}

void UInteractionComponent::StopInteract()
{
	if (!CurrentInteractableComponent)
		return;
	
	if (StopIteractDelegate.IsBound())
		StopIteractDelegate.Broadcast(CurrentInteractableComponent);

	CurrentInteractableComponent->StopInteract(this);
	CurrentInteractableComponent = nullptr;
}

void UInteractionComponent::IteractNotify()
{
	if (IteractableDataDelegate.IsBound())
		IteractableDataDelegate.Broadcast(TargetInteractableComponent);
}

void UInteractionComponent::EndIteractNotify()
{
	if (IteractableDataDelegate.IsBound())
		EndIteractDelegate.Broadcast(TargetInteractableComponent);
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckInterval)
	{
		PerformInteractionCheck();
	}

	if (TargetInteractableComponent && GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_Interaction))
	{
		float Elapsed = GetWorld()->GetTimeSeconds() - InteractionStartTime;
		float Progress = FMath::Clamp(Elapsed / TargetInteractableComponent->InteractableData.InteractableDuration, 0.0f, 1.0f);
		if(OnInteractionProgress.IsBound())
			OnInteractionProgress.Broadcast(Progress);
	}
	else
	{
		if(OnInteractionProgress.IsBound())
			OnInteractionProgress.Broadcast(0);
	}
}
