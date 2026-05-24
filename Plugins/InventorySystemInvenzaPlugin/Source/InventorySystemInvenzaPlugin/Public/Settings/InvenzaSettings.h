//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "InputCoreTypes.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectPtr.h"
#include "UI/Drag/DragContainerWidget.h"
#include "InvenzaSettings.generated.h"

class UCraftMenuChoose;
class UCraftDashboard;
class USlotbasedInventorySlot;
class UModalTradeWidget;
class UItemTooltipWidget;
class UInputMappingContext;
class UInputAction;
class UInventorySlot;
class UHighlightSlotWidget;
class APickup;
class UInventoryItemWidget;
class UInvenzaBaseWidget;

USTRUCT(Blueprintable)
struct FUISettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	FGameplayTag MainInvTag;
	
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Input")
    UInputMappingContext* InventoryMappingContext;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Input")
    UInputAction* ToggleInventoryAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Input")
	UInputAction* ToggleCraftAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Input")
    UInputAction* ToggleEquipmentAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Input")
    UInputAction* IA_Mod_QuickGrab;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Input")
    UInputAction* IA_Mod_GrabAllSame;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Input")
	UInputAction* IA_RotateDraggedItem;
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Input")
    FKey ItemSelectKey;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Input")
    FKey ItemUseKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Highlight")
	FLinearColor AllowedColor = FLinearColor::Green;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Highlight")
	FLinearColor NotAllowedColor = FLinearColor::Red;
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Highlight")
	//FLinearColor PartialColor = FLinearColor::Yellow;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Highlight")
	float HighlightItemOpacity = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Highlight")
	TObjectPtr<UMaterialInterface> HighlightItemMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|SlotBasedInv")
	TObjectPtr<UMaterialInterface> SlotBasedInventoryItemMaterial;

	// Widgets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
    TSubclassOf<UInventoryItemWidget> InventoryItemVisualClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
    TSubclassOf<UInventoryItemWidget> DraggedWidgetClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Widgets")
    TSubclassOf<UHighlightSlotWidget> HighlightSlotWidgetClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
    TSubclassOf<UItemTooltipWidget> ItemTooltipWidgetClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
    TSubclassOf<UModalTradeWidget> ModalTradeWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
	TSubclassOf<UCraftDashboard> CraftDashboardClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
	TSubclassOf<UCraftMenuChoose> CraftMenuChooseClass;
	
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Layout")
    FVector2D DragWidgetSlotSize = FVector2D(0.f);

	FUISettings(): InventoryMappingContext(nullptr),
	               ToggleInventoryAction(nullptr),
	               ToggleEquipmentAction(nullptr),
	               IA_Mod_QuickGrab(nullptr),
	               IA_Mod_GrabAllSame(nullptr), IA_RotateDraggedItem(nullptr),
	               ItemSelectKey(EKeys::LeftMouseButton),
	               ItemUseKey(EKeys::RightMouseButton)
	{
	}
};

