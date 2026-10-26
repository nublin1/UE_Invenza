// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Data/Inventory/InventoryTypes.h"
#include "InventoryTemplateTypes.generated.h"

/** Soft class references keep editor startup independent of inventory Blueprint loading. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventorySlotTemplateSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
	TSoftClassPtr<USlotbasedInventorySlot> SlotbasedInventorySlotClass;

	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "Inventory|Slots",
		meta =
			(ToolTip = "If true, item size will be ignored when placing items. Used for slot-based inventory systems"))
	bool bIgnoreItemSize = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int InitNumberRows = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int InitNumColumns = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FMargin SlotSpacing;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FVector2D InvCellSize = FVector2D(64.0f, 64.0f);

	FInventorySlotBasedSettings ResolveSettings() const;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventoryTemplateSettings
{
	GENERATED_BODY()

	/* Optional custom unique ID for this inventory.
	If left empty, a unique ID will be generated automatically. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FString CustomInventoryID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FGameplayTag InventoryTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	EInventoryType InventoryType = EInventoryType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (ToolTip = "-1 means infinite capacity"))
	float InventoryMaxWeightCapacity = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory",
			  meta = (ToolTip = "Maximum number of unique items allowed. -1 means infinite"))
	int32 MaxStackCount = -1;

	// Container
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	bool bIsAlwaysVisible = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	bool bShowTotalMoney = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	bool bShowTotalWeight = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FText ContainerTitle;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	bool bIsClosable = true;

	// Filters
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	bool bShowFilterPanel = true;

	// Reference system
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Reference",
			  meta = (ToolTip = "If true this container acts as a reference source."))
	bool bIsReferenceContainer = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Reference",
			  meta = (ToolTip = "If true items from this container can be referenced."))
	bool bAllowItemReferencing = false;

	// Usage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Usage")
	bool bAllowItemUsage = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|UI")
	bool bShowItemTooltips = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|UI")
	bool bCollectInvDataFromWidget = true;

	// Assets
	UPROPERTY(Category = "Invenza|Inventory", EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UInventoryBase> InventoryClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|UI")
	TSoftClassPtr<UInventoryContainerWidget> ContainerWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|UI")
	TSoftClassPtr<UUInventoryBaseWidget> InventoryWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|UI")
	TSoftClassPtr<UOperationPanelWidget> OperationPanelWidgetClass;

	// Slot-based inventory settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|UI")
	FInventorySlotTemplateSettings InventorySlotBasedSettings;

	// List-based inventory settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|UI")
	TSoftClassPtr<UInventoryListEntry> EntryClass;

	FInventorySettings ResolveSettings() const;
};
