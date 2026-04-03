// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/UIStructs.h"
#include "ImageBaseWidget.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UImageBaseWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UImageBaseWidget();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UImage> MainImage;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "UI|Material")
	void SetNewMaterial(UMaterialInterface* NewMaterial);
	UFUNCTION(BlueprintCallable, Category = "UI|Material")
	void SetMaterialScalarParam(FName ParameterName, float Value);
	UFUNCTION(BlueprintCallable, Category = "UI|Material")
	void SetMaterialTextureParam(FName ParameterName, UTexture* Texture);
	UFUNCTION(BlueprintCallable, Category = "UI|Material")
	void SetMaterialVectorParam(FName ParameterName, FLinearColor Value);

	UFUNCTION(BlueprintCallable)
	void SetBaseMaterial(UMaterialInterface* NewMaterial) {BaseMaterial = NewMaterial;}
	UFUNCTION(BlueprintCallable)
	void UpdateImage(UTexture2D* NewTexture);
	UFUNCTION(BlueprintCallable)
	void UpdateBrush(FSlateBrush NewBrush);

	UFUNCTION(BlueprintCallable)
	void SyncImage();

protected:
	// Config
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Material")
	TObjectPtr<UMaterialInterface> BaseMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Material")
	FUIBrushStyle BrushStyle;

	// Data
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void EnsureDynamicMaterial();
};
