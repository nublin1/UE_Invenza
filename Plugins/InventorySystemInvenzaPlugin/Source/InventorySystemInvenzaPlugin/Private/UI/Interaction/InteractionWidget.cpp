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
	OnInteractionDisplayChanged(Entries);

	if (InteractionProgressBar)
	{
		InteractionProgressBar->SetVisibility(ESlateVisibility::Visible);
	}

	SetVisibility(ESlateVisibility::Visible);
}

void UInteractionWidget::OnLostInteractable_Implementation(const TArray<FInteractionDisplayEntry>& Entries)
{
	if (FirstRow) FirstRow->SetVisibility(ESlateVisibility::Hidden);
	if (SecondRow) SecondRow->SetVisibility(ESlateVisibility::Hidden);
	if (ThirdRow) ThirdRow->SetVisibility(ESlateVisibility::Hidden);
	
	if (InteractionProgressBar) InteractionProgressBar->SetVisibility(ESlateVisibility::Hidden);
}

void UInteractionWidget::OnInteractionDisplayChanged_Implementation(const TArray<FInteractionDisplayEntry>& Entries)
{
	UInteractionRowWidget* Rows[] = { FirstRow, SecondRow, ThirdRow };

	for (int32 Index = 0; Index < 3; ++Index)
	{
		UInteractionRowWidget* Row = Rows[Index];
		if (!Row)
		{
			continue;
		}

		if (!Entries.IsValidIndex(Index))
		{
			Row->SetVisibility(ESlateVisibility::Hidden);
			continue;
		}

		const FInteractionDisplayEntry& Entry = Entries[Index];
		Row->SetRowData(Entry.KeyLabel, Entry.Data, Entry.Data.bHoldToInteract);
		Row->SetVisibility(ESlateVisibility::Visible);
	}
}

void UInteractionWidget::UpdateProgressBar(float Progress)
{
	if (InteractionProgressBar)
	{
		InteractionProgressBar->SetPercent(Progress);
	}
}
