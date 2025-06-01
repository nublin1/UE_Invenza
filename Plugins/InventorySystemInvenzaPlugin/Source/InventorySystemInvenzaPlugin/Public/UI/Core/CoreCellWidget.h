//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "CoreCellWidget.generated.h"

class UImage;
class USizeBox;
class UTextBlock;
class UBorder;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UCoreCellWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	TObjectPtr<USizeBox> SizeBox;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	TObjectPtr<UBorder>	 Left_Border;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	TObjectPtr<UBorder>  Right_Border;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	TObjectPtr<UBorder>  Top_Border;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	TObjectPtr<UBorder>  BottomBorder;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	TObjectPtr<UImage>	 Content_Image;
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UCoreCellWidget();

	UFUNCTION()
	virtual void ResetBorderColor();
	UFUNCTION()
	virtual void ResetContentImage();

	UFUNCTION()
	virtual void SetContentImage(UTexture2D* NewTexture);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FVector2D DefaultSlotSize = FVector2d(100.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> DefaultContent_Image;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DefaultTintColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DefaultColorAndOpacity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DefaultBorderColor;
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativePreConstruct() override;

	UFUNCTION()
	virtual void ApplyDefaultContentImageStyle();
	
};
