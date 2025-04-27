// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CanvasPanel.h"
#include "Settings/Settings.h"
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
class GRIDINVENTORYPLUGIN_API UCoreHUDWidget : public UBaseUserWidget
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

	UFUNCTION(BlueprintCallable)
	void InitializeWidget();

	//Inventory
	UFUNCTION(BlueprintCallable)
	void ToggleInventoryLayout();
	void DisplayInventoryMenu();
	void HideInventoryMenu();
	// Equipment
	UFUNCTION(BlueprintCallable)
	void ToggleEquipmentLayout();
	void DisplayEquipmentMenu();
	void HideEquipmentMenu();

	//
	UFUNCTION()
	void UpdateInputState();
	
	//Getters
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCanvasPanel> ContentPanel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional))
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
	UPROPERTY(BlueprintReadWrite)
	FUISettings UISettings;
	UPROPERTY()
	int32 OpenMenuCount = 0;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	virtual void Hide(UBaseUserWidget* UserWidget);
	
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
