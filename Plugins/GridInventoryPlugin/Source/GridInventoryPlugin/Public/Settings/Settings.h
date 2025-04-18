//  Nublin Studio 2025 All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Settings.generated.h"

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
	UInputAction* ToggleInventoryAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	UInputAction* IA_Mod_QuickGrab;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	UInputAction* IA_Mod_GrabAllSame ;
	UPROPERTY(EditAnywhere, Category = "Inventory|Input")
	FKey ItemSelectKey = EKeys::LeftMouseButton;
	UPROPERTY(EditAnywhere, Category = "Inventory|Input")
	FKey ItemUseKey = EKeys::RightMouseButton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UInventoryItemWidget> InventoryItemVisualClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UInventoryItemWidget> DraggedWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UHighlightSlotWidget> HighlightSlotWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MainInvWidgetName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ContainerInWorldWidgetName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VendorInvWidgetName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HotbarInvWidgetName = "Hotbar";
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D SlotSize = FVector2D(0.f);
};

USTRUCT(Blueprintable)
struct FRegularSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> PickupClass;
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