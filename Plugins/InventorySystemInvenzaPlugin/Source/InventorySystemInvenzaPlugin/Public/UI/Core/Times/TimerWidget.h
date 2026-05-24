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

	
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerFinished);
#pragma endregion

public:
	UTimerWidget();
	
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable)
	FOnTimerFinished OnTimerFinished;
	
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UImageBaseWidget> ClockIcon;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> TimeText;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable)
	void SetTime(float InSeconds);

	UFUNCTION(BlueprintCallable)
	void UpdateTime();
	UFUNCTION(BlueprintCallable)
	void UpdateTimeWithDeltaSeconds(float DeltaSeconds);

	UFUNCTION(BlueprintCallable)
	void StartTimer();
	UFUNCTION(BlueprintCallable)
	void ResetTimer();
	UFUNCTION(BlueprintCallable)
	void PauseTimer();
	UFUNCTION(BlueprintCallable)
	void ResumeTimer();
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	float TotalTime = 0.f;
	float RemainingTime = 0.f;

	FTimerHandle TimerHandle;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	void RefreshText() const;
};
