// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Core/CoreHUDWidget.h"

#include "ActorComponents/ItemCollection.h"
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
	else
	{
		HideInventoryMenu();
		const FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(false);
	}	
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
	if (!InOperation) return false;

	auto DragOp = Cast<UItemDragDropOperation>(InOperation);
	DragOp->ItemMoveData.SourceInventory->GetItemCollection()->RemoveItemFromAllContainers(DragOp->ItemMoveData.SourceItem);
	return true;
	
	//UE_LOG(LogTemp, Log, TEXT("CoreHUDWidget"));
	
	//return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
