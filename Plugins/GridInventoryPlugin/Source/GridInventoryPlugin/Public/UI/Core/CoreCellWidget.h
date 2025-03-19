// Fill out your copyright notice in the Description page of Project Settings.

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
class GRIDINVENTORYPLUGIN_API UCoreCellWidget : public UBaseUserWidget
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

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FVector2D StartSlotSize = FVector2d(100.0f);
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual void NativePreConstruct() override;
};
