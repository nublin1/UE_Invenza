//  Nublin Studio 2025 All Rights Reserved.

#include "UI/Core/CoreCellWidget.h"

#include "Engine/Texture2D.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "UI/Core/Image/ImageBaseWidget.h"

UCoreCellWidget::UCoreCellWidget() : DefaultTintColor(), DefaultColorAndOpacity(), DefaultBorderColor()
{
}

void UCoreCellWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	ResetBorderColor();

	ApplyDefaultContentImageStyle();

	if (SizeBox)
	{
		float W = SizeBox->GetWidthOverride();
		float H = SizeBox->GetHeightOverride();

		if (SizeBox->IsWidthOverride() && SizeBox->IsHeightOverride())
		{
			CurrentSlotSize = FVector2D(W, H);
		}
	}
}

void UCoreCellWidget::ResetBorderColor()
{
	if (!Left_Border || !Right_Border || !Top_Border || !BottomBorder)
	{
		return;
	}

	Left_Border->SetBrushColor(ResolveThemeColorForProperty(TEXT("DefaultBorderColor"), DefaultBorderColor));
	Right_Border->SetBrushColor(ResolveThemeColorForProperty(TEXT("DefaultBorderColor"), DefaultBorderColor));
	Top_Border->SetBrushColor(ResolveThemeColorForProperty(TEXT("DefaultBorderColor"), DefaultBorderColor));
	BottomBorder->SetBrushColor(ResolveThemeColorForProperty(TEXT("DefaultBorderColor"), DefaultBorderColor));
}

FVector2D UCoreCellWidget::GetCurrentSlotSize() const
{
	return CurrentSlotSize;
}

void UCoreCellWidget::SetSlotSize(const FVector2D& NewSize)
{
	if (!SizeBox)
	{
		return;
	}

	SizeBox->SetWidthOverride(NewSize.X);
	SizeBox->SetHeightOverride(NewSize.Y);

	CurrentSlotSize = NewSize;
}

void UCoreCellWidget::SetContentImage(UTexture2D* NewTexture)
{
	if (!Content_Image || !NewTexture)
	{
		return;
	}

	Content_Image->UpdateImage(NewTexture);
	Content_Image->SetColorAndOpacity(ResolveThemeColorForProperty(TEXT("DefaultColorAndOpacity"), DefaultColorAndOpacity));
}

void UCoreCellWidget::SetImageRotationAngle(const float Angle)
{
	if (!Content_Image)
	{
		return;
	}

	const float NormalizedAngle = FMath::Fmod(Angle, 360.f) / 360.f;
	Content_Image->SetMaterialScalarParam(FName("RotationAngle"), NormalizedAngle);
}

void UCoreCellWidget::ApplyDefaultContentImageStyle()
{
	if (!Content_Image || !DefaultContent_Image)
	{
		return;
	}

	Content_Image->UpdateImage(DefaultContent_Image);
	Content_Image->SetColorAndOpacity(ResolveThemeColorForProperty(TEXT("DefaultColorAndOpacity"), DefaultColorAndOpacity));

	SetImageRotationAngle(0.0f);
}

void UCoreCellWidget::RefreshThemeAppearance()
{
	ResetBorderColor();
}
