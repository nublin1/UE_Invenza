//  Nublin Studio 2025 All Rights Reserved.

#include "UI/Core/CoreCellWidget.h"

#include "Engine/Texture2D.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "UI/Core/Image/ImageBaseWidget.h"

UCoreCellWidget::UCoreCellWidget(): DefaultTintColor(), DefaultColorAndOpacity(), DefaultBorderColor(),
                                    CurrentSlotSize()
{
}

void UCoreCellWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (SizeBox)
	{
		SizeBox->SetWidthOverride(DefaultSlotSize.X);
		SizeBox->SetHeightOverride(DefaultSlotSize.Y);

		CurrentSlotSize = DefaultSlotSize;
	}
	
	ResetBorderColor();
	
	ApplyDefaultContentImageStyle();
}

void UCoreCellWidget::ResetBorderColor()
{
	if (!Left_Border || !Right_Border || !Top_Border || !BottomBorder) return;
	
	Left_Border->SetBrushColor(DefaultBorderColor);
	Right_Border->SetBrushColor(DefaultBorderColor);
	Top_Border->SetBrushColor(DefaultBorderColor);
	BottomBorder->SetBrushColor(DefaultBorderColor);
}

void UCoreCellWidget::SetContentImage(UTexture2D* NewTexture)
{
	if (!Content_Image || !NewTexture) return;
	
	Content_Image->UpdateImage(NewTexture);
	Content_Image->SetColorAndOpacity(DefaultColorAndOpacity);
}

void UCoreCellWidget::SetImageRotationAngle(float Angle)
{
	if (!Content_Image) return;
	
	const float NormalizedAngle = FMath::Fmod(Angle, 360.f) / 360.f;
	Content_Image->SetMaterialScalarParam(FName("RotationAngle"), NormalizedAngle);
}

void UCoreCellWidget::ApplyDefaultContentImageStyle()
{
	if (!Content_Image || !DefaultContent_Image) return;
	
	Content_Image->UpdateImage(DefaultContent_Image);
	Content_Image->SetColorAndOpacity(DefaultColorAndOpacity);
	
	SetImageRotationAngle(0.0f);
}
