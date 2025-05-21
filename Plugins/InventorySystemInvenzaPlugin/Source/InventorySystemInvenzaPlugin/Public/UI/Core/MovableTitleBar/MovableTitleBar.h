//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "UI/Inrefaces/UDraggableWidgetInterface.h"
#include "MovableTitleBar.generated.h"

class UCoreCellWidget;
class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UMovableTitleBar : public UBaseUserWidget, public IUDraggableWidgetInterface
{
	GENERATED_BODY()
	
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|TitleBar", meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|TitleBar", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Money;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|TitleBar", meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Close;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UMovableTitleBar();
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|TitleBar")
	FText Title;

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Drag")
	bool bAllowDragging = true;

	//
	UPROPERTY()
	UCoreCellWidget* DragContainer_Temp;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	virtual bool OnDropped_Implementation(const FGeometry& DropGeometry, FVector2D DragOffset) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
};
