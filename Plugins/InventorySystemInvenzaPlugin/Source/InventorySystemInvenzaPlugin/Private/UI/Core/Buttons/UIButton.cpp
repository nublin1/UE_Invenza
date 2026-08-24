// Nublin Studio 2026 All Rights Reserved.


#include "UI/Core/Buttons/UIButton.h"

#include "EnhancedInputComponent.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Utility/InputUtility.h"


UUIButton::UUIButton(): bIsToggleButton(false), bIsToggleOn(false), DefaultButtonBackgroundColor()
{
}

void UUIButton::NativePreConstruct()
{
	Super::NativeConstruct();

	if (MainButton)
		DefaultButtonBackgroundColor = MainButton->GetBackgroundColor();
	
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
}

void UUIButton::NativeConstruct()
{
	if (!MainButton)
	{
		MainButton = Cast<UButton>(GetWidgetFromName(TEXT("MainButton")));
	}
	
	Super::NativeConstruct();

	if (MainButton)
		MainButton->OnPressed.AddDynamic(this, &UUIButton::OnMainButtonClicked);

	if (ClickAction)
	{
		SetupInput();
	}
}

void UUIButton::NativeDestruct()
{
	if (ClickActionHandle != 0)
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PC->InputComponent))
			{
				UInputUtility::RemoveBinding(Input, ClickActionHandle);
			}
		}
	}
}

void UUIButton::SetupInput()
{
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(GetOwningPlayer()->InputComponent);
	if (!Input) return;

	ClickActionHandle = UInputUtility::BindAction(
		Input,
		ClickAction,
		ETriggerEvent::Started,
		this,
		&UUIButton::ClickButton
	);
}

void UUIButton::ClickButton()
{
	MainButton->OnClicked.Broadcast();
}

void UUIButton::SetToggleStatus(const bool bNewStatus)
{
	bIsToggleOn = bNewStatus;

	OnToggled.Broadcast(bIsToggleOn);
	
	if (bIsToggleOn)
		MainButton->SetBackgroundColor(ToggleColor);
	else
		MainButton->SetBackgroundColor(DefaultButtonBackgroundColor);
}

void UUIButton::UpdateUseAction(UInputAction* NewAction)
{
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(GetOwningPlayer()->InputComponent);
	if (!Input) return;
	
	ClickAction = NewAction;
	ClickActionHandle = UInputUtility::RebindAction(
		Input,
		ClickActionHandle,
		ClickAction,
		ETriggerEvent::Started,
		this,
		&UUIButton::ClickButton
	);
}

void UUIButton::OnMainButtonClicked()
{
	if (bIsToggleButton)
	{
		bIsToggleOn = !bIsToggleOn;
		if (bIsToggleOn)
			MainButton->SetBackgroundColor(ToggleColor);
		else
			MainButton->SetBackgroundColor(DefaultButtonBackgroundColor);
	}

	if (OnButtonClicked.IsBound())
		OnButtonClicked.Broadcast(this);
}
