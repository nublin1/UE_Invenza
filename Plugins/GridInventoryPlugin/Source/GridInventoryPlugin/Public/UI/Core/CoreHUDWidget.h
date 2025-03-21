// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CanvasPanel.h"
#include "Settings/Settings.h"
#include "UI/BaseUserWidget.h"
#include "CoreHUDWidget.generated.h"

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

	//Inventory
	UFUNCTION()
	void ToggleInventoryMenu();
	void DisplayInventoryMenu();
	void HideInventoryMenu();
	
	
	//Getters
	UInteractionWidget* GetInteractionWidget() {return InteractionWidget;}
	UInventorySystemLayout* GetInventorySystemLayout() {return InventorySystemLayout;} 

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCanvasPanel> ContentPanel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UInteractionWidget> InteractionWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UInventorySystemLayout> InventorySystemLayout;

	//
	bool bIsShowingInventoryMenu = false;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
