//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SlateWrapperTypes.h"
#include "InventoryPanelPresentationTypes.generated.h"

class UCanvasPanel;
class UInventoryContainerWidget;

/** Original HUD placement of a panel temporarily used in another window. */
USTRUCT()
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventoryPanelCanvasPlacement
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UInventoryContainerWidget> Widget;
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> Parent;
	UPROPERTY(Transient)
	FAnchorData Layout;
	bool bAutoSize = false;
	int32 ZOrder = 0;
	ESlateVisibility Visibility = ESlateVisibility::Collapsed;
};

UENUM(BlueprintType)
enum class ECraftMenuState : uint8
{
	Dashboard,
	Choose
};
