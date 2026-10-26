// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryContainerWidget.h"
#include "UI/InvenzaBaseWidget.h"
#include "DualInventoryWidget.generated.h"

class UCanvasPanel;
/**
 *
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UDualInventoryWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UDualInventoryWidget() {}

protected:
	// Unreal overrides
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
							  UDragDropOperation* InOperation) override;

public:
	// Properties and variables
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* RootCanvas;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCanvasPanel> FloatingLayer;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Inventory|Dual")
	void SetInventoriesContainers(UInventoryContainerWidget* FirstInventoryContainerWidget,
								  UInventoryContainerWidget* SecondInventoryContainerWidget);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Dual")
	void SetLeftInventoryContainer(UInventoryContainerWidget* InContainer);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Dual")
	void SetRightInventoryContainer(UInventoryContainerWidget* InContainer);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Floating")
	bool AddFloatingInventory(UObject* Key, UInventoryContainerWidget* ExistingWidget, FVector2D ScreenPosition,
							  FVector2D Alignment = FVector2D(0.f, 0.f));

	UFUNCTION(BlueprintCallable, Category = "Inventory|Floating")
	bool RemoveFloatingInventory(UObject* Key);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Dual")
	void ClearAll();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Floating")
	void ClearFloatingInventories();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Alignment")
	void ApplyCornerAlignment();

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Layout")
	float CornerMargin = 16.f;

	bool bLeftPinned = true;

	bool bRightPinned = true;

	UPROPERTY(Category = "Invenza|UI|Inventory", VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInventoryContainerWidget> LeftContainer;

	UPROPERTY(Category = "Invenza|UI|Inventory", VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInventoryContainerWidget> RightContainer;

	UPROPERTY()
	TMap<TWeakObjectPtr<UObject>, TObjectPtr<UInventoryContainerWidget>> FloatingInventories;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Inventory|Floating")
	UInventoryContainerWidget* FindFloatingInventory(UObject* Key) const;
};
