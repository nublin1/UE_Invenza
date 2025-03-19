// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Core/CoreCellWidget.h"

#include "Components/SizeBox.h"

UCoreCellWidget::UCoreCellWidget()
{
}

void UCoreCellWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	SizeBox->SetWidthOverride(StartSlotSize.X);
	SizeBox->SetHeightOverride(StartSlotSize.Y);
}
