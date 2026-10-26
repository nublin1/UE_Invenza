//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Core/Progress/CurrentMaxDisplay.h"
#include "UI/Core/Progress/CraftProgressTypes.h"
#include "UI/UIStructs.h"
#include "CraftProgressDisplay.generated.h"

class UHorizontalBox;
class UProgressBar;
class USizeBox;

/** Reusable Core display for work amounts, a compact bar and/or a percentage. */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UCraftProgressDisplay : public UCurrentMaxDisplay
{
	GENERATED_BODY()

public:
	UCraftProgressDisplay();

protected:
	// Unreal overrides
	virtual void NativePreConstruct() override;

public:
	// Properties and variables
	// Properties
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft Progress")
	FCraftProgressDisplaySettings DisplaySettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft Progress|Appearance")
	FLinearColor FillColor = FLinearColor(0.65f, 0.43f, 0.12f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft Progress|Appearance")
	FLinearColor TrackColor = FLinearColor(0.018f, 0.012f, 0.007f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft Progress|Appearance", meta = (ClampMin = "1"))
	float BarHeight = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft Progress|Appearance")
	FLabelStyle TextStyle;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UHorizontalBox> WorkAmount;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<USizeBox> BarContainer;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> PercentageLabel;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Craft Progress")
	void ConfigureDisplay(const FCraftProgressDisplaySettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Craft Progress")
	void SetProgress(float CompletedWork, float RequiredWork);

	UFUNCTION(BlueprintPure, Category = "Craft Progress")
	float GetNormalizedProgress() const;

protected:
	// Runtime
	// Properties and variables
	// Properties
	float CurrentWork = 0.f;

	float TotalWork = 0.f;

	// Functions
	void RefreshDisplay();
};
