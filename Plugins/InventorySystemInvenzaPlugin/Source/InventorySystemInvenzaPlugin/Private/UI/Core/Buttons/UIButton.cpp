// Nublin Studio 2025 All Rights Reserved.


#include "UI/Core/Buttons/UIButton.h"

#include "EnhancedInputComponent.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"


UUIButton::UUIButton(): bIsToggleButton(false), bIsToggleOn(false), DefaultButtonBackgroundImage()
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
			Brush.SetResourceObject(Cast<UObject>(DefaultImage.Get()));
			Brush.ImageSize = FVector2D(DefaultSize.X, DefaultSize.Y);
			MainImage->SetBrush(Brush);
		}
	}

	if (MainButton)
		DefaultButtonBackgroundImage = MainButton->GetBackgroundColor();
}

void UUIButton::NativeConstruct()
{
	Super::NativeConstruct();
	
	MainButton->OnPressed.AddDynamic(this, &UUIButton::OnMainButtonClicked);

	if (ClickAction)
	{
		UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(GetOwningPlayer()->InputComponent);
		if (!Input) return;

		Input->BindAction(ClickAction, ETriggerEvent::Started, this, &UUIButton::ClickButton);
	}
}

void UUIButton::ClickButton()
{
	MainButton->OnClicked.Broadcast();
}

void UUIButton::SetToggleStatus(const bool bNewStatus)
{
	bIsToggleOn = bNewStatus;
	if (bIsToggleOn)
		MainButton->SetBackgroundColor(ToggleColor);
	else
		MainButton->SetBackgroundColor(DefaultButtonBackgroundImage);
}

void UUIButton::OnMainButtonClicked()
{
	if (bIsToggleButton)
	{
		bIsToggleOn = !bIsToggleOn;
		if (bIsToggleOn)
			MainButton->SetBackgroundColor(ToggleColor);
		else
			MainButton->SetBackgroundColor(DefaultButtonBackgroundImage);
	}

	if (OnButtonClicked.IsBound())
		OnButtonClicked.Broadcast(this);
}
