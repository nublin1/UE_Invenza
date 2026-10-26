// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "InvContainerDragDropOperation.generated.h"

class UDragContainerWidget;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvContainerDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UInvContainerDragDropOperation();

public:
	// Properties and variables
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Drag & Drop")
	FVector2D DragOffset;

	// Functions
	virtual void Drop_Implementation(const FPointerEvent& PointerEvent) override;

	virtual void DragCancelled_Implementation(const FPointerEvent& PointerEvent) override;
};
