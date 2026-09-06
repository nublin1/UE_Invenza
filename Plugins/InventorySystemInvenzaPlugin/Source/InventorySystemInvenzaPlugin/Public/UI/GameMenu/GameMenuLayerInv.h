// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interface/Inventory/InvUIProvider.h"
#include "UI/Core/Zones/WorldDropZoneWidget.h"
#include "UI/Layout/UILayer.h"
#include "GameMenuLayerInv.generated.h"

class UDualInventoryWidget;
class UWorldDropZoneWidget;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UGameMenuLayerInv : public UUILayer, public IInvUIProvider
{
	GENERATED_BODY()

public:
	UGameMenuLayerInv();

protected:
	virtual void NativeOnInitialized() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UPanelWidget> PawnInventories;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UWorldDropZoneWidget> WorldDropZone;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UDualInventoryWidget> DualInventoryWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UPanelWidget> PawnCraftWidgetsPanel;

	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual UWorldDropZoneWidget* GetWorldDropWidget() override {return WorldDropZone.Get();}
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual TArray<UUInventoryBaseWidget*> GetAllPawnInventories() const override;

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual TArray<UInventoryContainerWidget*> GetAllPawnInvContainers() const override;

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual UPanelSlot* AddPawnInvContainerWidget(UInventoryContainerWidget* InvContainerWidgetToAdd) const override;
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual void RemovePawnInvContainer(UInventoryContainerWidget* InvContainerToRemove) const override;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual void OpenDualInventoryView(UInventoryContainerWidget* ExternalContainerWidget, UInventoryContainerWidget* PlayerInventoryToShow) override;
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual void CloseDualInventoryView() override;

	// Для вложенных инвентарей (сумка внутри сундука и т.п.) — прокидываем во floating-слой
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual void OpenNestedInventory(UObject* Key, UInventoryContainerWidget* ContainerWidget, FVector2D ScreenPosition);
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual void CloseNestedInventory(UObject* Key);

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual void ToggleInventoryLayout() override;

	//

	virtual UInvenzaBaseWidget* GetCraftMenuDashboard() override;
	virtual UInvenzaBaseWidget* GetCraftChoose() override;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual UPanelSlot* AddPawnCraftDashboardWidget(UInvenzaBaseWidget* WidgetToAdd) override;
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual UPanelSlot* AddPawnCraftChooseWidget(UInvenzaBaseWidget* WidgetToAdd) override;

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual void ToggleCraftMenuLayout() override;
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual bool IsCraftMenuOpen() const override { return bCraftMenuOpen; }
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual void OpenCraftDashboard() override;
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual void CloseCraftMenu() override;
	
	virtual void BindCraftWidgets() override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory UI")
	TMap<EInventoryType, FVector2D> InventoryDefaultPositions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory UI")
	bool bInventoryOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory UI")
	bool bCraftMenuOpen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Craft UI")
	ECraftMenuState CraftMenuState = ECraftMenuState::Dashboard;
	
	// Runtime	
	UPROPERTY()
	TObjectPtr<UInventoryContainerWidget> BorrowedPlayerContainer;

	UPROPERTY()
	TObjectPtr<UInventoryContainerWidget> CurrentExternalContainer;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION()
	void HandleCraftMenuSwap(UUIButton* UIButton);

	UFUNCTION()
	void SetCraftMenuState(ECraftMenuState NewState);

	UFUNCTION()
	void UpdateInputMode();

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
