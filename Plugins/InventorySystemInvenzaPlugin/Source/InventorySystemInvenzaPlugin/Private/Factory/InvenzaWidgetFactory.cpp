//  Nublin Studio 2026 All Rights Reserved.


#include "Factory/InvenzaWidgetFactory.h"

#include "Components/NamedSlot.h"
#include "UI/Core/OperationsPanel/OperationPanelWidget.h"
#include "UI/Craft/CraftDashboard.h"
#include "UI/Inventory/UInventoryBaseWidget.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"

UInventoryContainerWidget* UInvenzaWidgetFactory::CreateInventoryWidget(APlayerController* OwningPlayer,
                                                                      TSubclassOf<UInventoryContainerWidget> ContainerWidgetClass, TSubclassOf<UUInventoryBaseWidget> InventoryWidgetClass,
                                                                      TSubclassOf<UOperationPanelWidget> OperationPanelClass)
{
	if (!OwningPlayer || !ContainerWidgetClass )
		return nullptr;

	auto InvContWidget = CreateWidget<UInventoryContainerWidget>(OwningPlayer, ContainerWidgetClass);
	if (!InvContWidget)
		return nullptr;

	auto InvWidget = CreateWidget<UUInventoryBaseWidget>(OwningPlayer, InventoryWidgetClass);
	if (!InvWidget)
		return nullptr;

	InvContWidget->ContainerSlot->AddChild(InvWidget);
	
	if (!OperationPanelClass)
		return InvContWidget;

	auto PanelWidget = CreateWidget<UUInventoryBaseWidget>(OwningPlayer, OperationPanelClass);
	if (!PanelWidget)
		return nullptr;

	InvContWidget->OperationsSlot->AddChild(PanelWidget);

	return InvContWidget;
}

UInvenzaBaseWidget* UInvenzaWidgetFactory::CreateInvenzaWidget(APlayerController* OwningPlayer,
	TSubclassOf<UInvenzaBaseWidget> InvenzaBaseWidgetClass)
{
	if (!OwningPlayer || !InvenzaBaseWidgetClass )
		return nullptr;

	auto CraftDashboard = CreateWidget<UInvenzaBaseWidget>(OwningPlayer, InvenzaBaseWidgetClass);
	if (!CraftDashboard)
		return nullptr;

	return CraftDashboard;
}
