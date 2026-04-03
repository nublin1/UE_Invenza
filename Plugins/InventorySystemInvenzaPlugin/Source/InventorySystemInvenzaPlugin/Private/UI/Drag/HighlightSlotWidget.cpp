//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Drag/HighlightSlotWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "UI/Core/CoreCellWidget.h"
#include "Data/Inventory/InventoryTypes.h"
#include "UI/Core/Image/ImageBaseWidget.h"

UHighlightSlotWidget::UHighlightSlotWidget()
{
}

void UHighlightSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//SetHighlightState(CurrentState);
}

void UHighlightSlotWidget::SetHighlightState(EHighlightState NewState)
{
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
	if (!CoreCellWidget || !CoreCellWidget->Content_Image) return;

	if (!NewTexture)
	{
		ClearVisual();
		return;
	}
	
	CoreCellWidget->Content_Image->UpdateImage(NewTexture);
}

void UHighlightSlotWidget::SetHighlightColors(FLinearColor Allowed, FLinearColor NotAllowed)
{
	AllowedColor = Allowed;
	NotAllowedColor = NotAllowed;
	//PartialColor = Partial;
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
