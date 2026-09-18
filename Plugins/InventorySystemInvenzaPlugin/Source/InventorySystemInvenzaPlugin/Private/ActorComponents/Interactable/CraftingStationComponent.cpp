// Nublin Studio 2026 All Rights Reserved.


#include "ActorComponents/Interactable/CraftingStationComponent.h"

#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/Crafting/CraftingComponent.h"
#include "Components/WidgetComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Interface/World/WorldCraft_WidProvider.h"
#include "Net/UnrealNetwork.h"
#include "Utility/InvenzayUtility.h"


UCraftingStationComponent::UCraftingStationComponent()
{
	SetIsReplicatedByDefault(true);
	
	if (!InteractableDataMap.Contains(EInteractionType::Primary))
	{
		FInteractableData PrimaryData;
		PrimaryData.DefaultInteractableType = EInteractableType::Craft;
		PrimaryData.Action = FText::FromString(TEXT("Open Craft"));
		PrimaryData.ActiveAction = FText::FromString(TEXT("Close Craft"));
		PrimaryData.Quantity = -1;
		InteractableDataMap.Add(EInteractionType::Primary, PrimaryData);
	}
	
	if (!InteractableDataMap.Contains(EInteractionType::Secondary))
	{
		FInteractableData SecondaryData;
		SecondaryData.DefaultInteractableType = EInteractableType::Craft;
		SecondaryData.Action = FText::FromString(TEXT("Work"));
		SecondaryData.ActiveAction = FText::FromString(TEXT("Stop Work"));
		SecondaryData.bHoldToInteract = false;
		SecondaryData.Quantity = -1;
		InteractableDataMap.Add(EInteractionType::Secondary, SecondaryData);
	}
}

void UCraftingStationComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UCraftingStationComponent, CraftingComponentLink);
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

void UCraftingStationComponent::HandleInteract(UInteractionComponent* InteractionComponent)
{
	if (!InteractionComponent)
		return;
	
	Super::HandleInteract(InteractionComponent);
	
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

void UCraftingStationComponent::HandleStopInteract(UInteractionComponent* InteractionComponent, EInteractionType Type)
{
	Super::HandleStopInteract(InteractionComponent, Type);
	
	SetInteracting(false);
	CurrentInteractionComponent = nullptr;
	if (bUseInteractorInventory)
	{
		CraftingComponentLink = nullptr;
		ItemCollectionRef = nullptr;
	}
}

void UCraftingStationComponent::OnRep_CraftingComponentLink()
{
	BindProgressWidget();
}

void UCraftingStationComponent::BindProgressWidget()
{
	if (!CraftingComponentLink) return;

	UWidgetComponent* WidgetComp = ProgressWidgetComponentLink
		                               ? ProgressWidgetComponentLink.Get()
		                               : GetOwner()->FindComponentByClass<UWidgetComponent>();

	if (!WidgetComp) return;

	if (IWorldCraft_WidProvider* Display = Cast<IWorldCraft_WidProvider>(WidgetComp->GetUserWidgetObject()))
	{
		Display->SetCraftComponentPtr(CraftingComponentLink);
	}
}

void UCraftingStationComponent::InitializeCraftingStation(AActor* ContextActor)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		//UE_LOG(LogTemp, Warning, TEXT("[%s] InitializeCraftingStation called without authority — ignoring."), *GetName());
		return;
	}
	
	CraftingComponentLink = nullptr;
	ItemCollectionRef = nullptr;

	if (!IsValid(ContextActor))
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to initialize crafting station: ContextActor is invalid."),
			*GetName());

		return;
	}

	CraftingComponentLink = ContextActor->FindComponentByClass<UCraftingComponent>();
	if (!CraftingComponentLink)
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
		auto CraftConfig = CraftingComponentLink->GetConfig();
		
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
			CraftingComponentLink->SetInputInventory(InputInventory);

		if (IsValid(OutputInventory))
			CraftingComponentLink->SetOutputInventory(OutputInventory);
		
		if (IsValid(FuelInventory))
			CraftingComponentLink->SetFuelInventory(FuelInventory);
	}
	
	CraftingComponentLink->RequestInitCraftingComponent();
}

void UCraftingStationComponent::InitializeInteractionComponent()
{
	InitializeCraftingStation(GetOwner());
}

void UCraftingStationComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();
}
