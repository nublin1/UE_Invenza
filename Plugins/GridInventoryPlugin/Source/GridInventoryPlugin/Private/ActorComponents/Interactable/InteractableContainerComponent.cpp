// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/Interactable/InteractableContainerComponent.h"

#include "ToolBuilderUtil.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Items/itemBase.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/Inventory/BaseInventoryWidget.h"

UInteractableContainerComponent::UInteractableContainerComponent()
{
}

void UInteractableContainerComponent::BeginFocus()
{
	Super::BeginFocus();
	if (auto StaticMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>())
	{		
		StaticMesh->SetRenderCustomDepth(true);
	}
}

void UInteractableContainerComponent::EndFocus()
{
	Super::EndFocus();
	if (auto StaticMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>())
	{
		StaticMesh->SetRenderCustomDepth(false);
	}
}

void UInteractableContainerComponent::Interact(UInteractionComponent* InteractionComponent)
{
	Super::Interact(InteractionComponent);
	
	if (!ItemCollection) return;

	if (ItemCollection->InitItems.Num()>0)
	{
		InitializeItemCollection();
		return;
	}

	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UUserWidget::StaticClass(), false);
	for (UUserWidget* Widget : FoundWidgets)
	{
		if (Widget->GetFName() != TargetWidgetName)
		{
			continue;
		}

		if (UBaseInventoryWidget* Container = Cast<UBaseInventoryWidget>(Widget))
		{
			Container->ReDrawAllItems();
		}
	}
}

void UInteractableContainerComponent::OnRegister()
{
	Super::OnRegister();
	InitializeContainerComponent();
}

void UInteractableContainerComponent::InitializeContainerComponent()
{
	ItemCollection = GetOwner()->FindComponentByClass<UItemCollection>();
	UpdateInteractableData();
}

void UInteractableContainerComponent::InitializeItemCollection()
{
	if (!ItemCollection)
	{
		return;
	}

	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UUserWidget::StaticClass(), false);
	for (UUserWidget* Widget : FoundWidgets)
	{
		if (Widget->GetFName() != TargetWidgetName)
		{
			continue;
		}

		if (UBaseInventoryWidget* Container = Cast<UBaseInventoryWidget>(Widget))
		{
			Container->SetItemCollection(ItemCollection);
			for (const auto& Item : ItemCollection->InitItems)
			{
				FItemMoveData ItemMoveData;
				ItemMoveData.SourceItem = UItemBase::CreateFromDataTable(ItemCollection->ItemDataTable, Item.Key, Item.Value);
				Container->HandleAddItem(ItemMoveData);
			}
		}
	}
	
	ItemCollection->InitItems.Empty();
}

void UInteractableContainerComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();
	
	InteractableData.DefaultInteractableType = EInteractableType::Container;
	InteractableData.Action =  FText::FromString(TEXT("Open"));
}
