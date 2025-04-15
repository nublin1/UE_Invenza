//  Nublin Studio 2025 All Rights Reserved.

#include "UI/Core/CoreHUDWidget.h"

#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Items/itemBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "UI/Container/InvBaseContainerWidget.h"
#include "UI/Inrefaces/UDraggableWidgetInterface.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"

UCoreHUDWidget::UCoreHUDWidget(): UISettings()
{
}

void UCoreHUDWidget::InitializeWidget()
{
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UInvBaseContainerWidget::StaticClass(), false);
	for (const auto Widget : FoundWidgets)
	{
		auto Inventory = Cast<UInvBaseContainerWidget>(Widget)->GetInventoryFromContainerSlot();
		if (!Inventory) return;
		
		Inventory->SetUISettings(UISettings);
		Inventory->InitializeInventory();

		if (Widget->GetName() == UISettings.MainInvWidgetName)
		{
			MainInvWidget = Cast<UInvBaseContainerWidget>(Widget);
			continue;
		}
		if (Widget->GetName() == UISettings.ContainerInWorldWidgetName)
		{
			ContainerInWorldWidget =Cast<UInvBaseContainerWidget>(Widget);
			continue;
		}
		if (Widget->GetName() == UISettings.ContainerInWorldWidgetName)
		{
			VendorInvWidget =Cast<UInvBaseContainerWidget>(Widget);
		}
	}
}

void UCoreHUDWidget::ToggleInventoryMenu()
{
	auto PlayerController = GetOwningPlayer();
	if (!PlayerController)
		return;
	
	if (!bIsShowingInventoryMenu)
	{
		DisplayInventoryMenu();
		
		const FInputModeGameAndUI InputMode;		
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(true);
		return;
	}
	
	HideInventoryMenu();
	const FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(false);
}

void UCoreHUDWidget::DisplayInventoryMenu()
{
	if (!MainInvWidget)
		return;

	MainInvWidget->SetVisibility(ESlateVisibility::Visible);
	bIsShowingInventoryMenu = true;
}

void UCoreHUDWidget::HideInventoryMenu()
{
	if (!MainInvWidget)
		return;

	MainInvWidget->SetVisibility(ESlateVisibility::Collapsed);
	bIsShowingInventoryMenu = false;
}

bool UCoreHUDWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
	if (!InOperation) return false;

	auto DragOp = Cast<UItemDragDropOperation>(InOperation);
	if (DragOp->Payload && DragOp->Payload->GetClass()->ImplementsInterface(UUDraggableWidgetInterface::StaticClass()))
	{
		return IUDraggableWidgetInterface::Execute_OnDropped(DragOp->Payload, InGeometry, DragOp->DragOffset);
	}

	if (DragOp->ItemMoveData.SourceInventory->GetIsUseReferences())
	{
		DragOp->ItemMoveData.SourceInventory->HandleRemoveItemFromContainer(DragOp->ItemMoveData.SourceItem);
		return true;
	}
	
	DragOp->ItemMoveData.SourceInventory->GetItemCollection()->RemoveItemFromAllContainers(DragOp->ItemMoveData.SourceItem);
	auto Item = DragOp->ItemMoveData.SourceItem->DuplicateItem();

	if (auto Pawn = GetOwningPlayerPawn())
	{
		auto Interaction = Pawn->FindComponentByClass<UInteractionComponent>();
		if (!Interaction) return true;

		Interaction->DropItem(Item);
	}
	
	return true;
	
	//return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
