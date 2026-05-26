//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Inrefaces/UDraggableWidgetInterface.h"
#include "MovableTitleBar.generated.h"

class UUIButton;
class UInvContainerDragDropOperation;
class UDragContainerWidget;
class ULabelBaseText;
class UCoreCellWidget;
class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UMovableTitleBar : public UInvenzaBaseWidget, public IUDraggableWidgetInterface
{
	GENERATED_BODY()
	
public:
	UMovableTitleBar();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

public:	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(BlueprintReadWrite, Category = "TitleBar", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> TitleName;
	UPROPERTY(BlueprintReadWrite, Category = "TitleBar", meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> Button_Close;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TitleBar|Drag")
	void OnDragFinished(bool bSuccess, UDragDropOperation* InOperation);
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TitleBar|Config")
	bool bAllowDragging = true;

	// INTERNAL
	UPROPERTY(Transient)
	TObjectPtr<UDragContainerWidget> DragContainer_Temp = nullptr;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
};
