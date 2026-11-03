// Nublin Studio 2026 All Rights Reserved.

#include "UI/Core/Buttons/UIButton.h"

#include "EnhancedInputComponent.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Utility/InputUtility.h"

UUIButton::UUIButton() : bIsToggleButton(false), bIsToggleOn(false), DefaultButtonBackgroundColor()
{
}

void UUIButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (MainButton && !bCapturedThemeBackground)
	{
		CaptureLocalThemeValues();
		ResolvedNormalBackground = ResolveThemeWidgetColor(MainButton->GetFName(), TEXT("BackgroundColor"), MainButton->GetBackgroundColor());
		bCapturedThemeBackground = true;
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
}

void UUIButton::NativeConstruct()
{
	if (!MainButton)
	{
		MainButton = Cast<UButton>(GetWidgetFromName(TEXT("MainButton")));
	}

	Super::NativeConstruct();

	if (MainButton)
	{
		MainButton->OnPressed.RemoveDynamic(this, &UUIButton::OnMainButtonClicked);
		MainButton->OnPressed.AddDynamic(this, &UUIButton::OnMainButtonClicked);
	}

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
	Super::NativeDestruct();
}

void UUIButton::SetupInput()
{
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(GetOwningPlayer()->InputComponent);
	if (!Input)
	{
		return;
	}

	ClickActionHandle =
		UInputUtility::BindAction(Input, ClickAction, ETriggerEvent::Started, this, &UUIButton::ClickButton);
}

void UUIButton::ClickButton()
{
	MainButton->OnClicked.Broadcast();
}

void UUIButton::SetToggleStatus(const bool bNewStatus)
{
	if (MainButton && !bCapturedThemeBackground)
	{
		CaptureLocalThemeValues();
		ResolvedNormalBackground = ResolveThemeWidgetColor(MainButton->GetFName(), TEXT("BackgroundColor"), MainButton->GetBackgroundColor());
		bCapturedThemeBackground = true;
	}
	bIsToggleOn = bNewStatus;

	OnToggled.Broadcast(bIsToggleOn);

	if (!MainButton)
	{
		return;
	}
	if (bIsToggleOn)
	{
		MainButton->SetBackgroundColor(ResolveThemeColorForProperty(TEXT("ToggleColor"), ToggleColor));
	}
	else
	{
		MainButton->SetBackgroundColor(ResolvedNormalBackground);
	}
}

void UUIButton::UpdateUseAction(UInputAction* NewAction)
{
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(GetOwningPlayer()->InputComponent);
	if (!Input)
	{
		return;
	}

	ClickAction = NewAction;
	ClickActionHandle = UInputUtility::RebindAction(Input, ClickActionHandle, ClickAction, ETriggerEvent::Started, this,
													&UUIButton::ClickButton);
}

void UUIButton::OnMainButtonClicked()
{
	if (bIsToggleButton)
	{
		bIsToggleOn = !bIsToggleOn;
		if (bIsToggleOn)
		{
			MainButton->SetBackgroundColor(ResolveThemeColorForProperty(TEXT("ToggleColor"), ToggleColor));
		}
		else
		{
			MainButton->SetBackgroundColor(ResolvedNormalBackground);
		}
	}

	if (OnButtonClicked.IsBound())
	{
		OnButtonClicked.Broadcast(this);
	}
}

void UUIButton::RefreshThemeAppearance()
{
	if (MainButton)
	{
		CaptureLocalThemeValues();
		ResolvedNormalBackground = ResolveThemeWidgetColor(MainButton->GetFName(), TEXT("BackgroundColor"), MainButton->GetBackgroundColor());
		bCapturedThemeBackground = true;
		const auto Selected = ResolveThemeColorForProperty(TEXT("ToggleColor"), ToggleColor);
		MainButton->SetBackgroundColor(bIsToggleOn ? Selected : ResolvedNormalBackground);
	}
}
