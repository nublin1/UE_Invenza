//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Container/InvBaseContainerWidget.h"

#include "Components/NamedSlot.h"
#include "UI/Inventory/BaseInventoryWidget.h"

UInvBaseContainerWidget::UInvBaseContainerWidget()
{
}

UBaseInventoryWidget* UInvBaseContainerWidget::GetInventoryFromContainerSlot()
{
	if (!ContainerSlot || ContainerSlot->GetChildrenCount() == 0)
	{
		return nullptr;
	}

	if (UBaseInventoryWidget* BaseInventoryWidget = Cast<UBaseInventoryWidget>(ContainerSlot->GetChildAt(0)))
	{
		return BaseInventoryWidget;
	}

	return nullptr;
}
