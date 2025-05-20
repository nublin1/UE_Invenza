// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Drag/HighlightSlotWidget.h"

#include "Components/Border.h"
#include "UI/Core/CoreCellWidget.h"
#include "UI/Inventory/InventoryTypes.h"

UHighlightSlotWidget::UHighlightSlotWidget()
{
}

void UHighlightSlotWidget::SetHighlightState(EHighlightState NewState)
{
	if (CurrentState == NewState)
		return;

	CurrentState = NewState;
	
	switch (NewState)
	{
	case EHighlightState::Allowed:
		SetBordersColor(AllowedColor);
		break;
	case EHighlightState::NotAllowed:
		SetBordersColor(NotAllowedColor);
		break;
	case EHighlightState::Partial:
		SetBordersColor(PartialColor);
		break;
	default:
		SetBordersColor(FLinearColor::Transparent);
		break;
	}
}

void UHighlightSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetHighlightState(CurrentState);
}

void UHighlightSlotWidget::SetBordersColor(const FLinearColor& Color)
{
	if (CoreCellWidget->Left_Border)
	{
		CoreCellWidget->Left_Border->SetBrushColor(Color);
	}
	if (CoreCellWidget->Right_Border)
	{
		CoreCellWidget->Right_Border->SetBrushColor(Color);
	}
	if (CoreCellWidget->Top_Border)
	{
		CoreCellWidget->Top_Border->SetBrushColor(Color);
	}
	if (CoreCellWidget->BottomBorder)
	{
		CoreCellWidget->BottomBorder->SetBrushColor(Color);
	}
}
