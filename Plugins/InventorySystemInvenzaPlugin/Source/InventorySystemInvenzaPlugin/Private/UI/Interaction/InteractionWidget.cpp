// Nublin Studio 2025 All Rights Reserved.

#include "UI/Interaction/InteractionWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Interactable/InteractableData.h"


UInteractionWidget::UInteractionWidget()
{
}

void UInteractionWidget::OnFoundInteractable_Implementation( FInteractableData& NewInteractableData)
{
	UpdateText(NewInteractableData);
	SetVisibility(ESlateVisibility::Visible);
}

void UInteractionWidget::OnLostInteractable_Implementation( FInteractableData& NewInteractableData)
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UInteractionWidget::UpdateProgressBar(float Progress)
{
	if (InteractionProgressBar)
	{
		InteractionProgressBar->SetPercent(Progress);
	}
}

void UInteractionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (InteractionProgressBar)
	{
		InteractionProgressBar->SetPercent(0);
	}
}

void UInteractionWidget::UpdateText(FInteractableData& NewInteractableData)
{
	ActionText->SetText(NewInteractableData.Action);

	if (NewInteractableData.bHoldToInteract)
	{
		KeyPressText->SetText(FText::FromString("Hold"));
	}
	else
	{
		KeyPressText->SetText(FText::FromString("Press"));
	}
	
	if (NewInteractableData.Quantity >= 0)
	{
		QuantityText->SetText(FText::AsNumber(NewInteractableData.Quantity));
		QuantityText->SetVisibility(ESlateVisibility::Visible );
	}
	else
	{
		QuantityText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (!NewInteractableData.Name.IsEmpty())
	{
		NameText->SetText(NewInteractableData.Name);
		NameText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		NameText->SetVisibility(ESlateVisibility::Collapsed);
	}
}
