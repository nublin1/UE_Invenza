// Nublin Studio 2026 All Rights Reserved.


#include "ActorComponents/Interactable/CraftingStationComponent.h"

#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/Crafting/CraftingComponent.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Utility/InvenzayUtility.h"


UCraftingStationComponent::UCraftingStationComponent()
{
	SetIsReplicatedByDefault(true);
}

void UCraftingStationComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeInteractionComponent();
}

void UCraftingStationComponent::Interact(UInteractionComponent* InteractionComponent)
{
	if (!InteractionComponent)
		return;
	
	Super::Interact(InteractionComponent);
	
	CurrentInteractionComponent = InteractionComponent;
	if (bIsInteracting == false)
	{
		if (bUseInteractorInventory)
		{
			AActor* InteractorActor = InteractionComponent->GetOwner();
			InitializeCraftingStation(InteractorActor);
			CurrentInteractionComponent = nullptr;
			return;
			
		}

		SetInteracting(true);
	}
	else
	{
		SetInteracting(false);
	}
}

void UCraftingStationComponent::StopInteract(UInteractionComponent* InteractionComponent)
{
	Super::StopInteract(InteractionComponent);
	
	SetInteracting(false);
	CurrentInteractionComponent = nullptr;
	if (bUseInteractorInventory)
	{
		CraftingComponentRef = nullptr;
		ItemCollectionRef = nullptr;
	}
}

void UCraftingStationComponent::InitializeCraftingStation(AActor* ContextActor)
{
	CraftingComponentRef = nullptr;
	ItemCollectionRef = nullptr;

	if (!IsValid(ContextActor))
	{
		UE_LOG(LogTemp, Error,
			TEXT("[%s] Failed to initialize crafting station: ContextActor is invalid."),
			*GetName());

		return;
	}

	CraftingComponentRef = ContextActor->FindComponentByClass<UCraftingComponent>();
	if (!CraftingComponentRef)
	{
		UE_LOG(LogTemp,	Error, TEXT("[%s] Actor '%s' has UCraftingStationComponent, but no UCraftingComponent was found."),
			*GetName(),	*ContextActor->GetName());
		return;
	}

	if (bUseInteractorInventory == false)
	{
		ItemCollectionRef = ContextActor->FindComponentByClass<UItemCollection>();
		if (!ItemCollectionRef)
		{
			UE_LOG(LogTemp,	Error, TEXT("[%s] ItemCollectionRef no was found '%s"),
				*GetName(),	*ContextActor->GetName());
		}
		
		auto GSettings = UInvenzayUtility::GetInvenzaGlobalSettings(GetWorld());
		
		const auto InputInventoryTag = GSettings->InputInvTagByDefault;
		const auto OutputInventoryTag = GSettings->OutputInvTagByDefault;
		const auto FuelInventoryTag = GSettings->FuelInvTagByDefault;
		
		UInventoryBase* InputInventory = ItemCollectionRef->GetInventoryByTag(InputInventoryTag);
		UInventoryBase* OutputInventory = ItemCollectionRef->GetInventoryByTag(OutputInventoryTag);
		UInventoryBase* FuelInventory = ItemCollectionRef->GetInventoryByTag(FuelInventoryTag);
		if (!InputInventory || !OutputInventory || !FuelInventory)
		{
			return;
		}
		
		CraftingComponentRef->SetInputInventory(InputInventory);
		CraftingComponentRef->SetOutputInventory(OutputInventory);
		CraftingComponentRef->SetFuelInventory(FuelInventory);
		
	}
}

void UCraftingStationComponent::InitializeInteractionComponent()
{
	InitializeCraftingStation(GetOwner());
}

void UCraftingStationComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();

	InteractableData.DefaultInteractableType = EInteractableType::Craft;
	InteractableData.Action = FText::FromString(TEXT("Open Craft"));
	InteractableData.Quantity = -1;
}
