// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/InteractionComponent.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/InteractionInterface.h"

UInteractionComponent::UInteractionComponent(): TraceChannel()
{
	PrimaryComponentTick.bCanEverTick = true;
	InteractionCheckInterval = 0.1f;
	InteractionCheckDistance = 325.0f;
	BaseEyeHeight = 74.0f;

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

	Input->BindAction(InteractAction, ETriggerEvent::Triggered, this, &UInteractionComponent::BeginInteract);
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

	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 2.0f, 0, 2.0f);
	if (GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, TraceChannel, QueryParams))
	{
		AActor* HitActor = TraceHit.GetActor();

		if (HitActor && HitActor->Implements<UInteractionInterface>())
		{
			if (HitActor != InteractionData.CurrentInteractable)
			{
				FoundInteractable(HitActor);
			}
			return;
		}
	}

	NotFoundInteractable();
}

void UInteractionComponent::FoundInteractable(AActor* NewInteractable)
{
	if (InteractionData.CurrentInteractable)
	{
		if (InteractionData.CurrentInteractable != InteractionData.LastInteractable)
		{
			InteractionData.LastInteractable = InteractionData.CurrentInteractable;
		}

		TargetInteractable = InteractionData.CurrentInteractable;
		TargetInteractable->EndFocus();
	}

	InteractionData.CurrentInteractable = NewInteractable;
	TargetInteractable = NewInteractable;

	//if (TargetInteractable)
	//	InventoryHUDComponent->UpdateInteractionWidget(TargetInteractable->InteractableData);

	TargetInteractable->BeginFocus();
	if (BeginFocusDelegate.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("BeginFocusDelegate вызван!"));
		BeginFocusDelegate.Broadcast(InteractionData);
	}
}

void UInteractionComponent::NotFoundInteractable()
{
	/*if (IsInteracting())
	{
		GetOwner()->GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);
	}*/

	if (InteractionData.CurrentInteractable)
	{
		if (IsValid(TargetInteractable.GetObject()))
		{
			TargetInteractable->EndFocus();
		}

		//InventoryHUDComponent->HideInteractionWidget();
		InteractionData.CurrentInteractable = nullptr;
		InteractionData.LastInteractable = InteractionData.CurrentInteractable;
		TargetInteractable = nullptr;
	}

	if (EndFocusDelegate.IsBound())
		EndFocusDelegate.Broadcast(InteractionData);
}

void UInteractionComponent::BeginInteract()
{
}

void UInteractionComponent::EndInteract()
{
}

void UInteractionComponent::Interact()
{
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckInterval)
	{
		PerformInteractionCheck();
	}
}
