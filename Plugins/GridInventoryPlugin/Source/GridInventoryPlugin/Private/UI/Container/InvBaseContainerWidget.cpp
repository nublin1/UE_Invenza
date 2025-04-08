//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Container/InvBaseContainerWidget.h"

#include "Components/NamedSlot.h"
#include "Components/TextBlock.h"
#include "UI/Core/Weight/InvWeightWidget.h"
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

void UInvBaseContainerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HeaderSlot && HeaderSlot->GetChildrenCount()>0)
	{
		if (auto Widget = Cast<UBaseUserWidget>(HeaderSlot->GetChildAt(0)))
			Widget->SetParentWidget(this);
	}

	if (InvWeight)
	{
		auto Inv = GetInventoryFromContainerSlot();
		if (!Inv)
			return;

		if (Inv->GetWeightCapacity() <0)
			InvWeight->SetVisibility(ESlateVisibility::Collapsed);
		else
		{
			Inv->OnWightUpdatedDelegate.AddDynamic(this, &UInvBaseContainerWidget::UInvBaseContainerWidget::UpdateWeightInfo);
		}
	}
}

void UInvBaseContainerWidget::UpdateWeightInfo(float InventoryTotalWeight, float InventoryWeightCapacity)
{
	FString Text = {" " + FString::SanitizeFloat(InventoryTotalWeight) + "/" + FString::SanitizeFloat(InventoryWeightCapacity)};
	InvWeight->WeightInfo->SetText(FText::FromString(Text));
}
