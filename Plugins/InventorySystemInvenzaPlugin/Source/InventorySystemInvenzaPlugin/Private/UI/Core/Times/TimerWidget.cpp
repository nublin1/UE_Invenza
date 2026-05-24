// Nublin Studio 2026 All Rights Reserved.



#include "UI/Core/Times/TimerWidget.h"

#include "UI/Core/LabelBaseText.h"


UTimerWidget::UTimerWidget()
{
}

void UTimerWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	RefreshText();
}

void UTimerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshText();
}

void UTimerWidget::SetTime(const float InSeconds)
{
	TotalTime = InSeconds;
	RemainingTime = InSeconds;
	RefreshText();
}

void UTimerWidget::UpdateTime()
{
	if (RemainingTime <= 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		return;
	}

	RemainingTime -= 1.f;
	RemainingTime = FMath::Max(0.f, RemainingTime);

	RefreshText();

	if (RemainingTime <= 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		OnTimerFinished.Broadcast();
	}
}

void UTimerWidget::UpdateTimeWithDeltaSeconds(float DeltaSeconds)
{
	if (RemainingTime <= 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		return;
	}

	RemainingTime -= DeltaSeconds;
	RemainingTime = FMath::Max(0.f, RemainingTime);

	RefreshText();

	if (RemainingTime <= 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		OnTimerFinished.Broadcast();
	}
}

void UTimerWidget::StartTimer()
{
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		this,
		&UTimerWidget::UpdateTime,
		1.0f,
		true
	);
}

void UTimerWidget::ResetTimer()
{
	RemainingTime = TotalTime;
	RefreshText();
}

void UTimerWidget::PauseTimer()
{
	GetWorld()->GetTimerManager().PauseTimer(TimerHandle);
}

void UTimerWidget::ResumeTimer()
{
	GetWorld()->GetTimerManager().UnPauseTimer(TimerHandle);
}

void UTimerWidget::RefreshText() const
{
	if (!TimeText)
		return;

	int32 TotalSeconds = FMath::CeilToInt(RemainingTime);

	int32 Minutes = TotalSeconds / 60;
	int32 Seconds = TotalSeconds % 60;

	FString Formatted = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	
	TimeText->UpdateText(FText::FromString(Formatted));

}