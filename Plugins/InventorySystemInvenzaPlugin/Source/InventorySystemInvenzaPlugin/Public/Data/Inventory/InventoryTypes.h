//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/ItemDataStructures.h"
#include "InventoryTypes.generated.h"


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
class UItemBase;
class UUInventoryBaseWidget;
class UItemCollection;

USTRUCT(BlueprintType)
struct FItemPlacementData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 Quantity = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	EItemOrientationType Orientation = EItemOrientationType::Horizontal;
};

UENUM(BlueprintType)
enum class EHighlightState : uint8
{
	Allowed		UMETA(DisplayName = "Allowed"),
	NotAllowed	UMETA(DisplayName = "Not Allowed"),
	Partial		UMETA(DisplayName = "Partial")
};


UENUM(BlueprintType)
enum class EItemAddResult : uint8
{
	IAR_NoItemAdded UMETA(DisplayName = "No item added"),
	IAR_PartialAmountItemAdded UMETA(DisplayName = "Partial amount of item added"),
	IAR_AllItemAdded UMETA(DisplayName = "All of item added"),
	IAR_ItemSwapped UMETA(DisplayName = "Item swapped") 
};

USTRUCT(BlueprintType, meta=(ScriptName="FItemAddResult"))
struct FItemAddResult
{
	GENERATED_BODY()

	FItemAddResult():
	ActualAmountAdded(0),
	bIsUsedReferences(false),
	OperationResult(EItemAddResult::IAR_NoItemAdded),
	ResultMessage(FText::GetEmpty())
	{};

	// Actual amount of item that was added to the inventory
	UPROPERTY(BlueprintReadOnly, Category="Item Add Result")
	int32 ActualAmountAdded;
	UPROPERTY()
	bool bIsUsedReferences;
	UPROPERTY()
	TMap<UInventorySlotData*, FItemPlacementData> AffectedPivotSlots;
	UPROPERTY(BlueprintReadOnly, Category="Item Add Result")
	EItemAddResult OperationResult;

	// Describes the result
	UPROPERTY(BlueprintReadOnly, Category="Item Add Result")
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
		const FText& ErrorText, const TMap<UInventorySlotData*, FItemPlacementData>& InAffectedSlots)
	{
		FItemAddResult  AddedPartialResult;
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
struct FSlotReservationData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInventorySlotData> Slot;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FItemPlacementData ItemPlacementData;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UItemBase> Resource;
};

USTRUCT(BlueprintType)
struct FItemMoveData 
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UItemBase> SourceItem;
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UInventoryBase> SourceInventory;
	UPROPERTY(BlueprintReadWrite)
	FIntPoint SourceItemPivotSlotCoordinate = FIntPoint(-1);
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UInventoryBase> TargetInventory;
	UPROPERTY(BlueprintReadWrite)
	FIntPoint TargetSlotCoordinate = FIntPoint(-1); // For SlotBased
	UPROPERTY(BlueprintReadWrite)
	EItemOrientationType SavedOrientation = EItemOrientationType::Horizontal;
	UPROPERTY(BlueprintReadWrite)
	EItemOrientationType TargetOrientation = EItemOrientationType::Horizontal;

	FItemMoveData (): SourceItem(nullptr),
	                  SourceInventory(nullptr),
	                  TargetInventory(nullptr)
	{
	}

	FItemMoveData (UItemBase* _SourceItem,
	               UInventoryBase* _SourceInventory,
	               FIntPoint _SourceItemPivotSlotCoordinate,
	               UInventoryBase* _TargetInventory,
	               FIntPoint _TargetSlot = FIntPoint(-1))
	{
		SourceItem = _SourceItem;
		SourceInventory = _SourceInventory;
		SourceItemPivotSlotCoordinate = _SourceItemPivotSlotCoordinate;
		TargetInventory = _TargetInventory;
		TargetSlotCoordinate = _TargetSlot;
	}
};

USTRUCT(BlueprintType)
struct FInventorySlotBasedSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
	TSubclassOf<USlotbasedInventorySlot> SlotbasedInventorySlotClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Slots", 
		meta=(ToolTip="If true, item size will be ignored when placing items. Used for slot-based inventory systems"))
	bool bIgnoreItemSize = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	int InitNumberRows = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	int InitNumColumns = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	FMargin SlotSpacing;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	FVector2D InvCellSize = FVector2D(64.0f, 64.0f);
};

USTRUCT(BlueprintType)
struct FInventorySettings
{
	GENERATED_BODY()

	/* Must be uniq */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	FString InventoryID;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	FGameplayTag InventoryTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	EInventoryType InventoryType = EInventoryType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory", meta = (ToolTip = "-1 means infinite capacity"))
	float InventoryMaxWeightCapacity = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory",
		meta=(ToolTip="Maximum number of unique items allowed. -1 means infinite"))
	int32 MaxStackCount = -1;

	// Reference system
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory|Reference", meta=(ToolTip="If true this container acts as a reference source."))
	bool bIsReferenceContainer = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory|Reference", meta=(ToolTip="If true items from this container can be referenced."))
	bool bAllowItemReferencing = false;

	// Usage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Usage")
	bool bAllowItemUsage = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|UI")
	bool bShowItemTooltips = true;

	/*// Restrictions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Restrictions")
	TArray<FName> AllowedItemCategories;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Restrictions")
	TArray<TSoftObjectPtr<UItemBase>> AllowedItems;*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|UI")
	bool bCollectInvDataFromWidget = true;

	// Assets
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UInventoryBase> InventoryClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|UI")
	TSubclassOf<UInventoryContainerWidget> ContainerWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|UI")
	TSubclassOf<UUInventoryBaseWidget> InventoryWidgetClass;

	// Slot-based inventory settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|UI")
	FInventorySlotBasedSettings InventorySlotBasedSettings;

	// List-based inventory settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|UI")
	TSubclassOf<UInventoryListEntry> EntryClass;

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


USTRUCT(BlueprintType)
struct FInventoryStartupData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FInventorySettings Settings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FInitItemsEntry> StartItems;
};

USTRUCT(BlueprintType)
struct FLinkedInventories
{
	GENERATED_BODY()
	
	// ===== CURRENT =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryBase> ExternalInventory = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryBase> VendorInventory = nullptr;

	// ===== PREVIOUS =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryBase> PrevExternalInventory = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
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
		PrevVendorInventory   = VendorInventory;

		ExternalInventory = NewExternal;
		VendorInventory   = NewVendor;
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
struct FInventorySimulationOperation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Simulation")
	FItemMoveData MoveData;
	UPROPERTY(BlueprintReadOnly, Category="Inventory|Simulation")
	FItemAddResult Result;
};

USTRUCT(BlueprintType)
struct FInventorySlotInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FName SlotName = NAME_None;

	UPROPERTY()
	FIntPoint CellPosition;
	
	UPROPERTY()
	TSoftObjectPtr<UInputAction> UseAction;

	UPROPERTY()
	EItemCategory AllowedCategory = EItemCategory::All;

	UPROPERTY()
	FGameplayTag LinkedEquipmentSlot;
};

USTRUCT(Blueprintable)
struct FItemMapping
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	FString InventoryID;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	bool bIsReferenceContainer = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	EInventoryType InventoryType = EInventoryType::None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TArray<TObjectPtr<UInventorySlotData>> OccupiedSlots;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	EItemOrientationType ItemOrientation = EItemOrientationType::Horizontal;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory", NotReplicated)
	TObjectPtr<UInventoryItemWidget> ItemVisualLinked;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<AStorageVisualRepresentation> ItemVisualRepresentation;

	FItemMapping() = default;

	explicit FItemMapping(UInventorySlotData* SlotData): InventoryType()
	{
		OccupiedSlots.Add(SlotData);
	}
	
	bool operator==(const FItemMapping& Other) const
	{
		return InventoryID == Other.InventoryID
			&& bIsReferenceContainer == Other.bIsReferenceContainer
			&& ItemOrientation == Other.ItemOrientation
			&& ItemVisualLinked == Other.ItemVisualLinked
			&& ItemVisualRepresentation == Other.ItemVisualRepresentation
			&& OccupiedSlots == Other.OccupiedSlots;
	}
};

USTRUCT(BlueprintType)
struct FItemMappingArrayWrapper
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TArray<FItemMapping> Mappings;
};
