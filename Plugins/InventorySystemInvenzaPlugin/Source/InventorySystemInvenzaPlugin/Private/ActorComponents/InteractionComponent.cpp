//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/InteractionComponent.h"

#include "EnhancedInputComponent.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "TimerManager.h"      
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Utility/InputUtility.h"


UInteractionComponent::UInteractionComponent(): TargetInteractableComponent(nullptr),
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
	}, 1.5f, false);
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
		float Progress = FMath::Clamp(Elapsed / TargetInteractableComponent->GetInteractableData().InteractableDuration, 0.0f, 1.0f);
		if(OnInteractionProgress.IsBound())
			OnInteractionProgress.Broadcast(Progress);
	}
	else
	{
		if(OnInteractionProgress.IsBound())
			OnInteractionProgress.Broadcast(0);
	}
}

void UInteractionComponent::InitInteractionComponent()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
		return;

	if (auto CameraComp = OwnerPawn->FindComponentByClass<UCameraComponent>())
		CameraComponent = CameraComp;

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent);
	if (!Input) return;

	for (const FInteractionKeyBinding& Binding : KeyBindings)
	{
		if (!Binding.Action) continue;

		Input->BindAction(Binding.Action, ETriggerEvent::Started, this, &UInteractionComponent::BeginInteract, Binding.Type);
		Input->BindAction(Binding.Action, ETriggerEvent::Completed, this, &UInteractionComponent::EndInteract, Binding.Type);
	}
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

	const TArray<FInteractionDisplayEntry> DisplayEntry = BuildDisplayEntries(TargetInteractableComponent);
	OnBeginFocus.Broadcast(DisplayEntry);
	
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

		if (OnEndFocus.IsBound())
		{
			OnEndFocus.Broadcast(BuildDisplayEntries(InteractionData.CurrentInteractable));
		}
       
		InteractionData.CurrentInteractable = nullptr;    
		InteractionData.LastInteractable = InteractionData.CurrentInteractable;
		TargetInteractableComponent = nullptr;
	}
}

void UInteractionComponent::BeginInteract(EInteractionType Type)
{
	// verify nothing has changed with the iteractable state since beginning interaction
	PerformInteractionCheck();

	if (!InteractionData.CurrentInteractable || !IsValid(TargetInteractableComponent))
		return;

	const FInteractableData* Data = TargetInteractableComponent->GetInteractableDataForType(Type);
	if (!Data) return; 

	if (CurrentInteractableComponent && CurrentInteractableComponent == TargetInteractableComponent
		&& ActiveInteractionType == Type)
	{
		StopInteract();
		return;
	}

	if (TargetInteractableComponent->IsInteracting())
	{
		BusyNotify();
		return;
	}

	StopInteract();
	PendingInteractionType = Type;

	TargetInteractableComponent->BeginInteract(this, Type);

	if (FMath::IsNearlyZero(Data->InteractableDuration, 0.1f))
	{
		Interact();
	}
	else
	{
		InteractionStartTime = GetWorld()->GetTimeSeconds();
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Interaction, this, &UInteractionComponent::Interact, Data->InteractableDuration, false);
	}
}

void UInteractionComponent::EndInteract(EInteractionType Type)
{
	if (IsValid(TargetInteractableComponent))
	{
		const FInteractableData* Data = TargetInteractableComponent->GetInteractableDataForType(Type);
		if (Data && Data->bHoldToInteract)
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Interaction);
			TargetInteractableComponent->EndInteract(this, Type);
			EndInteractNotify();
		}
	}
}

void UInteractionComponent::Interact()
{	
	CurrentInteractableComponent = TargetInteractableComponent;
	ActiveInteractionType = PendingInteractionType;

	if (IsValid(TargetInteractableComponent))
	{
		TargetInteractableComponent->HandleInteract(this);
		InteractNotify();
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_Interaction))
		EndInteract(PendingInteractionType);
}

void UInteractionComponent::StopInteract()
{
	if (!CurrentInteractableComponent) return;

	OnStopInteract.Broadcast(CurrentInteractableComponent, ActiveInteractionType);
	CurrentInteractableComponent->HandleStopInteract(this, ActiveInteractionType);
	CurrentInteractableComponent = nullptr;
}

void UInteractionComponent::InteractNotify()
{
	OnInteract.Broadcast(TargetInteractableComponent, PendingInteractionType);
}

void UInteractionComponent::EndInteractNotify()
{
	OnEndInteract.Broadcast(TargetInteractableComponent);
}

void UInteractionComponent::BusyNotify()
{
	OnInteractableBusy.Broadcast();
}

void UInteractionComponent::CollectInteractionActions()
{
	AvailableInteractions.Reset();
	DefaultInteractionData = FInteractableData();
	
	if (!CurrentInteractableComponent)
	{
		return;
	}

	AActor* TargetActor = CurrentInteractableComponent->GetOwner();

	if (!TargetActor)
	{
		return;
	}

	TArray<UActorComponent*> Components;
	TargetActor->GetComponents(Components);

	for (UActorComponent* Component : Components)
	{
		if (!Component)
		{
			continue;
		}

		UInteractableComponent* InteractableComponent =	Cast<UInteractableComponent>(Component);
		if (!InteractableComponent)
		{
			continue;
		}

		auto Data = InteractableComponent->GetInteractableData();
		AvailableInteractions.Add(Data);
	}

	if (AvailableInteractions.Num() > 0)
	{
		DefaultInteractionData = AvailableInteractions[0];
	}
}

TArray<FInteractionDisplayEntry> UInteractionComponent::BuildDisplayEntries(UInteractableComponent* Target) const
{
	TArray<FInteractionDisplayEntry> Result;
	if (!Target) return Result;

	const auto& DataMap = Target->GetInteractableDataMap();

	for (const FInteractionKeyBinding& Binding : KeyBindings)
	{
		const FInteractableData* Data = DataMap.Find(Binding.Type);
		if (!Data) continue;

		FInteractionDisplayEntry Entry;
		Entry.KeyLabel = UInputUtility::GetKeyForAction(GetWorld(), Binding.Action);
		Entry.Data = *Data;
		Result.Add(Entry);
	}

	return Result;
}
