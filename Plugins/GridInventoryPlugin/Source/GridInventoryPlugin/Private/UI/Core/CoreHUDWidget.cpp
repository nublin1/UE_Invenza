//  Nublin Studio 2025 All Rights Reserved.

#include "UI/Core/CoreHUDWidget.h"

#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Items/itemBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
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

		if (Widget->GetName() == UISettings.MainInvWidgetName)
		{
			MainInvWidget = Cast<UInvBaseContainerWidget>(Widget);
			MainInvWidget->GetInventoryFromContainerSlot()->SetItemCollection(PlayerCollection);
			MainInvWidget->OnClose.AddDynamic(this, &UCoreHUDWidget::Hide);
			continue;
		}
		if (Widget->GetName() == UISettings.ContainerInWorldWidgetName)
		{
			ContainerInWorldWidget =Cast<UInvBaseContainerWidget>(Widget);
			continue;
		}
		if (Widget->GetName() == UISettings.VendorInvWidgetName)
		{
			VendorInvWidget =Cast<UInvBaseContainerWidget>(Widget);
			continue;
		}
		if (Widget->GetName() == UISettings.HotbarInvWidgetName)
		{
			HotbarInvWidget =Cast<UInvBaseContainerWidget>(Widget);
			HotbarInvWidget->GetInventoryFromContainerSlot()->SetItemCollection(PlayerCollection);
			continue;
		}
		if (Widget->GetName() == UISettings.EquipmentInvWidgetName)
		{
			EquipmentInvWidget = Cast<UInvBaseContainerWidget>(Widget);
			EquipmentInvWidget->GetInventoryFromContainerSlot()->SetItemCollection(PlayerCollection);
			EquipmentInvWidget->OnClose.AddDynamic(this, &UCoreHUDWidget::Hide);
			continue;
		}
	}
}

void UCoreHUDWidget::ToggleInventoryLayout()
{
	if (MainInvWidget->GetVisibility() != ESlateVisibility::Visible)
	{
		DisplayInventoryMenu();
		++OpenMenuCount;
	}
	else
	{
		HideInventoryMenu();
	}

	UpdateInputState();
}

void UCoreHUDWidget::DisplayInventoryMenu()
{
	if (!MainInvWidget)
		return;

	MainInvWidget->SetVisibility(ESlateVisibility::Visible);
}

void UCoreHUDWidget::HideInventoryMenu()
{
	if (!MainInvWidget)
		return;

	MainInvWidget->SetVisibility(ESlateVisibility::Collapsed);
	--OpenMenuCount;
}

void UCoreHUDWidget::ToggleEquipmentLayout()
{
	if (EquipmentInvWidget->GetVisibility() != ESlateVisibility::Visible)
	{
		DisplayEquipmentMenu();
		++OpenMenuCount;
	}
	else
	{
		HideEquipmentMenu();
		
	}

	UpdateInputState();
}

void UCoreHUDWidget::DisplayEquipmentMenu()
{
	if (!EquipmentInvWidget)
		return;

	EquipmentInvWidget->SetVisibility(ESlateVisibility::Visible);
}

void UCoreHUDWidget::HideEquipmentMenu()
{
	if (!EquipmentInvWidget)
		return;

	EquipmentInvWidget->SetVisibility(ESlateVisibility::Collapsed);
	--OpenMenuCount;
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
	if (UserWidget == MainInvWidget)
	{
		HideInventoryMenu();
	}
	else if (UserWidget == EquipmentInvWidget)
	{
		HideEquipmentMenu();
	}

	UpdateInputState();
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
