// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SlotbasedInventoryWidget.h"
#include "UI/Inventory/UInventoryBaseWidget.h"
#include "EquipmentInventoryWidget.generated.h"

class UCanvasPanel;
class UUniformGridPanel;
class ASceneCapture2D;
class UImage;
class UTextureRenderTarget2D;
class FEquipmentPortraitScene;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UEquipmentInventoryWidget : public UInventoryContainerWidget
{
	GENERATED_BODY()

public:
	UEquipmentInventoryWidget();

	virtual ~UEquipmentInventoryWidget() override;

protected:
	// Unreal overrides
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// Functions
	/** Uses a private render target; the player's existing portrait remains independent. */
	UFUNCTION(BlueprintCallable, Category = "Equipment|Preview")
	void SetPreviewActor(AActor* Actor);

	UTextureRenderTarget2D* GetPreviewRenderTarget() const { return PreviewRenderTarget; }

	AActor* GetPreviewActor() const { return PreviewActor.Get(); }

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> PreviewActor;

	UPROPERTY(Transient)
	TObjectPtr<ASceneCapture2D> PreviewCapture;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> PreviewRenderTarget;

	UPROPERTY(Transient)
	TObjectPtr<UImage> PreviewImage;

	UPROPERTY(Transient)
	FSlateBrush OriginalPreviewBrush;

	bool bHasOriginalPreviewBrush = false;

	TSharedPtr<FEquipmentPortraitScene> PortraitScene;

	// Functions
	void UpdateCharacterPreview();

	void ReleaseCharacterPreview();
};
