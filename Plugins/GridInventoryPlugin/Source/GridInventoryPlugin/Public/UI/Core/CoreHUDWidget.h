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
	UFUNCTION()
	void ToggleInventoryMenu();
	void DisplayInventoryMenu();
	void HideInventoryMenu();
	
	//Getters
	UInteractionWidget* GetInteractionWidget() {return InteractionWidget;}
	UInvBaseContainerWidget* GetMainInvWidget() {return MainInvWidget;}

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
	
	//
	bool bIsShowingInventoryMenu = false;
	UPROPERTY(BlueprintReadWrite)
	FUISettings UISettings;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
