//  Nublin Studio 2026 All Rights Reserved.

#include "UI/Controls/ControlsHelpWidget.h"
#include "UI/Controls/ControlsHelpRowWidget.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "UI/Controls/ControlsHelpLibrary.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/TextBlock.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Core/Buttons/UIButton.h"
#include "Subsystems/ModalWindowManager.h"
#include "Subsystems/UIInputModeSubsystem.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Font.h"

UControlsHelpWidget::UControlsHelpWidget()
{
	const auto* Settings = GetDefault<UInvenzaInventoryDeveloperSettings>();
	RowStyle.FontInfo =
		FSlateFontInfo(Settings->DefaultUIFont.LoadSynchronous(), Settings->HelpRowFontSize, TEXT("Regular"));
	RowStyle.Color = FSlateColor(Settings->DefaultTextColor);
	HeadingStyle = RowStyle;
	HeadingStyle.FontInfo.Size = Settings->HelpHeadingFontSize;
	HeadingStyle.FontInfo.TypefaceFontName = TEXT("Bold");
}

void UControlsHelpWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	OKButton->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::HandleOK);
	if (auto* Local = GetOwningLocalPlayer())
	{
		if (auto* Input = Local->GetSubsystem<UUIInputModeSubsystem>())
		{
			Input->RequestUIInput(this);
		}
	}
	RefreshControls();
}

void UControlsHelpWidget::ReleaseInput()
{
	if (auto* Local = GetOwningLocalPlayer())
	{
		if (auto* Input = Local->GetSubsystem<UUIInputModeSubsystem>())
		{
			Input->ReleaseUIInput(this);
		}
	}
}

void UControlsHelpWidget::NativeDestruct()
{
	ReleaseInput();
	if (OKButton)
	{
		OKButton->OnButtonClicked.RemoveDynamic(this, &ThisClass::HandleOK);
	}
	Super::NativeDestruct();
}

void UControlsHelpWidget::RefreshControls()
{
	SetRows(UControlsHelpLibrary::BuildRows(GetOwningPlayer(),
											UInvenzaInventorySettingsSubsystem::GetSettingsStatic(this)));
}

void UControlsHelpWidget::SetRows(const TArray<FControlsHelpRow>& Rows)
{
	if (!ControlsList || !LabelClass)
	{
		return;
	}
	ControlsList->ClearChildren();
	TArray<FString> Groups;
	for (const auto& Row : Rows)
	{
		Groups.AddUnique(Row.Group.ToString());
	}
	auto Label = [&](const FText& Text, bool bHeading)
	{
		auto* Result = CreateWidget<ULabelBaseText>(this, LabelClass);
		FLabelStyle Style = bHeading ? HeadingStyle : RowStyle;
		Style.Text = Text;
		Result->UpdateTextAndFont(Style);
		Result->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (Result->MainTextBlock)
		{
			Result->MainTextBlock->SetAutoWrapText(true);
		}
		return Result;
	};
	for (const FString& Group : Groups)
	{
		if (!Group.IsEmpty())
		{
			Cast<UScrollBoxSlot>(ControlsList->AddChild(Label(FText::FromString(Group), true)))
				->SetPadding(HeadingPadding);
		}
		for (const auto& Row : Rows)
		{
			if (Row.Group.ToString() != Group)
			{
				continue;
			}
			const auto RowClass =
				GetDefault<UInvenzaInventoryDeveloperSettings>()->ControlsHelpRowClass.LoadSynchronous();
			if (!RowClass)
			{
				UE_LOG(LogTemp, Error, TEXT("ControlsHelpRowClass is not configured."));
				continue;
			}
			auto* Line = CreateWidget<UControlsHelpRowWidget>(this, RowClass);
			Line->SetRow(Row.Label, Row.Keys, RowStyle);
			Cast<UScrollBoxSlot>(ControlsList->AddChild(Line))->SetPadding(RowPadding);
		}
	}
	ControlsList->ScrollToStart();
}

void UControlsHelpWidget::HandleOK(UUIButton* Button)
{
	if (auto* Local = GetOwningLocalPlayer())
	{
		Local->GetSubsystem<UModalWindowManager>()->CloseControlsHelp();
	}
}

FReply UControlsHelpWidget::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
	if (Event.GetKey() == EKeys::Escape || Event.GetKey() == EKeys::Enter ||
		Event.GetKey() == EKeys::Gamepad_FaceButton_Right)
	{
		HandleOK(nullptr);
	}
	return FReply::Handled();
}

FReply UControlsHelpWidget::NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event)
{
	// Clicking inside the dialog must not trigger the modal layer's background handler.
	return FReply::Handled();
}
