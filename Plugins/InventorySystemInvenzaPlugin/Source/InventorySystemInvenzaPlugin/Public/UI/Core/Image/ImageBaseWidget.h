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
	// Unreal overrides
	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;

public:
	// Properties and variables
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UImage> MainImage;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "UI|Material")
	void SetNewMaterial(UMaterialInterface* NewMaterial);

	UFUNCTION(BlueprintCallable, Category = "UI|Material")
	void SetMaterialScalarParam(FName ParameterName, float Value);

	UFUNCTION(BlueprintCallable, Category = "UI|Material")
	void SetMaterialTextureParam(FName ParameterName, UTexture* Texture);

	UFUNCTION(BlueprintCallable, Category = "UI|Material")
	void SetMaterialVectorParam(FName ParameterName, FLinearColor Value);

	UFUNCTION(Category = "Invenza|UI|Image", BlueprintCallable)
	void SetBaseMaterial(UMaterialInterface* NewMaterial) { BaseMaterial = NewMaterial; }

	UFUNCTION(Category = "Invenza|UI|Image", BlueprintCallable)
	void UpdateImage(UTexture2D* NewTexture);

	UFUNCTION(Category = "Invenza|UI|Image", BlueprintCallable)
	void UpdateBrush(FSlateBrush NewBrush);

	UFUNCTION(Category = "Invenza|UI|Image", BlueprintCallable)
	void SyncImage();

protected:
	// Runtime
	// Properties and variables
	// Config
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Material")
	TObjectPtr<UMaterialInterface> BaseMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Material")
	FUIBrushStyle BrushStyle;

	// Data
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	// Functions
	UFUNCTION()
	void EnsureDynamicMaterial();
};
