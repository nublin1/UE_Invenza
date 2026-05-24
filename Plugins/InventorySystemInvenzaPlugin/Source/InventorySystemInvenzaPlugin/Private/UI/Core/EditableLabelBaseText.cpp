// Nublin Studio 2026 All Rights Reserved.


#include "UI/Core/EditableLabelBaseText.h"

#include "Components/EditableTextBox.h"
#include "UI/UIStructs.h"


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
		MainEditableTextBox->OnTextChanged.AddDynamic(this, &UEditableLabelBaseText::HandleTextChanged);
	}

	SyncTextAndFont();
}

void UEditableLabelBaseText::UpdateText(FText NewText)
{
	Text = NewText;
	SyncTextAndFont();
}

void UEditableLabelBaseText::UpdateHintText(FText NewHintText)
{
}

void UEditableLabelBaseText::UpdateFont(FSlateFontInfo NewFontInfo)
{
	FontInfo = NewFontInfo;
	SyncTextAndFont();
}

void UEditableLabelBaseText::UpdateTextAndFont(FLabelStyle LabelStyle)
{
	Text = LabelStyle.Text;
	Color = LabelStyle.Color;
	FontInfo = LabelStyle.FontInfo;

	SyncTextAndFont();
}

void UEditableLabelBaseText::SyncTextAndFont()
{
	if (!MainEditableTextBox)
		return;

	MainEditableTextBox->SetText(Text);
	MainEditableTextBox->SetHintText(HintText);
	
	FEditableTextBoxStyle& Style = MainEditableTextBox->WidgetStyle;
	Style.TextStyle.SetFont(FontInfo);
	MainEditableTextBox->SynchronizeProperties();
}

void UEditableLabelBaseText::ClearText()
{
	Text = FText::GetEmpty();
	SyncTextAndFont();
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
