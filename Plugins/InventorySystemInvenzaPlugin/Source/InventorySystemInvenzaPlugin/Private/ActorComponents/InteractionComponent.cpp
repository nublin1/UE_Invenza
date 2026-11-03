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
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "Engine/GameInstance.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"

UInteractionComponent::UInteractionComponent()
	: TargetInteractableComponent(nullptr), CurrentInteractableComponent(nullptr)
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
	InteractionCheckInterval = 0.1f;
	InteractionCheckDistance = 500.0f;
	TraceChannel = GetDefault<UInvenzaInventoryDeveloperSettings>()->InteractionTraceChannel;

	ComponentTags.Add(FName("InteractionComponent"));
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(InteractionInitTimer, this, &ThisClass::InitInteractionComponent, 1.5f,
										   false);
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopInteract();
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

bool UInteractionComponent::IsWithinInteractionDistance(const AActor* Interactor, const UInteractableComponent* Target)
{
	if (!IsValid(Interactor) || Interactor->IsActorBeingDestroyed() || !IsValid(Target) || Target->IsBeingDestroyed() ||
		!IsValid(Target->GetOwner()) || Target->GetOwner()->IsActorBeingDestroyed() ||
		Interactor->GetWorld() != Target->GetWorld())
	{
		return false;
	}
	const UInvenzaInventorySettingsAsset* Settings = nullptr;
	if (const auto* GI = Interactor->GetGameInstance())
	{
		if (const auto* Subsystem = GI->GetSubsystem<UInvenzaInventorySettingsSubsystem>())
		{
			Settings = Subsystem->GetSettings();
		}
	}
	if (!Settings)
	{
		Settings = GetDefault<UInvenzaInventorySettingsAsset>();
	}
	const float Distance = FMath::Max(1.f, Settings->MaxInteractionDistance);
	return FVector::DistSquared(Interactor->GetActorLocation(), Target->GetOwner()->GetActorLocation()) <=
		   FMath::Square(Distance);
}

void UInteractionComponent::StartInteractionRangeTimer()
{
	if (GetOwner()->HasAuthority() && CurrentInteractableComponent &&
		!GetWorld()->GetTimerManager().IsTimerActive(InteractionRangeTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(InteractionRangeTimer, this, &ThisClass::CheckInteractionRange, 0.25f,
											   true);
	}
}

void UInteractionComponent::CheckInteractionRange()
{
	if (!IsWithinInteractionDistance(GetOwner(), CurrentInteractableComponent) ||
		(!bServerSessionCompleted && GetWorld()->GetTimeSeconds() - ServerInteractionStartTime >
			ServerInteractionDuration + FMath::Max(0.25f, CompletionRequestGracePeriod)))
	{
		StopInteract();
	}
}

void UInteractionComponent::TrackActiveInteraction(UInteractableComponent* Target, EInteractionType Type)
{
	if (!GetOwner()->HasAuthority() || !IsWithinInteractionDistance(GetOwner(), Target) ||
		!Target->TryAcquireInteraction(GetOwner()))
	{
		return;
	}
	if (CurrentInteractableComponent != Target || ActiveInteractionType != Type)
	{
		StopInteract();
		if (!Target->TryAcquireInteraction(GetOwner()))
		{
			return;
		}
		CurrentInteractableComponent = Target;
		ActiveInteractionType = Type;
		++ActiveInteractionSerial;
	}
	StartInteractionRangeTimer();
}

void UInteractionComponent::Server_BeginInteraction_Implementation(UInteractableComponent* Target,
	EInteractionType Type, uint32 Serial)
{
	if (Serial == 0 || static_cast<int32>(Serial - LastAcceptedBeginSerial) <= 0)
	{
		return;
	}
	LastAcceptedBeginSerial = Serial;
	const auto* Data = IsValid(Target) ? Target->GetInteractableDataForType(Type) : nullptr;
	if (!Data || !FMath::IsFinite(Data->InteractableDuration) ||
		!IsWithinInteractionDistance(GetOwner(), Target) || Target->IsInteractionLockedByOther(GetOwner()))
	{
		Client_StopInteraction(Target, Type, Serial);
		return;
	}
	// The initiating authority already set this local serial before entering the protocol.
	if (CurrentInteractableComponent && (CurrentInteractableComponent != Target || ActiveInteractionSerial != Serial))
	{
		StopInteract();
	}
	if (!Target->TryAcquireInteraction(GetOwner()))
	{
		Client_StopInteraction(Target, Type, Serial);
		return;
	}
	CurrentInteractableComponent = Target;
	ActiveInteractionType = Type;
	ActiveInteractionSerial = Serial;
	ServerInteractionStartTime = GetWorld()->GetTimeSeconds();
	ServerInteractionDuration = FMath::Max(0.f, Data->InteractableDuration);
	bServerSessionCompleted = false;
	bApprovedInteractionConsumed = false;
	StartInteractionRangeTimer();
}

void UInteractionComponent::Server_TrackInteraction_Implementation(UInteractableComponent* Target,
	EInteractionType Type, uint32 Serial)
{
	if (bServerSessionCompleted || CurrentInteractableComponent != Target ||
		ActiveInteractionType != Type || ActiveInteractionSerial != Serial)
	{
		UE_LOG(LogTemp, Log, TEXT("Interaction completion rejected: missing, stale or completed session."));
		return;
	}
	if (!IsWithinInteractionDistance(GetOwner(), Target) || !Target->GetInteractableDataForType(Type) ||
		!Target->IsInteractionOwnedBy(GetOwner()))
	{
		StopInteract();
		return;
	}
	const double Remaining = ServerInteractionDuration - (GetWorld()->GetTimeSeconds() - ServerInteractionStartTime);
	if (Remaining > 0.0)
	{
		// A fast completion request cannot shorten the server hold, including network jitter.
		UE_LOG(LogTemp, Verbose, TEXT("Interaction completion deferred: %.3f seconds remain."), Remaining);
		GetWorld()->GetTimerManager().SetTimer(ServerCompletionTimer, this,
			&ThisClass::FinishServerInteraction, static_cast<float>(FMath::Max(Remaining, 0.001)), false);
		return;
	}
	FinishServerInteraction();
}

void UInteractionComponent::FinishServerInteraction()
{
	if (bServerSessionCompleted || !IsWithinInteractionDistance(GetOwner(), CurrentInteractableComponent) ||
		!CurrentInteractableComponent->IsInteractionOwnedBy(GetOwner()) ||
		!CurrentInteractableComponent->GetInteractableDataForType(ActiveInteractionType))
	{
		StopInteract();
		return;
	}
	if (GetWorld()->GetTimeSeconds() - ServerInteractionStartTime < ServerInteractionDuration)
	{
		Server_TrackInteraction_Implementation(CurrentInteractableComponent, ActiveInteractionType, ActiveInteractionSerial);
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(ServerCompletionTimer);
	bServerSessionCompleted = true;
	bAwaitingCompletion = false;
	UInteractableComponent* Target = CurrentInteractableComponent;
	const auto Type = ActiveInteractionType;
	const uint32 Serial = ActiveInteractionSerial;
	TargetInteractableComponent = Target;
	PendingInteractionType = Type;
	{
		TGuardValue<bool> ApprovalGuard(bExecutingApprovedInteraction, true);
		CompleteInteraction(true);
	}
	if (IsValid(Target) && CurrentInteractableComponent == Target)
	{
		Client_ConfirmInteraction(Target, Type, Serial);
	}
}

bool UInteractionComponent::ConsumeApprovedInteraction(UInteractableComponent* Target, EInteractionType Type)
{
	if (!GetOwner()->HasAuthority() || !bExecutingApprovedInteraction || bApprovedInteractionConsumed ||
		!bServerSessionCompleted || CurrentInteractableComponent != Target || ActiveInteractionType != Type)
	{
		return false;
	}
	bApprovedInteractionConsumed = true;
	return true;
}

void UInteractionComponent::Client_ConfirmInteraction_Implementation(UInteractableComponent* Target,
	EInteractionType Type, uint32 Serial)
{
	if (GetOwner()->HasAuthority() || !IsValid(Target) || CurrentInteractableComponent != Target || ActiveInteractionType != Type ||
		ActiveInteractionSerial != Serial)
	{
		return;
	}
	bAwaitingCompletion = false;
	TargetInteractableComponent = Target;
	PendingInteractionType = Type;
	CompleteInteraction(false);
}

void UInteractionComponent::Server_StopInteraction_Implementation(UInteractableComponent* Target, EInteractionType Type,
																  uint32 Serial)
{
	if (CurrentInteractableComponent == Target && ActiveInteractionType == Type && ActiveInteractionSerial == Serial)
	{
		StopInteract();
	}
}

void UInteractionComponent::Client_StopInteraction_Implementation(UInteractableComponent* Target, EInteractionType Type,
																  uint32 Serial)
{
	// A destroyed target may no longer resolve on the client; the serial still identifies the session.
	if ((!IsValid(Target) || CurrentInteractableComponent == Target) && ActiveInteractionType == Type &&
		ActiveInteractionSerial == Serial)
	{
		TGuardValue<bool> Guard(bApplyingServerStop, true);
		StopInteract();
	}
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
		const auto* Data = TargetInteractableComponent->GetInteractableDataForType(PendingInteractionType);
		const float Progress = Data && Data->InteractableDuration > SMALL_NUMBER
			? FMath::Clamp(Elapsed / Data->InteractableDuration, 0.0f, 1.0f) : 0.f;
		if (OnInteractionProgress.IsBound())
		{
			OnInteractionProgress.Broadcast(Progress);
		}
	}
	else
	{
		if (OnInteractionProgress.IsBound())
		{
			OnInteractionProgress.Broadcast(0);
		}
	}
}

void UInteractionComponent::InitInteractionComponent()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	if (auto CameraComp = OwnerPawn->FindComponentByClass<UCameraComponent>())
	{
		CameraComponent = CameraComp;
	}

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent);
	if (!Input)
	{
		return;
	}

	for (const FInteractionKeyBinding& Binding : KeyBindings)
	{
		if (!Binding.Action)
		{
			continue;
		}

		Input->BindAction(Binding.Action, ETriggerEvent::Started, this, &UInteractionComponent::BeginInteract,
						  Binding.Type);
		Input->BindAction(Binding.Action, ETriggerEvent::Completed, this, &UInteractionComponent::EndInteract,
						  Binding.Type);
	}
}

void UInteractionComponent::RefreshInteractionDisplay()
{
	if (IsValid(TargetInteractableComponent))
	{
		OnInteractionDisplayChanged.Broadcast(BuildDisplayEntries(TargetInteractableComponent));
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

		UInteractableComponent* InteractableComp =
			IsValid(HitActor) ? HitActor->FindComponentByClass<UInteractableComponent>() : nullptr;

		if (IsValid(InteractableComp))
		{
			if (InteractableComp != InteractionData.CurrentInteractable)
			{
				FoundInteractable(HitActor, InteractableComp);
			}

			return;
		}
	}

	NotFoundInteractable();
}

void UInteractionComponent::FoundInteractable(AActor* NewInteractable, UInteractableComponent* NewInteractableComp)
{
	if (bAwaitingCompletion && CurrentInteractableComponent != NewInteractableComp)
	{
		StopInteract();
	}
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
	if (bAwaitingCompletion)
	{
		StopInteract();
	}
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
	PerformInteractionCheck();
	if (InteractionData.CurrentInteractable)
	{
		RequestInteraction(TargetInteractableComponent, Type);
	}
}

void UInteractionComponent::RequestInteraction(UInteractableComponent* Target, EInteractionType Type)
{
	if (!IsWithinInteractionDistance(GetOwner(), Target))
	{
		return;
	}
	const auto* Data = Target->GetInteractableDataForType(Type);
	if (!Data || !FMath::IsFinite(Data->InteractableDuration))
	{
		return;
	}
	if (CurrentInteractableComponent == Target && ActiveInteractionType == Type)
	{
		StopInteract();
		return;
	}
	if (Target->IsInteractionLockedByOther(GetOwner()))
	{
		return;
	}
	StopInteract();
	TargetInteractableComponent = Target;
	CurrentInteractableComponent = Target;
	PendingInteractionType = Type;
	ActiveInteractionType = Type;
	++ActiveInteractionSerial;
	bAwaitingCompletion = true;
	bCompletionRequested = false;
	InteractionStartTime = GetWorld()->GetTimeSeconds();
	Server_BeginInteraction(Target, Type, ActiveInteractionSerial);
	if (GetOwner()->HasAuthority() && (bServerSessionCompleted || CurrentInteractableComponent != Target))
	{
		return;
	}
	Target->BeginInteract(this, Type);
	const float Duration = FMath::Max(0.f, Data->InteractableDuration);
	if (Duration <= 0.f)
	{
		Interact();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Interaction, this, &ThisClass::Interact, Duration, false);
	}
	RefreshInteractionDisplay();
}

void UInteractionComponent::EndInteract(EInteractionType Type)
{
	if (IsValid(TargetInteractableComponent))
	{
		const FInteractableData* Data = TargetInteractableComponent->GetInteractableDataForType(Type);
		if (Data && Data->bHoldToInteract)
		{
			if (bAwaitingCompletion && !bCompletionRequested && Type == ActiveInteractionType)
			{
				StopInteract();
			}
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Interaction);
			TargetInteractableComponent->EndInteract(this, Type);
			EndInteractNotify();
			RefreshInteractionDisplay();
		}
	}
}

void UInteractionComponent::Interact()
{
	if (!bAwaitingCompletion || !IsWithinInteractionDistance(GetOwner(), CurrentInteractableComponent))
	{
		return;
	}
	bCompletionRequested = true;
	Server_TrackInteraction(CurrentInteractableComponent, ActiveInteractionType, ActiveInteractionSerial);
}

void UInteractionComponent::CompleteInteraction(bool bNotify)
{
	if (IsValid(TargetInteractableComponent))
	{
		TargetInteractableComponent->HandleInteract(this);
		if (bNotify)
		{
			InteractNotify();
		}
	}
	if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_Interaction))
	{
		EndInteract(PendingInteractionType);
	}
	RefreshInteractionDisplay();
}

void UInteractionComponent::StopInteract()
{
	UInteractableComponent* PreviousTarget = CurrentInteractableComponent;
	const bool bOwnedPreviousTarget = IsValid(PreviousTarget) && PreviousTarget->IsInteractionOwnedBy(GetOwner());
	const EInteractionType PreviousType = ActiveInteractionType;
	const uint32 PreviousSerial = ActiveInteractionSerial;

	// Clear before notifying listeners: a UI close callback may re-enter here.
	CurrentInteractableComponent = nullptr;
	bAwaitingCompletion = false;
	bServerSessionCompleted = true;
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Interaction);
	GetWorld()->GetTimerManager().ClearTimer(ServerCompletionTimer);
	GetWorld()->GetTimerManager().ClearTimer(InteractionRangeTimer);
	if (PreviousTarget && !bApplyingServerStop)
	{
		if (GetOwner()->HasAuthority())
		{
			Client_StopInteraction(PreviousTarget, PreviousType, PreviousSerial);
		}
		else
		{
			Server_StopInteraction(PreviousTarget, PreviousType, PreviousSerial);
		}
	}

	if (PreviousTarget)
	{
		TGuardValue<bool> Guard(bNotifyingStop, true);
		OnStopInteract.Broadcast(PreviousTarget, PreviousType);

		if (IsValid(PreviousTarget))
		{
			if (!GetOwner()->HasAuthority() || bOwnedPreviousTarget)
			{
				PreviousTarget->HandleStopInteract(this, PreviousType);
				PreviousTarget->ReleaseInteraction(GetOwner());
			}
		}
	}

	RefreshInteractionDisplay();
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

		UInteractableComponent* InteractableComponent = Cast<UInteractableComponent>(Component);
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
	if (!Target)
	{
		return Result;
	}

	const auto& DataMap = Target->GetInteractableDataMap();

	for (const FInteractionKeyBinding& Binding : KeyBindings)
	{
		const FInteractableData* Data = DataMap.Find(Binding.Type);
		if (!Data)
		{
			continue;
		}

		FInteractionDisplayEntry Entry;
		Entry.KeyLabel = UInputUtility::GetKeyForAction(GetWorld(), Binding.Action);
		Entry.Data = *Data;
		const bool bHolding = TargetInteractableComponent == Target && PendingInteractionType == Binding.Type &&
			GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_Interaction);
		const bool bActiveForInteractor = bHolding ||
			(CurrentInteractableComponent == Target && ActiveInteractionType == Binding.Type);
		Entry.Data.Action = Target->GetInteractionActionText(Binding.Type, bActiveForInteractor);
		Result.Add(Entry);
	}

	return Result;
}
