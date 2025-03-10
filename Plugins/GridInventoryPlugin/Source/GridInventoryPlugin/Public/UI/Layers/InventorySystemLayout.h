// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "InventorySystemLayout.generated.h"

class UBaseInventoryWidget;
class UCanvasPanel;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UInventorySystemLayout : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInventorySystemLayout();

	UBaseInventoryWidget* GetMainInventory() {return MainInventory;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> ContentPanel;

	//
	TArray<TObjectPtr<UBaseInventoryWidget>> InventoryWidgets;
	UPROPERTY()
	TObjectPtr<UBaseInventoryWidget> MainInventory;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	virtual void Init();
};
