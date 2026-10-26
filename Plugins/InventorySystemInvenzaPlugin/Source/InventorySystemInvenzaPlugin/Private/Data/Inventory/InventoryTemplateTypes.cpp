// Nublin Studio 2026 All Rights Reserved.

#include "Data/Inventory/InventoryTemplateTypes.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/ListInventory/InventoryListEntry.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"
#include "UI/Inventory/UInventoryBaseWidget.h"
#include "UI/Core/OperationsPanel/OperationPanelWidget.h"
#include "UI/Inventory/SlotbasedInventorySlot.h"

FInventorySlotBasedSettings FInventorySlotTemplateSettings::ResolveSettings() const
{
	FInventorySlotBasedSettings Result;
	Result.SlotbasedInventorySlotClass = SlotbasedInventorySlotClass.LoadSynchronous();
	Result.bIgnoreItemSize = bIgnoreItemSize;
	Result.InitNumberRows = InitNumberRows;
	Result.InitNumColumns = InitNumColumns;
	Result.SlotSpacing = SlotSpacing;
	Result.InvCellSize = InvCellSize;
	return Result;
}

FInventorySettings FInventoryTemplateSettings::ResolveSettings() const
{
	FInventorySettings Result;
	Result.CustomInventoryID = CustomInventoryID;
	Result.InventoryTag = InventoryTag;
	Result.InventoryType = InventoryType;
	Result.InventoryMaxWeightCapacity = InventoryMaxWeightCapacity;
	Result.MaxStackCount = MaxStackCount;
	Result.bIsAlwaysVisible = bIsAlwaysVisible;
	Result.bShowTotalMoney = bShowTotalMoney;
	Result.bShowTotalWeight = bShowTotalWeight;
	Result.ContainerTitle = ContainerTitle;
	Result.bIsClosable = bIsClosable;
	Result.bShowFilterPanel = bShowFilterPanel;
	Result.bIsReferenceContainer = bIsReferenceContainer;
	Result.bAllowItemReferencing = bAllowItemReferencing;
	Result.bAllowItemUsage = bAllowItemUsage;
	Result.bShowItemTooltips = bShowItemTooltips;
	Result.bCollectInvDataFromWidget = bCollectInvDataFromWidget;
	Result.InventoryClass = InventoryClass.LoadSynchronous();
	Result.ContainerWidgetClass = ContainerWidgetClass.LoadSynchronous();
	Result.InventoryWidgetClass = InventoryWidgetClass.LoadSynchronous();
	Result.OperationPanelWidgetClass = OperationPanelWidgetClass.LoadSynchronous();
	Result.InventorySlotBasedSettings = InventorySlotBasedSettings.ResolveSettings();
	Result.EntryClass = EntryClass.LoadSynchronous();
	return Result;
}
