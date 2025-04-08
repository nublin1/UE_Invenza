//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "UI/Inrefaces/UDraggableWidgetInterface.h"
#include "MovableTitleBar.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UMovableTitleBar : public UBaseUserWidget, public IUDraggableWidgetInterface
{
	GENERATED_BODY()
	
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleName;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UMovableTitleBar();
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	virtual bool OnDropped_Implementation(const FGeometry& DropGeometry, FVector2D DragOffset) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	
};
