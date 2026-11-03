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
	if (MainTextBlock)
	{
		MainTextBlock->SetText(Text);
	}
}

void ULabelBaseText::UpdateFont(FSlateFontInfo NewFontInfo)
{
	SetThemePropertyOverride(GET_MEMBER_NAME_CHECKED(ThisClass, FontInfo).ToString(), true);
	FontInfo = NewFontInfo;
	RefreshThemeAppearance();
}

void ULabelBaseText::UpdateAppearance(FSlateFontInfo NewFontInfo, FSlateColor NewColor)
{
	SetThemePropertyOverride(GET_MEMBER_NAME_CHECKED(ThisClass, FontInfo).ToString(), true);
	SetThemePropertyOverride(GET_MEMBER_NAME_CHECKED(ThisClass, Color).ToString(), true);
	FontInfo = NewFontInfo;
	Color = NewColor;
	RefreshThemeAppearance();
}

void ULabelBaseText::UpdateTextAndFont(FLabelStyle LabelStyle)
{
	Text = LabelStyle.Text;
	UpdateAppearance(LabelStyle.FontInfo, LabelStyle.Color);
	UpdateText(LabelStyle.Text);
}

void ULabelBaseText::SyncTextAndFont()
{
	if (!MainTextBlock)
	{
		return;
	}

	MainTextBlock->SetText(Text);
	MainTextBlock->SetFont(ResolveThemeFont(TEXT("FontInfo"), FontInfo));
	MainTextBlock->SetColorAndOpacity(ResolveThemeSlateColor(TEXT("Color"), Color));
}

void ULabelBaseText::ClearText()
{
	UpdateText(FText::GetEmpty());
}
void ULabelBaseText::RefreshThemeAppearance()
{
	if (MainTextBlock)
	{
		MainTextBlock->SetFont(ResolveThemeFont(TEXT("FontInfo"), FontInfo));
		MainTextBlock->SetColorAndOpacity(ResolveThemeSlateColor(TEXT("Color"), Color));
	}
}
