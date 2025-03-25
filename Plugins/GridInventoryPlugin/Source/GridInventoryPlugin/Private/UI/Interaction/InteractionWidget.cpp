// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Interaction/InteractionWidget.h"

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

void UInteractionWidget::UpdateText(FInteractableData& NewInteractableData)
{
	NameText->SetText(NewInteractableData.Name);
	ActionText->SetText(NewInteractableData.Action);
}
