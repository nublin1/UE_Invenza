//  Nublin Studio 2025 All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Settings.generated.h"

class UModalTradeWidget;
class UItemTooltipWidget;
class UInputMappingContext;
class UInputAction;
class UInventorySlot;
class UHighlightSlotWidget;
class APickup;
class UInventoryItemWidget;

USTRUCT(Blueprintable)
struct FUISettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	UInputMappingContext* GameplayMappingContext;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	UInputMappingContext* InventoryMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	UInputAction* ToggleInventoryAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	UInputAction* ToggleEquipmentAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	UInputAction* IA_Mod_QuickGrab;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	UInputAction* IA_Mod_GrabAllSame ;
	UPROPERTY(EditAnywhere, Category = "Inventory|Input")
	FKey ItemSelectKey = EKeys::LeftMouseButton;
	UPROPERTY(EditAnywhere, Category = "Inventory|Input")
	FKey ItemUseKey = EKeys::RightMouseButton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> PickupClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UInventoryItemWidget> InventoryItemVisualClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UInventoryItemWidget> DraggedWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UHighlightSlotWidget> HighlightSlotWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UItemTooltipWidget> ItemTooltipWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UModalTradeWidget> ModalTradeWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D SlotSize = FVector2D(0.f);

	FUISettings(): GameplayMappingContext(nullptr), InventoryMappingContext(nullptr), ToggleInventoryAction(nullptr),
	               ToggleEquipmentAction(nullptr),
	               IA_Mod_QuickGrab(nullptr),
	               IA_Mod_GrabAllSame(nullptr)
	{
	}
};

USTRUCT(BlueprintType)
struct FInventoryModifierState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	bool bIsQuickGrabModifierActive = false;
	UPROPERTY(BlueprintReadWrite)
	bool bIsGrabAllSameModifierActive = false;
};