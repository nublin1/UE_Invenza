// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CanvasPanel.h"
#include "Settings/InvnzaSettings.h"
#include "UI/BaseUserWidget.h"
#include "CoreHUDWidget.generated.h"

class UInvBaseContainerWidget;
class UInputAction;
class UInventorySystemLayout;
class UInteractionWidget;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UCoreHUDWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UCoreHUDWidget();

	UFUNCTION(BlueprintCallable, Category = "HUD|Setup")
	void InitializeWidget();
	
	void ToggleWidget(UBaseUserWidget* Widget);
	void ShowWidget(UBaseUserWidget* Widget);
	void HideWidget(UBaseUserWidget* Widget);
	//Inventory
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ToggleInventoryLayout();
	// Equipment
	UFUNCTION(BlueprintCallable,Category = "HUD")
	void ToggleEquipmentLayout();

	//
	UFUNCTION()
	void UpdateInputState();
	
	//Getters
	UCanvasPanel* GetContentPanel() {return ContentPanel;}
	UInteractionWidget* GetInteractionWidget() {return InteractionWidget;}
	UInvBaseContainerWidget* GetMainInvWidget() {return MainInvWidget.Get();}
	UInvBaseContainerWidget* GetContainerInWorldWidget() {return ContainerInWorldWidget.Get();}
	UInvBaseContainerWidget* GetVendorInvWidget() {return VendorInvWidget.Get();}
	UInvBaseContainerWidget* GetHotbarInvWidget() {return HotbarInvWidget.Get();}
	UInvBaseContainerWidget* GetEquipmentInvWidget() {return EquipmentInvWidget.Get();}

	//Setters
	void SetUISettings(FUISettings NewUISettings) {UISettings = NewUISettings;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|UI", meta = (BindWidget))
	TObjectPtr<UCanvasPanel> ContentPanel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|UI", meta = (BindWidget))
	TObjectPtr<UInteractionWidget> InteractionWidget;
	
	//
	UPROPERTY()
	TObjectPtr<UInvBaseContainerWidget> MainInvWidget;
	UPROPERTY()
	TObjectPtr<UInvBaseContainerWidget> ContainerInWorldWidget;
	UPROPERTY()
	TObjectPtr<UInvBaseContainerWidget> VendorInvWidget;
	UPROPERTY()
	TObjectPtr<UInvBaseContainerWidget> HotbarInvWidget;
	UPROPERTY()
	TObjectPtr<UInvBaseContainerWidget> EquipmentInvWidget;
	
	//	
	UPROPERTY(BlueprintReadWrite, Category = "HUD|Settings")
	FUISettings UISettings;
	UPROPERTY(VisibleInstanceOnly, Category = "HUD|Settings")
	int32 OpenMenuCount = 0;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	virtual void Hide(UBaseUserWidget* UserWidget);
	
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
