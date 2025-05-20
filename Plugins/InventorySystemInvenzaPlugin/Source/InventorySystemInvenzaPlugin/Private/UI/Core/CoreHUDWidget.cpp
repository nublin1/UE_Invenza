//  Nublin Studio 2025 All Rights Reserved.

#include "UI/Core/CoreHUDWidget.h"

#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Items/itemBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "DragDrop/InvContainerDragDropOperation.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Container/InvBaseContainerWidget.h"
#include "UI/Inrefaces/UDraggableWidgetInterface.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"

class UEnhancedInputLocalPlayerSubsystem;

UCoreHUDWidget::UCoreHUDWidget(): UISettings()
{
}

void UCoreHUDWidget::InitializeWidget()
{
	auto PlayerController = GetOwningPlayer();
	auto PlayerCollection = PlayerController->GetPawn()->FindComponentByClass<UItemCollection>();
	
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UInvBaseContainerWidget::StaticClass(), false);
	for (const auto Widget : FoundWidgets)
	{
		auto Inventory = Cast<UInvBaseContainerWidget>(Widget)->GetInventoryFromContainerSlot();
		if (!Inventory) continue;
		
		Inventory->SetUISettings(UISettings);
		Inventory->InitializeInventory();

		UInvBaseContainerWidget* Inv = Cast<UInvBaseContainerWidget>(Widget);
		if (Inv->GetInventoryType() == EInventoryType::MainInventory)
		{
			MainInvWidget = Inv;
			MainInvWidget->GetInventoryFromContainerSlot()->SetParentWidget(Inv);
			MainInvWidget->GetInventoryFromContainerSlot()->SetItemCollection(PlayerCollection);
			MainInvWidget->GetInventoryFromContainerSlot()->InitItemsInItemsCollection();
			MainInvWidget->OnClose.AddDynamic(this, &UCoreHUDWidget::Hide);
			continue;
		}
		if (Inv->GetInventoryType() == EInventoryType::ContainerInventory)
		{
			ContainerInWorldWidget = Inv;
			ContainerInWorldWidget->GetInventoryFromContainerSlot()->SetParentWidget(Inv);
			ContainerInWorldWidget->OnClose.AddDynamic(this, &UCoreHUDWidget::Hide);
			continue;
		}
		if (Inv->GetInventoryType() == EInventoryType::VendorInventory)
		{
			VendorInvWidget = Inv;
			VendorInvWidget->GetInventoryFromContainerSlot()->SetParentWidget(Inv);
			VendorInvWidget = Cast<UInvBaseContainerWidget>(Widget);
			VendorInvWidget->OnClose.AddDynamic(this, &UCoreHUDWidget::Hide);
			continue;
		}
		if (Inv->GetInventoryType() == EInventoryType::Hotbar)
		{
			HotbarInvWidget =Inv;
			HotbarInvWidget->GetInventoryFromContainerSlot()->SetParentWidget(Inv);
			HotbarInvWidget->GetInventoryFromContainerSlot()->SetItemCollection(PlayerCollection);
			continue;
		}
		if (Inv->GetInventoryType() == EInventoryType::EquipmentInventory)
		{
			EquipmentInvWidget = Inv;
			EquipmentInvWidget->GetInventoryFromContainerSlot()->SetParentWidget(Inv);
			EquipmentInvWidget->GetInventoryFromContainerSlot()->SetItemCollection(PlayerCollection);
			EquipmentInvWidget->OnClose.AddDynamic(this, &UCoreHUDWidget::Hide);
			continue;
		}
	}
}

void UCoreHUDWidget::ToggleWidget(UBaseUserWidget* Widget)
{
	if (!Widget)
		return;

	if (Widget->GetVisibility() != ESlateVisibility::Visible)
	{
		ShowWidget(Widget);
	}
	else
	{
		HideWidget(Widget);
	}

	UpdateInputState();
}

void UCoreHUDWidget::ShowWidget(UBaseUserWidget* Widget)
{
	if (!Widget)
		return;

	if (Widget->GetVisibility() != ESlateVisibility::Visible)
	{
		Widget->SetVisibility(ESlateVisibility::Visible);
		++OpenMenuCount;
	}
}

void UCoreHUDWidget::HideWidget(UBaseUserWidget* Widget)
{
	if (!Widget)
		return;

	if (Widget->GetVisibility() == ESlateVisibility::Visible)
	{
		Widget->SetVisibility(ESlateVisibility::Collapsed);
		--OpenMenuCount;
	}
}

void UCoreHUDWidget::ToggleInventoryLayout()
{
	ToggleWidget(MainInvWidget);
}

void UCoreHUDWidget::ToggleEquipmentLayout()
{
	ToggleWidget(EquipmentInvWidget);
}

void UCoreHUDWidget::UpdateInputState()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	if (OpenMenuCount > 0)
	{
		const FInputModeGameAndUI InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
	else
	{
		const FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
}

void UCoreHUDWidget::Hide(UBaseUserWidget* UserWidget)
{
	if (UserWidget == MainInvWidget.Get())
	{
		HideWidget(MainInvWidget.Get());
	}
	else if (UserWidget == EquipmentInvWidget)
	{
		HideWidget(EquipmentInvWidget);
	}
	else if (auto Interaction = GetOwningPlayerPawn()->FindComponentByClass<UInteractionComponent>())
	{
		Interaction->StopInteract();
	}

	UpdateInputState();
}

bool UCoreHUDWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                  UDragDropOperation* InOperation)
{
	const bool bSuperHandled = Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
	if (!InOperation) return bSuperHandled;

	if (auto DragOp = Cast<UItemDragDropOperation>(InOperation))
	{
		if (!DragOp)
			return bSuperHandled;
		
		if (DragOp->Payload && DragOp->Payload->GetClass()->ImplementsInterface(UUDraggableWidgetInterface::StaticClass()))
		{
			return IUDraggableWidgetInterface::Execute_OnDropped(DragOp->Payload, InGeometry, DragOp->DragOffset);
		}

		if (DragOp->ItemMoveData.SourceInventory->GetInventorySettings().bUseReferences)
		{
			DragOp->ItemMoveData.SourceInventory->HandleRemoveItemFromContainer(DragOp->ItemMoveData.SourceItem);
			return true;
		}
	
		DragOp->ItemMoveData.SourceInventory->GetInventoryData().ItemCollectionLink->RemoveItemFromAllContainers(DragOp->ItemMoveData.SourceItem);
		auto Item = DragOp->ItemMoveData.SourceItem->DuplicateItem();
		Item->DropItem(GetWorld());
	
		return true;
	}

	if (auto DragOp = Cast<UInvContainerDragDropOperation>(InOperation))
	{
		if (DragOp->Payload && DragOp->Payload->GetClass()->ImplementsInterface(UUDraggableWidgetInterface::StaticClass()))
		{
			return IUDraggableWidgetInterface::Execute_OnDropped(DragOp->Payload, InGeometry, DragOp->DragOffset);
		}
	}
	return bSuperHandled;
	
}
