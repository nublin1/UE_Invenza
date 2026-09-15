// Nublin Studio 2025 All Rights Reserved.

#include "UI/Interaction/InteractionWidget.h"

#include "Components/ProgressBar.h"
#include "Data/Interactable/InteractableData.h"


UInteractionWidget::UInteractionWidget()
{
}

void UInteractionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (InteractionProgressBar)
	{
		InteractionProgressBar->SetPercent(0);
	}
}

void UInteractionWidget::OnFoundInteractable_Implementation(const TArray<FInteractionDisplayEntry>& Entries)
{
	TArray Rows = { FirstRow, SecondRow, ThirdRow };

	for (int32 i = 0; i < Rows.Num(); ++i)
	{
		if (!Rows[i]) continue;

		if (Entries.IsValidIndex(i))
		{
			Rows[i]->SetRowData(Entries[i].KeyLabel, Entries[i].Data, Entries[i].Data.bHoldToInteract);
			Rows[i]->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Rows[i]->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	InteractionProgressBar->SetVisibility(ESlateVisibility::Visible);
	SetVisibility(ESlateVisibility::Visible);
}

void UInteractionWidget::OnLostInteractable_Implementation(const TArray<FInteractionDisplayEntry>& Entries)
{
	if (FirstRow) FirstRow->SetVisibility(ESlateVisibility::Hidden);
	if (SecondRow) SecondRow->SetVisibility(ESlateVisibility::Hidden);
	if (ThirdRow) ThirdRow->SetVisibility(ESlateVisibility::Hidden);
	
	if (InteractionProgressBar) InteractionProgressBar->SetVisibility(ESlateVisibility::Hidden);
}

void UInteractionWidget::UpdateProgressBar(float Progress)
{
	if (InteractionProgressBar)
	{
		InteractionProgressBar->SetPercent(Progress);
	}
}