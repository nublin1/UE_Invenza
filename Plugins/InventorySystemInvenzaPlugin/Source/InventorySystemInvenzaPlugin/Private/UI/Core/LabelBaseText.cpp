// Nublin Studio 2026 All Rights Reserved.

#include "UI/Core/LabelBaseText.h"

#include "Components/TextBlock.h"
#include "UI/UIStructs.h"

ULabelBaseText::ULabelBaseText()
{
}

void ULabelBaseText::NativePreConstruct()
{
	Super::NativePreConstruct();

	SyncTextAndFont();
}

void ULabelBaseText::NativeConstruct()
{
	Super::NativeConstruct();
}

void ULabelBaseText::UpdateText(FText NewText)
{
	Text = NewText;
	SyncTextAndFont();
}

void ULabelBaseText::UpdateFont(FSlateFontInfo NewFontInfo)
{
	FontInfo = NewFontInfo;
	SyncTextAndFont();
}

void ULabelBaseText::UpdateTextAndFont(FLabelStyle LabelStyle)
{
	Text = LabelStyle.Text;
	Color = LabelStyle.Color;
	FontInfo = LabelStyle.FontInfo;

	SyncTextAndFont();
}

void ULabelBaseText::SyncTextAndFont()
{
	if (!MainTextBlock)
		return;
	
	MainTextBlock->SetText(Text);
	MainTextBlock->SetFont(FontInfo);
	MainTextBlock->SetColorAndOpacity(Color);
}

void ULabelBaseText::ClearText()
{
	Text = FText::FromString("");
	SyncTextAndFont();
}