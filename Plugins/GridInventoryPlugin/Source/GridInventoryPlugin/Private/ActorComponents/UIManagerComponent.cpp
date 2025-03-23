// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/UIManagerComponent.h"

#include "EnhancedInputComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/PickupComponent.h"
#include "ActorComponents/Items/itemBase.h"
#include "GameFramework/Character.h"
#include "UI/Core/CoreHUDWidget.h"
#include "UI/Interaction/InteractionWidget.h"
#include "UI/Inventory/BaseInventoryWidget.h"
#include "UI/Layers/InventorySystemLayout.h"


// Sets default values for this component's properties
UUIManagerComponent::UUIManagerComponent(): ToggleMenuAction(nullptr)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UUIManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UUIManagerComponent::BindEvents(AActor* TargetActor)
{
	if (!TargetActor) return;

	
	UInteractionComponent* InteractionComponent = TargetActor->FindComponentByClass<UInteractionComponent>();

	if (!InteractionComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("InteractionComponent is not valid!"));
		return;
	}

	auto InteractionWidget = CoreHUDWidget->GetInteractionWidget();

	if (!IsValid(InteractionWidget))
	{
		
		UE_LOG(LogTemp, Warning, TEXT("InteractionWidget is not valid or pending kill!"));
		return;
	}
	
	//
	InteractionComponent->RegularSettings = this->RegularSettings;
	InteractionComponent->BeginFocusDelegate.AddDynamic(InteractionWidget, &UInteractionWidget::OnFoundInteractable);
	InteractionComponent->EndFocusDelegate.AddDynamic(InteractionWidget, &UInteractionWidget::OnLostInteractable);
	InteractionComponent->IteractableDataDelegate.AddDynamic(this, &UUIManagerComponent::UIIteract);

	UItemCollection* ItemCollection = TargetActor->FindComponentByClass<UItemCollection>();
	if (!ItemCollection) return;

	auto MainInv = CoreHUDWidget->GetInventorySystemLayout()->GetMainInventory();
	for (auto Item : ItemCollection->InitItems)
	{
		FItemMoveData ItemMoveData;
		ItemMoveData.SourceItem = UItemBase::CreateFromDataTable(ItemCollection->ItemDataTable, Item.Key, Item.Value);
		MainInv->HandleAddItem(ItemMoveData);
	}
}

void UUIManagerComponent::UIIteract( UInteractableComponent* TargetInteractableComponent)
{
	if (!TargetInteractableComponent) return;

	if (auto* PickupComp = Cast<UPickupComponent>(TargetInteractableComponent))
	{
		auto Inv = CoreHUDWidget->GetInventorySystemLayout()->GetMainInventory();
		auto Item =  PickupComp->GetItemBase();

		if (!Inv)
		{
			UE_LOG(LogTemp, Warning, TEXT("MainInventory is NULL!"));
			return;
		}
		if (!Item)
		{
			UE_LOG(LogTemp, Warning, TEXT("Item is NULL!"));
			return;
		}

		FItemMoveData Data;
		Data.SourceItem = Item;
		Data.TargetInventory = Inv;
		
		FItemAddResult Result = Inv->HandleAddItem(Data);
		//UE_LOG(LogTemp, Warning, TEXT("USpecialInteractableComponent"));
	}
}

void UUIManagerComponent::InitializeMenuBindings()
{
	if (ToggleMenuAction)
	{
		UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent);
		if (!Input)
			return;

		Input->BindAction(ToggleMenuAction, ETriggerEvent::Started, CoreHUDWidget.Get(), &UCoreHUDWidget::ToggleInventoryMenu);
	}

	
}


void UUIManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

