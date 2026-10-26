//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "CoreCellWidget.generated.h"

class UImageBaseWidget;
class UImage;
class USizeBox;
class UTextBlock;
class UBorder;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UCoreCellWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UCoreCellWidget();

protected:
	// Unreal overrides
	virtual void NativePreConstruct() override;

public:
	// Properties and variables
	// Widgets
	UPROPERTY(VisibleAnywhere, Category = "Cell UI|Components", meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox;

	UPROPERTY(VisibleAnywhere, Category = "Cell UI|Components", meta = (BindWidget))
	TObjectPtr<UBorder> Left_Border;

	UPROPERTY(VisibleAnywhere, Category = "Cell UI|Components", meta = (BindWidget))
	TObjectPtr<UBorder> Right_Border;

	UPROPERTY(VisibleAnywhere, Category = "Cell UI|Components", meta = (BindWidget))
	TObjectPtr<UBorder> Top_Border;

	UPROPERTY(VisibleAnywhere, Category = "Cell UI|Components", meta = (BindWidget))
	TObjectPtr<UBorder> BottomBorder;

	UPROPERTY(VisibleAnywhere, Category = "Cell UI|Components", meta = (BindWidget))
	TObjectPtr<USizeBox> Content_ImageSizeBox;

	UPROPERTY(VisibleAnywhere, Category = "Cell UI|Components", meta = (BindWidget))
	TObjectPtr<UImageBaseWidget> Content_Image;

	// Functions
	UFUNCTION()
	virtual void ResetBorderColor();

	UFUNCTION()
	FVector2D GetCurrentSlotSize() const;

	UFUNCTION()
	virtual void SetSlotSize(const FVector2D& NewSize);

	UFUNCTION()
	virtual void SetContentImage(UTexture2D* NewTexture);

	UFUNCTION()
	void SetImageRotationAngle(float Angle);

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell UI|Config")
	TObjectPtr<UTexture2D> DefaultContent_Image;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell UI|Config")
	FLinearColor DefaultTintColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell UI|Config")
	FLinearColor DefaultColorAndOpacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell UI|Defaults")
	FLinearColor DefaultBorderColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell UI|Data")
	FVector2D CurrentSlotSize;

	// Functions
	UFUNCTION()
	virtual void ApplyDefaultContentImageStyle();
};
