//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/InteractableContainerComponent.h"

#include "ToolBuilderUtil.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Items/itemBase.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/Container/InvBaseContainerWidget.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"

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
	InitializeItemCollection();	

	InventoryWidget = FindContainerWidget();
	if (!InventoryWidget) return;
	
	if (bContainerIsOpen == false)
	{
		ContainerWidget->SetVisibility(ESlateVisibility::Visible);
		InventoryWidget->ReDrawAllItems();
		bContainerIsOpen = true;
	}
	else
	{
		ContainerWidget->SetVisibility(ESlateVisibility::Collapsed);
		bContainerIsOpen = false;
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
	InventoryWidget = FindContainerWidget();
	if (!InventoryWidget) return;
	InventoryWidget->SetItemCollection(ItemCollection);

	if (ItemCollection->InitItems.IsEmpty())
		return;
	
	for (const auto& Item : ItemCollection->InitItems)
	{
		FItemMoveData ItemMoveData;
		ItemMoveData.SourceItem = UItemBase::CreateFromDataTable(ItemCollection->ItemDataTable, Item.ItemName, Item.ItemCount);
		if (ItemMoveData.SourceItem)
		{
			InventoryWidget->HandleAddItem(ItemMoveData);
		}
	}
	
	ItemCollection->InitItems.Empty();
}

void UInteractableContainerComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();

	InteractableData.DefaultInteractableType = EInteractableType::Container;
	InteractableData.Action = FText::FromString(TEXT("Open"));
	InteractableData.Quantity = -1;
}

USlotbasedInventoryWidget* UInteractableContainerComponent::FindContainerWidget()
{
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UInvBaseContainerWidget::StaticClass(), false);

	for (UUserWidget* Widget : FoundWidgets)
	{
		if (Widget->GetFName() == ContainerWidgetName)
		{
			auto InvBaseContainerWidget = Cast<UInvBaseContainerWidget>(Widget);
			ContainerWidget = InvBaseContainerWidget;
			return InvBaseContainerWidget->GetInventoryFromContainerSlot();
		}
	}

	return nullptr;
}
