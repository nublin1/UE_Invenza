//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.generated.h"


class UInvBaseContainerWidget;
struct FInventorySlotData;
class UInputAction;
class UInventorySlot;
class UItemTooltipWidget;
struct FInventoryCheck;
class UInventoryItemWidget;
class UItemBase;
class UUInventoryWidgetBase;
class UItemCollection;


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
	static FItemAddResult AddedPartial(const int32 PartialAmountAdded, const bool bIsUsedReferences, const FText& ErrorText)
	{
		FItemAddResult  AddedPartialResult;
		AddedPartialResult.ActualAmountAdded = PartialAmountAdded;
		AddedPartialResult.bIsUsedReferences = bIsUsedReferences;
		AddedPartialResult.OperationResult = EItemAddResult::IAR_PartialAmountItemAdded;
		AddedPartialResult.ResultMessage = ErrorText;
		return AddedPartialResult;
	};
	static FItemAddResult AddedAll(const int32 AmountAdded, const bool bIsUsedReferences, const FText& Message)
	{
		FItemAddResult AddedAllResult;
		AddedAllResult.ActualAmountAdded = AmountAdded;
		AddedAllResult.bIsUsedReferences = bIsUsedReferences;
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
	Hotbar UMETA(DisplayName = "Hotbar"),
	Inventory UMETA(DisplayName = "Inventory"),
	MainInventory UMETA(DisplayName = "MainInventory"),
	VendorInventory UMETA(DisplayName = "VendorInventory"),
	ContainerInventory UMETA(DisplayName = "ContainerInventory"),
	EquipmentInventory UMETA(DisplayName = "EquipmentInventory"),
};

USTRUCT(BlueprintType)
struct FItemMoveData 
{
	GENERATED_BODY()

	UPROPERTY()
	UItemBase* SourceItem;
	UPROPERTY()
	TObjectPtr<UUInventoryWidgetBase> SourceInventory;
	UPROPERTY()
	TObjectPtr<UInventorySlot> SourceItemPivotSlot;
	UPROPERTY()
	TObjectPtr<UUInventoryWidgetBase> TargetInventory;
	UPROPERTY()
	TObjectPtr<UInventorySlot> TargetSlot;
	//UPROPERTY()
	//EOrientationType SavedOrientation; Future

	FItemMoveData (): SourceItem(nullptr),
					   SourceInventory(nullptr),
					   SourceItemPivotSlot(nullptr), TargetInventory(nullptr),
					   TargetSlot(nullptr)
	{
	}

	FItemMoveData (UItemBase* _SourceItem, UUInventoryWidgetBase* _SourceInventory, UUInventoryWidgetBase* _TargetInventory)
	{
		SourceItem = _SourceItem;
		SourceInventory = _SourceInventory;
		TargetInventory = _TargetInventory;
	}
};

USTRUCT(BlueprintType)
struct FInventorySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "-1 means infinite capacity"))
	float InventoryWeightCapacity = -1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(tooltip="If true this container will be used as reference."))
	bool bUseReferences = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(tooltip="Can the items be referenced from this container"))
	bool bCanReferenceItems = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	bool bCanUseItems = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	bool bShowTooltips = true;
};

USTRUCT(BlueprintType)
struct FInventoryData
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory")
	TObjectPtr<UItemCollection> ItemCollectionLink;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory")
	TObjectPtr<UItemTooltipWidget> ItemTooltipWidget = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory")
	TArray<UInventorySlot*> InventorySlots;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	float InventoryTotalWeight = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	int32 InventoryTotalMoney = 0;
};

USTRUCT(BlueprintType)
struct FInventorySlotData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotName = " ";

	UPROPERTY(VisibleAnywhere)
	FIntVector2 SlotPosition{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> UseAction;

	FInventorySlotData() {}

	bool operator==(const FInventorySlotData& Other) const
	{
		return SlotName == Other.SlotName
			&& SlotPosition == Other.SlotPosition
			&& UseAction == Other.UseAction;
	}
};

USTRUCT(Blueprintable)
struct FItemMapping
{
	GENERATED_BODY()

	UPROPERTY()
	FName InventoryContainerName;
	UPROPERTY()
	EInventoryType InventoryType;
	UPROPERTY()
	TArray<FInventorySlotData> ItemSlotDatas;
	UPROPERTY()
	TObjectPtr<UInventoryItemWidget> ItemVisualLinked;

	FItemMapping(): InventoryType()
	{
	}

	explicit FItemMapping(FInventorySlotData SlotData): InventoryType()
	{
		ItemSlotDatas.Add(SlotData);
	}
};

UENUM(BlueprintType)
enum class EHighlightState : uint8
{
	Allowed		UMETA(DisplayName = "Allowed"),
	NotAllowed	UMETA(DisplayName = "Not Allowed"),
	Partial		UMETA(DisplayName = "Partial")
};


