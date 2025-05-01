// Nublin Studio 2025 All Rights Reserved.


#include "UI/Core/Buttons/UIButton.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

UUIButton::UUIButton(): bIsToggleButton(false), bIsToggleOn(false)
{
}

void UUIButton::NativePreConstruct()
{
	Super::NativeConstruct();

	if (MainBox)
	{
		MainBox->SetWidthOverride(DefaultSize.X);
		MainBox->SetHeightOverride(DefaultSize.Y);
	}
	
	if (MainLabel)
	{
		MainLabel->SetText(DefaultText);
	}
	
	if (MainImage)
	{
		if (DefaultImage)
		{
			FSlateBrush Brush;
			Brush.SetResourceObject(DefaultImage);
			Brush.ImageSize = FVector2D(DefaultSize.X, DefaultSize.Y);
			MainImage->SetBrush(Brush);
		}
	}
}

void UUIButton::NativeConstruct()
{
	Super::NativeConstruct();
	
	MainButton->OnPressed.AddDynamic(this, &UUIButton::OnMainButtonClicked);
}

void UUIButton::OnMainButtonClicked()
{
	if (bIsToggleButton)
	{
		bIsToggleOn = !bIsToggleOn;
	}

	if (OnButtonClicked.IsBound())
		OnButtonClicked.Broadcast(this);
}
