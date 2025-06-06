//  Nublin Studio 2025 All Rights Reserved.

#include "UI/Core/CoreCellWidget.h"

#include "Engine/Texture2D.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"

UCoreCellWidget::UCoreCellWidget(): DefaultTintColor(), DefaultColorAndOpacity(), DefaultBorderColor()
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

	if (SizeBox)
	{
		SizeBox->SetWidthOverride(DefaultSlotSize.X);
		SizeBox->SetHeightOverride(DefaultSlotSize.Y);
	}
	
	ResetBorderColor();
	
	ApplyDefaultContentImageStyle();
}

void UCoreCellWidget::ResetContentImage()
{
	if (!Content_Image)
	{
		return;
	}

	FSlateBrush EmptyBrush;
	Content_Image->SetBrush(EmptyBrush);
	Content_Image->SetBrushTintColor(DefaultTintColor);
	Content_Image->SetColorAndOpacity(DefaultColorAndOpacity);
}

void UCoreCellWidget::SetContentImage(UTexture2D* NewTexture)
{
	if (!Content_Image)
	{
		return;
	}

	if (!NewTexture)
	{
		ResetContentImage();
		return;
	}

	FSlateBrush NewBrush;
	NewBrush.SetResourceObject(NewTexture);
	NewBrush.ImageSize = DefaultSlotSize;

	Content_Image->SetBrush(NewBrush);
	Content_Image->SetBrushTintColor(DefaultTintColor);
	Content_Image->SetColorAndOpacity(DefaultColorAndOpacity);
}

void UCoreCellWidget::ApplyDefaultContentImageStyle()
{
	if (!Content_Image)
	{
		return;
	}

	if (!DefaultContent_Image)
	{
		ResetContentImage();
		return;
	}

	FSlateBrush Brush;
	Brush.SetResourceObject(DefaultContent_Image);
	Brush.ImageSize = DefaultSlotSize;

	Content_Image->SetBrush(Brush);
	Content_Image->SetBrushTintColor(DefaultTintColor);
	Content_Image->SetColorAndOpacity(DefaultColorAndOpacity);
}
