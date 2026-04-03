//  Nublin Studio 2026 All Rights Reserved.


#include "Factory/InvenzaWidgetFactory.h"

#include "Components/NamedSlot.h"
#include "UI/Core/OperationsPanel/OperationPanelWidget.h"
#include "UI/Inventory/UInventoryWidgetBase.h"
#include "UI/Inventory/Container/InvBaseContainerWidget.h"

UInvBaseContainerWidget* UInvenzaWidgetFactory::CreateInventoryWidget(APlayerController* OwningPlayer,
                                                                      TSubclassOf<UInvBaseContainerWidget> ContainerWidgetClass, TSubclassOf<UUInventoryWidgetBase> InventoryWidgetClass,
                                                                      TSubclassOf<UOperationPanelWidget> OperationPanelClass)
{
	if (!OwningPlayer || !ContainerWidgetClass )
		return nullptr;

	auto InvContWidget = CreateWidget<UInvBaseContainerWidget>(OwningPlayer, ContainerWidgetClass);
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
