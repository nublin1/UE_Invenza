// Nublin Studio 2026 All Rights Reserved.


#include "UI/Core/Image/ImageBaseWidget.h"

#include "Components/Image.h"

UImageBaseWidget::UImageBaseWidget()
{
}

void UImageBaseWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (BaseMaterial)
	{
		EnsureDynamicMaterial();
	}
	else
	{
		SyncImage();
	}
}

void UImageBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UImageBaseWidget::SetNewMaterial(UMaterialInterface* NewMaterial)
{
	if (BaseMaterial == NewMaterial) return;

	BaseMaterial = NewMaterial;
	DynamicMaterial = nullptr; 
	EnsureDynamicMaterial();
}

void UImageBaseWidget::SetMaterialScalarParam(FName ParameterName, float Value)
{
	EnsureDynamicMaterial();
	if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue(ParameterName, Value);
	}
}

void UImageBaseWidget::SetMaterialTextureParam(FName ParameterName, UTexture* Texture)
{
	EnsureDynamicMaterial();
	if (DynamicMaterial && Texture)
	{
		DynamicMaterial->SetTextureParameterValue(ParameterName, Texture);
	}
}

void UImageBaseWidget::SetMaterialVectorParam(FName ParameterName, FLinearColor Value)
{
	EnsureDynamicMaterial();
	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(ParameterName, Value);
	}
}

void UImageBaseWidget::UpdateImage(UTexture2D* NewTexture)
{
	if (BaseMaterial)
	{
		SetMaterialTextureParam(FName("IconTexture"), NewTexture);
	}
	else
	{
		BrushStyle.Brush.SetResourceObject(NewTexture);
		SyncImage();
	}
}

void UImageBaseWidget::UpdateBrush(FSlateBrush NewBrush)
{
	BrushStyle.Brush = NewBrush;
	BaseMaterial = nullptr;
	DynamicMaterial = nullptr;
	SyncImage();
}

void UImageBaseWidget::SyncImage()
{
	if (!MainImage) return;
	if (DynamicMaterial)
	{
		MainImage->SetBrushFromMaterial(DynamicMaterial);
	}
	else
	{
		MainImage->SetBrush(BrushStyle.Brush);
	}
    
	MainImage->SetColorAndOpacity(BrushStyle.BrushColor);
}

void UImageBaseWidget::EnsureDynamicMaterial()
{
	if (!MainImage || !BaseMaterial) return;
	
	if (!DynamicMaterial)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		MainImage->SetBrushFromMaterial(DynamicMaterial);
	}
}
