//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Inrefaces/UDraggableWidgetInterface.h"
#include "MovableTitleBar.generated.h"

class UCanvasPanelSlot;
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
	// Unreal overrides
	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
									  UDragDropOperation*& OutOperation) override;

public:
	// Properties and variables
	//Widgets
	UPROPERTY(BlueprintReadWrite, Category = "TitleBar", meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> HeaderCanvasPanel;

	UPROPERTY(BlueprintReadWrite, Category = "TitleBar", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> TitleName;

	UPROPERTY(BlueprintReadWrite, Category = "TitleBar", meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> Button_Close;

	// Functions
	void OnDragFinished_Implementation(bool bSuccess, UDragDropOperation* InOperation);

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TitleBar|Config")
	bool bAllowDragging = true;

	// INTERNAL
	UPROPERTY(Transient)
	TObjectPtr<UDragContainerWidget> DragContainer_Temp = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "TitleBar|Runtime")
	FAnchors SavedAnchors;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "TitleBar|Runtime")
	FVector2D SavedAlignment;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "TitleBar|Runtime")
	bool bAnchorsSaved = false;

	// Functions
	UFUNCTION(Category = "Invenza|UI|MovableTitleBar", BlueprintCallable)
	void ConvertSlotToTopLeftAnchors(UCanvasPanelSlot* CanvasSlot);
};
