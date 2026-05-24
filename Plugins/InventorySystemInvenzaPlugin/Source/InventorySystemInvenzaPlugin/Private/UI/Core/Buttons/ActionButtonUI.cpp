// Nublin Studio 2026 All Rights Reserved.

#include "UI/Core/Buttons/ActionButtonUI.h"

#include "UI/Core/LabelBaseText.h"
#include "Utility/InputUtility.h"

UActionButtonUI::UActionButtonUI()
{
}

void UActionButtonUI::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UActionButtonUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (ClickAction && KeyText)
	{
		KeyText->UpdateText(UInputUtility::GetKeyForAction(GetWorld(), ClickAction));
	}
}
