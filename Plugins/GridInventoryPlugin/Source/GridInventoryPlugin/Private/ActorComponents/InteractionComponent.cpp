//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/InteractionComponent.h"

#include "EnhancedInputComponent.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"


UInteractionComponent::UInteractionComponent(): TraceChannel(), TargetInteractableComponent(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	InteractionCheckInterval = 0.1f;
	InteractionCheckDistance = 500.0f;
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

void UInteractionComponent::DropItem(UItemBase* ItemToDrop)
{
	if (!ItemToDrop)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item to drop was somehow null"));
		return;
	}

	if (!RegularSettings.PickupClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item to drop was somehow null"));
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.bNoFail = true;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	const FVector SpawnLocation{GetOwner()->GetActorLocation() + (GetOwner()->GetActorForwardVector() * 50.0f)};
	const FTransform SpawnTransform(GetOwner()->GetActorRotation(), SpawnLocation);

	auto Pickup = GetWorld()->SpawnActor<AActor>(RegularSettings.PickupClass, SpawnTransform, SpawnParameters);
	if (auto PickupComponent = Pickup->FindComponentByClass<UPickupComponent>())
	{
		PickupComponent->InitializeDrop(ItemToDrop);
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

	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 2.0f, 0, 2.0f);
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
			InteractionData.LastInteractable = InteractionData.CurrentInteractable;
		}

		TargetInteractableComponent = InteractionData.CurrentInteractable;
		TargetInteractableComponent->EndFocus();
	}

	InteractionData.CurrentInteractable = NewInteractableComp;
	TargetInteractableComponent = NewInteractableComp;

	//if (TargetInteractable)
	//	InventoryHUDComponent->UpdateInteractionWidget(TargetInteractable->InteractableData);

	TargetInteractableComponent->BeginFocus();
	if (BeginFocusDelegate.IsBound())
	{
		BeginFocusDelegate.Broadcast(TargetInteractableComponent->InteractableData);
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
		if (IsValid(TargetInteractableComponent))
		{
			TargetInteractableComponent->EndFocus();
		}

		//InventoryHUDComponent->HideInteractionWidget();

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
		TargetInteractableComponent->BeginInteract(this);

		if (FMath::IsNearlyZero(TargetInteractableComponent->InteractableData.InteractableDuration, 0.1f))
		{
			Interact();
		}
		else
		{
			GetOwner()->GetWorldTimerManager().SetTimer(TimerHandle_Interaction,
											this, &UInteractionComponent::Interact,
											TargetInteractableComponent->InteractableData.InteractableDuration,
											false);
		}
	}
}

void UInteractionComponent::EndInteract()
{
	GetOwner()->GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);	
	
	if (IsValid(TargetInteractableComponent))
	{
		TargetInteractableComponent->EndInteract(this);
	}
}

void UInteractionComponent::Interact()
{
	GetOwner()->GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);

	if (IsValid(TargetInteractableComponent))
	{
		TargetInteractableComponent->Interact(this);
		IteractNotify();
	}
}

void UInteractionComponent::IteractNotify()
{
	if (IteractableDataDelegate.IsBound())
		IteractableDataDelegate.Broadcast(TargetInteractableComponent);
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
