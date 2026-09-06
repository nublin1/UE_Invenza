// Nublin Studio 2026 All Rights Reserved.


#include "ActorComponents/Interactable/CraftingStationComponent.h"

#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/Crafting/CraftingComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Utility/InvenzayUtility.h"


UCraftingStationComponent::UCraftingStationComponent()
{
	SetIsReplicatedByDefault(true);
}

void UCraftingStationComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	FTimerHandle InitTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(InitTimerHandle, FTimerDelegate::CreateLambda([this]()
	{
		InitializeInteractionComponent();
	}),  1.0f, false);
	
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
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		//UE_LOG(LogTemp, Warning, TEXT("[%s] InitializeCraftingStation called without authority — ignoring."), *GetName());
		return;
	}
	
	CraftingComponentRef = nullptr;
	ItemCollectionRef = nullptr;

	if (!IsValid(ContextActor))
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to initialize crafting station: ContextActor is invalid."),
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
		auto CraftConfig = CraftingComponentRef->GetConfig();
		
		auto FindInventory = [this](const FGameplayTag& OverrideTag, const FGameplayTag& DefaultTag) -> UInventoryBase*
		{
			if (OverrideTag.IsValid())
			{
				if (UInventoryBase* Inventory = ItemCollectionRef->GetInventoryByTag(OverrideTag))
				{
					return Inventory;
				}
			}

			if (DefaultTag.IsValid())
			{
				if (UInventoryBase* Inventory = ItemCollectionRef->GetInventoryByTag(DefaultTag))
				{
					return Inventory;
				}
			}

			return nullptr;
		};

		UInventoryBase* InputInventory = FindInventory(CraftConfig.InputInventoryTag, GSettings->InputInvTagByDefault);
		UInventoryBase* OutputInventory = FindInventory(CraftConfig.OutputInventoryTag, GSettings->OutputInvTagByDefault);
		UInventoryBase* FuelInventory = FindInventory(CraftConfig.FuelInventoryTag, GSettings->FuelInvTagByDefault);

		if (IsValid(InputInventory))
			CraftingComponentRef->SetInputInventory(InputInventory);

		if (IsValid(OutputInventory))
			CraftingComponentRef->SetOutputInventory(OutputInventory);
		
		if (IsValid(FuelInventory))
			CraftingComponentRef->SetFuelInventory(FuelInventory);
	}
	
	CraftingComponentRef->RequestInitCraftingComponent();
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
