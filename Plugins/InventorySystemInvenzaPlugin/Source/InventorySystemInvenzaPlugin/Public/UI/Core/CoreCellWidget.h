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
	virtual void NativePreConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, Category = "Cell UI|Components", meta=(BindWidget))
	TObjectPtr<USizeBox> SizeBox;
	UPROPERTY(VisibleAnywhere, Category = "Cell UI|Components", meta=(BindWidget))
	TObjectPtr<UBorder>	 Left_Border;
	UPROPERTY(VisibleAnywhere, Category = "Cell UI|Components", meta=(BindWidget))
	TObjectPtr<UBorder>  Right_Border;
	UPROPERTY(VisibleAnywhere, Category = "Cell UI|Components", meta=(BindWidget))
	TObjectPtr<UBorder>  Top_Border;
	UPROPERTY(VisibleAnywhere, Category = "Cell UI|Components", meta=(BindWidget))
	TObjectPtr<UBorder>  BottomBorder;
	UPROPERTY(VisibleAnywhere, Category = "Cell UI|Components", meta=(BindWidget))
	TObjectPtr<USizeBox>  Content_ImageSizeBox;
	UPROPERTY(VisibleAnywhere, Category = "Cell UI|Components", meta=(BindWidget))
	TObjectPtr<UImageBaseWidget> Content_Image;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	virtual void ResetBorderColor();

	UFUNCTION()
	FVector2D GetCurrentSlotSize() const {return CurrentSlotSize;}

	UFUNCTION()
	virtual void SetContentImage(UTexture2D* NewTexture);

	UFUNCTION()
	void SetImageRotationAngle(float Angle);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Cell UI|Config")
	FVector2D DefaultSlotSize = FVector2d(64.0f);
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Cell UI|Config")
	TObjectPtr<UTexture2D> DefaultContent_Image;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Cell UI|Config")
	FLinearColor DefaultTintColor;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Cell UI|Config")
	FLinearColor DefaultColorAndOpacity;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Cell UI|Defaults")
	FLinearColor DefaultBorderColor;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Cell UI|Data")
	FVector2D CurrentSlotSize;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION()
	virtual void ApplyDefaultContentImageStyle();
};
