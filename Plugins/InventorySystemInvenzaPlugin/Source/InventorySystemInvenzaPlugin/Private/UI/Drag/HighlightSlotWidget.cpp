//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Drag/HighlightSlotWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "UI/Core/CoreCellWidget.h"
#include "UI/Inventory/InventoryTypes.h"

UHighlightSlotWidget::UHighlightSlotWidget()
{
}

void UHighlightSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetHighlightState(CurrentState);
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

void UHighlightSlotWidget::UpdateVisualWithTexture(UTexture2D* NewTexture)
{
	if (!NewTexture)
	{
		CoreCellWidget->Content_Image->SetBrush(FSlateBrush());
		return;
	}
	
	CoreCellWidget->Content_Image->SetBrushFromTexture(NewTexture);
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
