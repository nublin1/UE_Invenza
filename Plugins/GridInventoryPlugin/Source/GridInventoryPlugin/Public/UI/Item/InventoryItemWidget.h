// Nublin Studio 2025 All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "InventoryItemWidget.generated.h"

class UItemBase;
class UTextBlock;
class UHorizontalBox;
class UCoreCellWidget;
class USizeBox;
class USlotbasedInventorySlot;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UInventoryItemWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInventoryItemWidget();

	UCoreCellWidget* GetCoreCellWidget() const {return CoreCellWidget.Get();}

	UFUNCTION()
	void UpdateVisual(UItemBase* Item);
	UFUNCTION()
	void UpdateVisualSize(FVector2D SlotSize, FIntVector2 ItemSize);
	UFUNCTION()
	void UpdateItemName(FText Name);
	UFUNCTION()
	void UpdateQuantityText(int Quantity);

	UFUNCTION(BlueprintCallable)
	void ChangeBorderColor(FLinearColor NewColor) const;
	UFUNCTION(BlueprintCallable)
	void ChangeOpacity(float NewValue);
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UCoreCellWidget> CoreCellWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<USizeBox> SizeBoxText;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory Slot", meta=(BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HBoxName;
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite, Category="Inventory Slot", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemName;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory Slot", meta=(BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HBoxQuantity;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory Slot", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemQuantity;
		
	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
};
