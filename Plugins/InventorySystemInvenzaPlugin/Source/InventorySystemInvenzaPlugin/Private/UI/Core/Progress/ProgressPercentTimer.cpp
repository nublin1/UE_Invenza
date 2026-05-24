// Nublin Studio 2026 All Rights Reserved.


#include "UI/Core/Progress/ProgressPercentTimer.h"

#include "Components/ProgressBar.h"
#include "UI/Core/LabelBaseText.h"


UProgressPercentTimer::UProgressPercentTimer()
{
}

void UProgressPercentTimer::SetPercentText(const FString& InText)
{
	if (!InText.IsNumeric())
	{
		return;
	}

	Percent->UpdateText(FText::FromString(InText + TEXT("%")));
}

void UProgressPercentTimer::SetProgressPercent(const FString& InText)
{
	if (!InText.IsNumeric())
	{
		return;
	}

	int32 Value = FCString::Atoi(*InText);

	// 0–100
	Value = FMath::Clamp(Value, 0, 100);

	// 0–1
	float Normalized = Value / 100.f;

	ProgressBar->SetPercent(Normalized);
}
