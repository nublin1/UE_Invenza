//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Margin.h"
#include "GameplayTagContainer.h"
#include "Data/ItemDataStructures.h"
#include "InventoryTypes.generated.h"

class UOperationPanelWidget;
class USlotbasedInventorySlot;
class UInventoryListEntry;
class UInventoryBase;
class USlotbasedInventory;
class USlotbasedInventoryWidget;
struct FItemPlacementData;
class AStorageVisualRepresentation;
class UInventorySlotData;
enum class EItemOrientationType : uint8;
class UInventoryContainerWidget;
class UInputAction;
class UInventorySlot;
class UItemTooltipWidget;
struct FInventoryCheck;
class UInventoryItemWidget;
class UObject;
class UUInventoryBaseWidget;
class UInventoryCollectionComponent;

UENUM(BlueprintType)
enum class EInventorySortCriteria : uint8
{
	ByName UMETA(DisplayName = "By Name")
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FItemPlacementData
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadWrite)
	int32 Quantity = 0;
	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadWrite)
	EItemOrientationType Orientation = EItemOrientationType::Horizontal;
};

UENUM(BlueprintType)
enum class EHighlightState : uint8
{
	Allowed UMETA(DisplayName = "Allowed"),
	NotAllowed UMETA(DisplayName = "Not Allowed"),
	Partial UMETA(DisplayName = "Partial")
};

UENUM(BlueprintType)
enum class EItemAddResult : uint8
{
	IAR_NoItemAdded UMETA(DisplayName = "No item added"),
	IAR_PartialAmountItemAdded UMETA(DisplayName = "Partial amount of item added"),
	IAR_AllItemAdded UMETA(DisplayName = "All of item added"),
	IAR_ItemSwapped UMETA(DisplayName = "Item swapped")
};

USTRUCT(BlueprintType, meta = (ScriptName = "FItemAddResult"))
struct INVENTORYSYSTEMINVENZAPLUGIN_API FItemAddResult
{
	GENERATED_BODY()

	FItemAddResult()
		: ActualAmountAdded(0), bIsUsedReferences(false), OperationResult(EItemAddResult::IAR_NoItemAdded),
		  ResultMessage(FText::GetEmpty()) {};

	// Actual amount of item that was added to the inventory
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32 ActualAmountAdded;
	UPROPERTY()
	bool bIsUsedReferences;
	UPROPERTY()
	TMap<UInventorySlotData*, FItemPlacementData> AffectedPivotSlots;
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	EItemAddResult OperationResult;

	// Describes the result
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	FText ResultMessage;

	static FItemAddResult AddedNone(const FText& ErrorText)
	{
		FItemAddResult AddedNoneResult;
		AddedNoneResult.ActualAmountAdded = 0;
		AddedNoneResult.OperationResult = EItemAddResult::IAR_NoItemAdded;
		AddedNoneResult.ResultMessage = ErrorText;
		return AddedNoneResult;
	};
	static FItemAddResult AddedPartial(const int32 PartialAmountAdded, const bool bIsUsedReferences,
									   const FText& ErrorText,
									   const TMap<UInventorySlotData*, FItemPlacementData>& InAffectedSlots)
	{
		FItemAddResult AddedPartialResult;
		AddedPartialResult.ActualAmountAdded = PartialAmountAdded;
		AddedPartialResult.bIsUsedReferences = bIsUsedReferences;
		AddedPartialResult.AffectedPivotSlots = InAffectedSlots;
		AddedPartialResult.OperationResult = EItemAddResult::IAR_PartialAmountItemAdded;
		AddedPartialResult.ResultMessage = ErrorText;
		return AddedPartialResult;
	};
	static FItemAddResult AddedAll(const int32 AmountAdded, const bool bIsUsedReferences, const FText& Message,
								   const TMap<UInventorySlotData*, FItemPlacementData>& InAffectedSlots)
	{
		FItemAddResult AddedAllResult;
		AddedAllResult.ActualAmountAdded = AmountAdded;
		AddedAllResult.bIsUsedReferences = bIsUsedReferences;
		AddedAllResult.AffectedPivotSlots = InAffectedSlots;
		AddedAllResult.OperationResult = EItemAddResult::IAR_AllItemAdded;
		AddedAllResult.ResultMessage = Message;
		return AddedAllResult;
	};
	static FItemAddResult Swapped(const int32 AmountAdded, const bool bIsUsedReferences, const FText& Message)
	{
		FItemAddResult AddedAllResult;
		AddedAllResult.ActualAmountAdded = AmountAdded;
		AddedAllResult.bIsUsedReferences = bIsUsedReferences;
		AddedAllResult.OperationResult = EItemAddResult::IAR_ItemSwapped;
		AddedAllResult.ResultMessage = Message;
		return AddedAllResult;
	};
};

UENUM(BlueprintType)
enum class EInventoryType : uint8
{
	None UMETA(DisplayName = "None"),
	MainInventory UMETA(DisplayName = "MainInventory"),
	Inventory UMETA(DisplayName = "Inventory"),
	VendorInventory UMETA(DisplayName = "VendorInventory"),
	LootContainer UMETA(DisplayName = "LootContainer"),
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FSlotReservationData
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInventorySlotData> Slot;
	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadWrite)
	FItemPlacementData ItemPlacementData;
	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UObject> Resource;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FItemDropData
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Inventory", BlueprintReadWrite)
	TObjectPtr<UObject> ItemToDrop;
	UPROPERTY(Category = "Invenza|Inventory", BlueprintReadWrite)
	TObjectPtr<UInventoryBase> SourceInventory;
	UPROPERTY(Category = "Invenza|Inventory", BlueprintReadWrite)
	int32 DropAmount = 0;

	FItemDropData() : ItemToDrop(nullptr), SourceInventory(nullptr), DropAmount(0) {}

	FItemDropData(UObject* InItem, UInventoryBase* InInv, int32 InAmount)
	{
		ItemToDrop = InItem;
		SourceInventory = InInv;
		DropAmount = InAmount;
	}
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FItemMoveData
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Inventory", BlueprintReadWrite)
	TObjectPtr<UObject> SourceItem;
	UPROPERTY(Category = "Invenza|Inventory", BlueprintReadWrite)
	TObjectPtr<UInventoryBase> SourceInventory;
	UPROPERTY(Category = "Invenza|Inventory", BlueprintReadWrite)
	FGuid SourceSlotID;
	UPROPERTY(Category = "Invenza|Inventory", BlueprintReadWrite)
	TObjectPtr<UInventoryBase> TargetInventory;
	UPROPERTY(Category = "Invenza|Inventory", BlueprintReadWrite)
	FGuid TargetSlotID;
	UPROPERTY(Category = "Invenza|Inventory", BlueprintReadWrite)
	EItemOrientationType SavedOrientation = EItemOrientationType::Horizontal;
	UPROPERTY(Category = "Invenza|Inventory", BlueprintReadWrite)
	EItemOrientationType TargetOrientation = EItemOrientationType::Horizontal;

	FItemMoveData() : SourceItem(nullptr), SourceInventory(nullptr), TargetInventory(nullptr)
	{
		TargetSlotID.Invalidate();
	}

	FItemMoveData(UObject* _SourceItem, UInventoryBase* _SourceInventory, FGuid _SourceSlotID,
				  UInventoryBase* _TargetInventory, FGuid _TargetSlotID)
	{
		SourceItem = _SourceItem;
		SourceInventory = _SourceInventory;
		SourceSlotID = _SourceSlotID;
		TargetInventory = _TargetInventory;
		TargetSlotID = _TargetSlotID;
	}
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventorySlotInfo
{
	GENERATED_BODY()

	// A fresh identity is intentional; independently constructed instances cannot compare equal.
	UPROPERTY(meta = (IgnoreForMemberInitializationTest))
	FGuid SlotGuid = FGuid::NewGuid();

	UPROPERTY()
	FName SlotName = NAME_None;

	UPROPERTY()
	FIntPoint CellPosition = FIntPoint::ZeroValue;

	UPROPERTY()
	TSoftObjectPtr<UInputAction> UseAction;

	UPROPERTY()
	FGameplayTag AllowedCategory;

	UPROPERTY()
	FGameplayTag LinkedEquipmentSlot;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FSlotBasedInventoryWidgetInitData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FInventorySlotInfo> SlotLayout;

	UPROPERTY()
	FVector2D InvCellSize = FVector2D::ZeroVector;
	UPROPERTY()
	FMargin SlotSpacing;

	UPROPERTY()
	FIntPoint InventorySize = FIntPoint::ZeroValue;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventorySlotBasedSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
	TSubclassOf<USlotbasedInventorySlot> SlotbasedInventorySlotClass;

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
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventorySettings
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
	TSubclassOf<UInventoryBase> InventoryClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|UI")
	TSubclassOf<UInventoryContainerWidget> ContainerWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|UI")
	TSubclassOf<UUInventoryBaseWidget> InventoryWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|UI")
	TSubclassOf<UOperationPanelWidget> OperationPanelWidgetClass;

	// Slot-based inventory settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|UI")
	FInventorySlotBasedSettings InventorySlotBasedSettings;

	// List-based inventory settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|UI")
	TSubclassOf<UInventoryListEntry> EntryClass;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventoryModifierState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Input")
	bool bIsQuickGrabModifierActive = false;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Input")
	bool bIsGrabAllSameModifierActive = false;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventoryStartupData
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Inventory", EditAnywhere, BlueprintReadWrite)
	FInventorySettings Settings;

	UPROPERTY(Category = "Invenza|Inventory", EditAnywhere, BlueprintReadWrite)
	TArray<FInitItemsEntry> StartItems;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FLinkedInventories
{
	GENERATED_BODY()

	// ===== CURRENT =====
	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryBase> ExternalInventory = nullptr;

	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryBase> VendorInventory = nullptr;

	// ===== PREVIOUS =====
	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryBase> PrevExternalInventory = nullptr;

	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryBase> PrevVendorInventory = nullptr;

	void SetExternal(UInventoryBase* NewInventory)
	{
		PrevExternalInventory = ExternalInventory;
		ExternalInventory = NewInventory;
	}

	void SetVendor(UInventoryBase* NewInventory)
	{
		PrevVendorInventory = VendorInventory;
		VendorInventory = NewInventory;
	}

	void SetBoth(UInventoryBase* NewExternal, UInventoryBase* NewVendor)
	{
		PrevExternalInventory = ExternalInventory;
		PrevVendorInventory = VendorInventory;

		ExternalInventory = NewExternal;
		VendorInventory = NewVendor;
	}
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FLinkedCraftInventories
{
	GENERATED_BODY()

	// ===== CURRENT =====
	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryBase> InputInventory = nullptr;

	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryBase> FuelInventory = nullptr;

	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryBase> OutputInventory = nullptr;

	// ===== PREVIOUS =====
	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryBase> PrevInputInventory = nullptr;

	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryBase> PrevFuelInventory = nullptr;

	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryBase> PrevOutputInventory = nullptr;

	void SetAll(UInventoryBase* NewInput, UInventoryBase* NewFuel, UInventoryBase* NewOutput)
	{
		PrevInputInventory = InputInventory;
		PrevFuelInventory = FuelInventory;
		PrevOutputInventory = OutputInventory;

		InputInventory = NewInput;
		FuelInventory = NewFuel;
		OutputInventory = NewOutput;
	}
};

UENUM(BlueprintType)
enum class EInventoryActionType : uint8
{
	Added,
	Updated,
	Removed
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventorySimulationOperation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Simulation")
	FItemMoveData MoveData;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Simulation")
	FItemAddResult Result;
};

USTRUCT(Blueprintable)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FItemMapping
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	FString InventoryID;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	bool bIsReferenceContainer = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	EInventoryType InventoryType = EInventoryType::None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<TObjectPtr<UInventorySlotData>> OccupiedSlots;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	EItemOrientationType ItemOrientation = EItemOrientationType::Horizontal;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", NotReplicated)
	TObjectPtr<UInventoryItemWidget> ItemVisualLinked;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<AStorageVisualRepresentation> ItemVisualRepresentation;

	FItemMapping() = default;

	explicit FItemMapping(UInventorySlotData* SlotData) : InventoryType() { OccupiedSlots.Add(SlotData); }

	bool operator==(const FItemMapping& Other) const
	{
		return InventoryID == Other.InventoryID && bIsReferenceContainer == Other.bIsReferenceContainer &&
			   ItemOrientation == Other.ItemOrientation && ItemVisualLinked == Other.ItemVisualLinked &&
			   ItemVisualRepresentation == Other.ItemVisualRepresentation && OccupiedSlots == Other.OccupiedSlots;
	}
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FItemMappingArrayWrapper
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FItemMapping> Mappings;
};

UENUM(BlueprintType)
enum class EInventoryContextActionResult : uint8
{
	NotApplicable,
	Success,
	IncompatibleSlot,
	SlotOccupied,
	ItemAlreadyEquipped,
	InvalidSlot,
	EquipmentSlotNotFound
};
