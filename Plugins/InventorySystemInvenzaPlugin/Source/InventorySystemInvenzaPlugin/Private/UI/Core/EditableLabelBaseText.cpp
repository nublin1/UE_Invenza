// Nublin Studio 2026 All Rights Reserved.

#include "UI/Core/EditableLabelBaseText.h"

#include "Components/EditableTextBox.h"
#include "UI/UIStructs.h"
#include "UI/Theme/InvenzaUIThemeLibrary.h"

UEditableLabelBaseText::UEditableLabelBaseText()
{
}

void UEditableLabelBaseText::NativePreConstruct()
{
	Super::NativePreConstruct();

	SyncTextAndFont();
}

void UEditableLabelBaseText::NativeConstruct()
{
	Super::NativeConstruct();

	if (MainEditableTextBox)
	{
		MainEditableTextBox->OnTextChanged.AddUniqueDynamic(this, &UEditableLabelBaseText::HandleTextChanged);
	}

	SyncTextAndFont();
}

void UEditableLabelBaseText::UpdateText(FText NewText)
{
	Text = NewText;
	if (MainEditableTextBox)
	{
		MainEditableTextBox->SetText(Text);
	}
}

void UEditableLabelBaseText::UpdateHintText(FText NewHintText)
{
	HintText = NewHintText;
	if (MainEditableTextBox)
	{
		MainEditableTextBox->SetHintText(HintText);
	}
}

void UEditableLabelBaseText::UpdateFont(FSlateFontInfo NewFontInfo)
{
	SetThemePropertyOverride(GET_MEMBER_NAME_CHECKED(ThisClass, FontInfo).ToString(), true);
	FontInfo = NewFontInfo;
	RefreshThemeAppearance();
}

void UEditableLabelBaseText::UpdateTextAndFont(FLabelStyle LabelStyle)
{
	SetThemePropertyOverride(GET_MEMBER_NAME_CHECKED(ThisClass, FontInfo).ToString(), true);
	SetThemePropertyOverride(GET_MEMBER_NAME_CHECKED(ThisClass, Color).ToString(), true);
	Text = LabelStyle.Text;
	Color = LabelStyle.Color;
	FontInfo = LabelStyle.FontInfo;

	SyncTextAndFont();
}

void UEditableLabelBaseText::SyncTextAndFont()
{
	if (!MainEditableTextBox)
	{
		return;
	}

	MainEditableTextBox->SetText(Text);
	MainEditableTextBox->SetHintText(HintText);

	FEditableTextBoxStyle& Style = MainEditableTextBox->WidgetStyle;
	Style.TextStyle.SetFont(ResolveThemeFont(TEXT("FontInfo"), FontInfo));
	MainEditableTextBox->SynchronizeProperties();
}

void UEditableLabelBaseText::ClearText()
{
	UpdateText(FText::GetEmpty());
}

FText UEditableLabelBaseText::GetText() const
{
	return Text;
}

void UEditableLabelBaseText::HandleTextChanged(const FText& NewText)
{
	FString WorkingString = NewText.ToString();
	if (bNumericOnly)
	{
		ApplyNumericFilter(WorkingString);

		if (WorkingString != NewText.ToString())
		{
			MainEditableTextBox->SetText(FText::FromString(WorkingString));
			return;
		}
	}

	CachedText = FText::FromString(WorkingString);

	Text = NewText;
	OnEditableTextChanged.Broadcast(NewText);
}

void UEditableLabelBaseText::ApplyNumericFilter(FString& InOutString)
{
	FString Filtered;
	for (TCHAR Char : InOutString)
	{
		if (FChar::IsDigit(Char))
		{
			Filtered.AppendChar(Char);
		}
	}

	InOutString = Filtered;
}

void UEditableLabelBaseText::RefreshThemeAppearance()
{
	if (MainEditableTextBox)
	{
		MainEditableTextBox->WidgetStyle.TextStyle.SetFont(ResolveThemeFont(TEXT("FontInfo"), FontInfo));
		UInvenzaUIThemeLibrary::SynchronizeAppearance(MainEditableTextBox);
	}
}
