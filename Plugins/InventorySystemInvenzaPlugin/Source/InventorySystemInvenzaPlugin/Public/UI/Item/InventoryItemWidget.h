// Nublin Studio 2025 All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "InventoryItemWidget.generated.h"

enum class EItemOrientationType : uint8;
class UItemBase;
class UTextBlock;
class UHorizontalBox;
class UCoreCellWidget;
class USizeBox;
class USlotbasedInventorySlot;

/**
 * Inventory Item Widget - Handles visual updates and interactions for inventory items.
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryItemWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory|UI Elements", meta=(BindWidget))
	TObjectPtr<UCoreCellWidget> CoreCellWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory|UI Elements", meta=(BindWidget))
	TObjectPtr<USizeBox> SizeBoxText;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory|UI Elements", meta=(BindWidget))
	TObjectPtr<UHorizontalBox> HBoxName;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory|UI Elements", meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemName;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory|UI Elements", meta=(BindWidget))
	TObjectPtr<UHorizontalBox> HBoxQuantity;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory|UI Elements", meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemQuantity;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInventoryItemWidget();

	UFUNCTION()
	void UpdateItemVisual(UItemBase* Item, EItemOrientationType Orientation,FVector2D TotalSize, FVector2D Position, bool bIgnoreSize);
	UFUNCTION()
	void UpdateVisual(UItemBase* Item, float AngleDegrees = 0.0f);
	UFUNCTION()
	void ClearVisual();
	UFUNCTION()
	void UpdateVisualSize(FVector2D ItemTotalSize) const;
	UFUNCTION()
	void UpdateItemName(FText Name);
	UFUNCTION()
	void UpdateQuantityText(int Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Styling")
	void ChangeBorderColor(FLinearColor NewColor) const;
	UFUNCTION(BlueprintCallable, Category = "Inventory|Styling")
	void ChangeOpacity(float NewValue);
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
	TObjectPtr<UMaterialInterface> IconMaterial;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CachedIconMaterial;
		
	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
};
