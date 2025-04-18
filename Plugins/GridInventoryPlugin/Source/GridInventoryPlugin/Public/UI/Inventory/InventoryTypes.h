//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SlotbasedInventorySlot.h"
#include "Data/ItemDataStructures.h"
#include "InventoryTypes.generated.h"

struct FInventoryCheck;
class UInventoryItemWidget;
class UItemBase;
class UUInventoryWidgetBase;
class UItemCollection;

USTRUCT(Blueprintable)
struct FItemMapping
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UUInventoryWidgetBase> InventoryWidgetBaseLink;
	UPROPERTY()
	TArray<TObjectPtr<UInventorySlot>> ItemSlots;
	UPROPERTY()
	TObjectPtr<UInventoryItemWidget> ItemVisualLinked;

	FItemMapping() {}
	explicit FItemMapping(UInventorySlot* Slot)
	{
		if (Slot)
		{
			ItemSlots.Add(Slot);
		}
	}
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
	UPROPERTY()
	EOrientationType SavedOrientation;

	FItemMoveData (): SourceItem(nullptr),
					   SourceInventory(nullptr),
					   SourceItemPivotSlot(nullptr), TargetInventory(nullptr),
					   TargetSlot(nullptr),
					   SavedOrientation(EOrientationType::Hotizontal)
	{
	}
};

USTRUCT(BlueprintType)
struct FInventorySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	float InventoryWeightCapacity = -1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(tooltip="If true this container will be used as reference."))
	bool bUseReferences = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(tooltip="Can the items be referenced from this container"))
	bool bCanReferenceItems = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	bool bCanUseItems = true;
};

USTRUCT(BlueprintType)
struct FInventoryData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FInventoryCheck> Checks;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory")
	TObjectPtr<UItemCollection> ItemCollectionLink;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory")
	TArray<UInventorySlot*> InventorySlots;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	float InventoryTotalWeight = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	int32 InventoryTotalMoney = 0;
};

UENUM(BlueprintType)
enum class EHighlightState : uint8
{
	Allowed UMETA(DisplayName = "Allowed"),
	NotAllowed UMETA(DisplayName = "Not Allowed"),
	Partial UMETA(DisplayName = "Partial")
};
