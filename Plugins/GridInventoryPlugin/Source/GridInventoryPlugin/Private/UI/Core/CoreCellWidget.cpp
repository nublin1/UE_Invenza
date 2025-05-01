// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Core/CoreCellWidget.h"

#include "Components/Border.h"
#include "Components/SizeBox.h"

UCoreCellWidget::UCoreCellWidget(): InitialBorderColor()
{
}

void UCoreCellWidget::ResetBorderColor()
{
	Left_Border->SetBrushColor(InitialBorderColor);
	Right_Border->SetBrushColor(InitialBorderColor);
	Top_Border->SetBrushColor(InitialBorderColor);
	BottomBorder->SetBrushColor(InitialBorderColor);
}

void UCoreCellWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	SizeBox->SetWidthOverride(StartSlotSize.X);
	SizeBox->SetHeightOverride(StartSlotSize.Y);

	InitialBorderColor = Left_Border->GetBrushColor();
}
