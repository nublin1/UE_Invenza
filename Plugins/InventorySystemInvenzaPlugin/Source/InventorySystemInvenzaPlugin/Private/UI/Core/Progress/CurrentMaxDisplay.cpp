// Nublin Studio 2026 All Rights Reserved.



#include "UI/Core/Progress/CurrentMaxDisplay.h"

#include "UI/Core/LabelBaseText.h"

void UCurrentMaxDisplay::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (PrefixText)
	{
		PrefixText->UpdateText(PreTextValue);
	}

	if (SeparatorSymbol)
	{
		SeparatorSymbol->UpdateText(SeparatorSymbolValue);
	}
}
