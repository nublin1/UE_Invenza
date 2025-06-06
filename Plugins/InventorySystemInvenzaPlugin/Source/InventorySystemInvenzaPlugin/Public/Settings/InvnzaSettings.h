//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "InputCoreTypes.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectPtr.h"
#include "InvnzaSettings.generated.h"

class USlotbasedInventorySlot;
class UModalTradeWidget;
class UItemTooltipWidget;
class UInputMappingContext;
class UInputAction;
class UInventorySlot;
class UHighlightSlotWidget;
class APickup;
class UInventoryItemWidget;
class UBaseUserWidget;

USTRUCT(Blueprintable)
struct FUISettings
{
	GENERATED_BODY()
	
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Input")
    UInputMappingContext* InventoryMappingContext;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Input")
    UInputAction* ToggleInventoryAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Input")
    UInputAction* ToggleEquipmentAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Input")
    UInputAction* IA_Mod_QuickGrab;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Input")
    UInputAction* IA_Mod_GrabAllSame;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Input")
    FKey ItemSelectKey;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Input")
    FKey ItemUseKey;
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|World")
    TSubclassOf<AActor> PickupClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|World")
    TSubclassOf<UBaseUserWidget> DefaultWorldContainerInventoryWidgetClass;
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
    TSubclassOf<USlotbasedInventorySlot> DefaultSlotbasedInventorySlotClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
    TSubclassOf<UInventoryItemWidget> InventoryItemVisualClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
    TSubclassOf<UInventoryItemWidget> DraggedWidgetClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
    TSubclassOf<UHighlightSlotWidget> HighlightSlotWidgetClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
    TSubclassOf<UItemTooltipWidget> ItemTooltipWidgetClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
    TSubclassOf<UModalTradeWidget> ModalTradeWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Widgets")
	TSubclassOf<UBaseUserWidget> DragContainerWidgetClass;
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Layout")
    FVector2D SlotSize = FVector2D(0.f);

	FUISettings(): InventoryMappingContext(nullptr),
	               ToggleInventoryAction(nullptr),
	               ToggleEquipmentAction(nullptr),
	               IA_Mod_QuickGrab(nullptr),
	               IA_Mod_GrabAllSame(nullptr),
					ItemSelectKey(EKeys::LeftMouseButton),
	               ItemUseKey(EKeys::RightMouseButton)
	{
	}
};

USTRUCT(BlueprintType)
struct FInventoryModifierState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Input")
	bool bIsQuickGrabModifierActive = false;
	
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Input")
	bool bIsGrabAllSameModifierActive = false;
};
