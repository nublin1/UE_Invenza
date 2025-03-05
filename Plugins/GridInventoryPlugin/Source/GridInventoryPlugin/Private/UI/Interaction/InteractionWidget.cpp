// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Interaction/InteractionWidget.h"

#include "Components/TextBlock.h"
#include "Interaction/InteractionData.h"

UInteractionWidget::UInteractionWidget()
{
}

void UInteractionWidget::OnFoundInteractable_Implementation( FInteractionData& NewInteractableData)
{
	UpdateText(NewInteractableData);
	SetVisibility(ESlateVisibility::Visible);
}

void UInteractionWidget::OnLostInteractable_Implementation( FInteractionData& NewInteractableData)
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UInteractionWidget::UpdateText(FInteractionData& NewInteractableData)
{
	NameText->SetText(FText::FromString(NewInteractableData.CurrentInteractable.GetName()));
}
