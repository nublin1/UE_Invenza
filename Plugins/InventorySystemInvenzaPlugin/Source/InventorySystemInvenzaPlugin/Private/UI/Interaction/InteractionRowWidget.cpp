// Nublin Studio 2026 All Rights Reserved.


#include "UI/Interaction/InteractionRowWidget.h"

void UInteractionRowWidget::SetRowData(const FText& KeyLabel, const FInteractableData& Data, bool bHoldToInteract)
{
	ActionText->SetText(Data.Action);

	const FString Prefix = bHoldToInteract ? TEXT("Hold ") : TEXT("Press ");
	KeyPressText->SetText(FText::FromString(Prefix + KeyLabel.ToString()));

	if (Data.Quantity >= 0)
	{
		QuantityText->SetText(FText::AsNumber(Data.Quantity));
		QuantityText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		QuantityText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (!Data.Name.IsEmpty())
	{
		NameText->SetText(Data.Name);
		NameText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		NameText->SetVisibility(ESlateVisibility::Collapsed);
	}
}
