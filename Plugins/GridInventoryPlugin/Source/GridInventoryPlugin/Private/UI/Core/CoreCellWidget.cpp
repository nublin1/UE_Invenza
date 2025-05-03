// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Core/CoreCellWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"

UCoreCellWidget::UCoreCellWidget(): DefaultBorderColor()
{
}

void UCoreCellWidget::ResetBorderColor()
{
	Left_Border->SetBrushColor(DefaultBorderColor);
	Right_Border->SetBrushColor(DefaultBorderColor);
	Top_Border->SetBrushColor(DefaultBorderColor);
	BottomBorder->SetBrushColor(DefaultBorderColor);
}

void UCoreCellWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	SizeBox->SetWidthOverride(DefaultSlotSize.X);
	SizeBox->SetHeightOverride(DefaultSlotSize.Y);

	DefaultBorderColor = Left_Border->GetBrushColor();
	if (DefaultContent_Image)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(DefaultContent_Image);
		Content_Image->SetBrush(Brush);
	}
}
