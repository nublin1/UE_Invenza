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
	// Unreal overrides
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
							  UDragDropOperation* InOperation) override;

public:
	// Properties and variables
	// Widgets
	UPROPERTY(Category = "Invenza|UI|GameMenu", VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UPanelWidget> PawnInventories;

	UPROPERTY(Category = "Invenza|UI|GameMenu", VisibleAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UWorldDropZoneWidget> WorldDropZone;

	UPROPERTY(Category = "Invenza|UI|GameMenu", VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UDualInventoryWidget> DualInventoryWidget;

	UPROPERTY(Category = "Invenza|UI|GameMenu", VisibleAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> PawnCraftWidgetsPanel;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual UWorldDropZoneWidget* GetWorldDropWidget() override { return WorldDropZone.Get(); }

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual TArray<UUInventoryBaseWidget*> GetAllPawnInventories() const override;

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual TArray<UInventoryContainerWidget*> GetAllPawnInvContainers() const override;

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual UPanelSlot* AddPawnInvContainerWidget(UInventoryContainerWidget* InvContainerWidgetToAdd) const override;

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual void RemovePawnInvContainer(UInventoryContainerWidget* InvContainerToRemove) const override;

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual void OpenDualInventoryView(UInventoryContainerWidget* ExternalContainerWidget,
									   UInventoryContainerWidget* PlayerInventoryToShow) override;

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual void CloseDualInventoryView() override;

	// Forward nested inventories, such as a bag inside a chest, to the floating layer.
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual void OpenNestedInventory(UObject* Key, UInventoryContainerWidget* ContainerWidget,
									 FVector2D ScreenPosition);

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

	virtual bool AttachStorageWindow(UUserWidget* Window) override;

	virtual void BindCraftWidgets() override;

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory UI")
	TMap<EInventoryType, FVector2D> InventoryDefaultPositions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory UI")
	bool bInventoryOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory UI")
	bool bCraftMenuOpen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Craft UI")
	ECraftMenuState CraftMenuState = ECraftMenuState::Dashboard;

	UPROPERTY()
	TObjectPtr<UInventoryContainerWidget> BorrowedPlayerContainer;

	UPROPERTY()
	TObjectPtr<UInventoryContainerWidget> CurrentExternalContainer;

	// Functions
	UFUNCTION()
	void HandleCraftMenuSwap(UUIButton* UIButton);

	UFUNCTION()
	void SetCraftMenuState(ECraftMenuState NewState);

	UFUNCTION()
	void UpdateInputMode();
};
