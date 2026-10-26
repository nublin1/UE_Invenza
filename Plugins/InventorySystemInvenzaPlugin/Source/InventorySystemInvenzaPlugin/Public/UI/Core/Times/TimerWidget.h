// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "TimerWidget.generated.h"

class ULabelBaseText;
class UImageBaseWidget;
/**
 *
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UTimerWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UTimerWidget();

protected:
	// Unreal overrides
	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerFinished);
#pragma endregion Delegates

	// Properties and variables
	UPROPERTY(Category = "Invenza|UI|Times", BlueprintAssignable)
	FOnTimerFinished OnTimerFinished;

	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UImageBaseWidget> ClockIcon;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> TimeText;

	// Functions
	UFUNCTION(Category = "Invenza|UI|Times", BlueprintCallable)
	void SetTime(float InSeconds);

	UFUNCTION(Category = "Invenza|UI|Times", BlueprintCallable)
	void UpdateTime();

	UFUNCTION(Category = "Invenza|UI|Times", BlueprintCallable)
	void UpdateTimeWithDeltaSeconds(float DeltaSeconds);

	UFUNCTION(Category = "Invenza|UI|Times", BlueprintCallable)
	void StartTimer();

	UFUNCTION(Category = "Invenza|UI|Times", BlueprintCallable)
	void ResetTimer();

	UFUNCTION(Category = "Invenza|UI|Times", BlueprintCallable)
	void PauseTimer();

	UFUNCTION(Category = "Invenza|UI|Times", BlueprintCallable)
	void ResumeTimer();

protected:
	// Runtime
	// Properties and variables
	float TotalTime = 0.f;

	float RemainingTime = 0.f;

	FTimerHandle TimerHandle;

	// Functions
	void RefreshText() const;
};
