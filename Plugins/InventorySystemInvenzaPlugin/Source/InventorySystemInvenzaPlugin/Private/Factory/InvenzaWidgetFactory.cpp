//  Nublin Studio 2026 All Rights Reserved.


#include "Factory/InvenzaWidgetFactory.h"

#include "Components/NamedSlot.h"
#include "UI/Core/OperationsPanel/OperationPanelWidget.h"
#include "UI/Inventory/UInventoryWidgetBase.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"

UInventoryContainerWidget* UInvenzaWidgetFactory::CreateInventoryWidget(APlayerController* OwningPlayer,
                                                                      TSubclassOf<UInventoryContainerWidget> ContainerWidgetClass, TSubclassOf<UUInventoryWidgetBase> InventoryWidgetClass,
                                                                      TSubclassOf<UOperationPanelWidget> OperationPanelClass)
{
	if (!OwningPlayer || !ContainerWidgetClass )
		return nullptr;

	auto InvContWidget = CreateWidget<UInventoryContainerWidget>(OwningPlayer, ContainerWidgetClass);
	if (!InvContWidget)
		return nullptr;

	auto InvWidget = CreateWidget<UUInventoryWidgetBase>(OwningPlayer, InventoryWidgetClass);
	if (!InvWidget)
		return nullptr;

	InvContWidget->ContainerSlot->AddChild(InvWidget);
	
	if (!OperationPanelClass)
		return InvContWidget;

	auto PanelWidget = CreateWidget<UUInventoryWidgetBase>(OwningPlayer, OperationPanelClass);
	if (!PanelWidget)
		return nullptr;

	InvContWidget->OperationsSlot->AddChild(PanelWidget);

	return InvContWidget;
}
