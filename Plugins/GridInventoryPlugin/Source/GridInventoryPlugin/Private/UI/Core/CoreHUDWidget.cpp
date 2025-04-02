//  Nublin Studio 2025 All Rights Reserved.

#include "UI/Core/CoreHUDWidget.h"

#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Items/itemBase.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "UI/Inventory/BaseInventoryWidget.h"
#include "UI/Layers/InventorySystemLayout.h"

UCoreHUDWidget::UCoreHUDWidget()
{
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
	if (!InventorySystemLayout)
		return;

	InventorySystemLayout->SetVisibility(ESlateVisibility::Visible);
	bIsShowingInventoryMenu = true;
}

void UCoreHUDWidget::HideInventoryMenu()
{
	if (!InventorySystemLayout)
		return;

	InventorySystemLayout->SetVisibility(ESlateVisibility::Collapsed);
	bIsShowingInventoryMenu = false;
}

bool UCoreHUDWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
	if (!InOperation) return false;

	auto DragOp = Cast<UItemDragDropOperation>(InOperation);
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
