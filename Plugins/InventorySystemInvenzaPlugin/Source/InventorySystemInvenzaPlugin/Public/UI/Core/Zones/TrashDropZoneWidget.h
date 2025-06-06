// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "TrashDropZoneWidget.generated.h"

class UBorder;
class UImage;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UTrashDropZoneWidget : public UBaseUserWidget
{
	GENERATED_BODY()
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UTrashDropZoneWidget();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (BindWidget))
	TObjectPtr<UBorder> TrashBorder;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta=(BindWidgetOptional))
	TObjectPtr<UImage> MainImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor ColorByDefault;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor ColorOnDrag = FLinearColor::Red;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeOnInitialized() override;
	
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                       UDragDropOperation* InOperation);
	void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);
};
