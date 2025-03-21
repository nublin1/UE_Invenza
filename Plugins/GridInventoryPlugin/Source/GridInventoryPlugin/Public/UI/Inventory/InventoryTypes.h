#pragma once

#include "BaseInventorySlot.h"
#include "CoreMinimal.h"
#include "Data/ItemDataStructures.h"
#include "InventoryTypes.generated.h"

class UInventoryItemWidget;
class UItemBase;
class UBaseInventoryWidget;

USTRUCT(Blueprintable)
struct FItemSlotMapping
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<TObjectPtr<UBaseInventorySlot>> ItemSlots;
	UPROPERTY()
	TObjectPtr<UInventoryItemWidget> ItemVisualLinked;
};

UENUM(BlueprintType)
enum class EItemAddResult :uint8
{
	IAR_NoItemAdded UMETA(DisplayName = "No item added"),
	IAR_PartialAmountItemAdded UMETA(DisplayName = "Partial amount of item added"),
	IAR_AllItemAdded UMETA(DisplayName = "All of item added")
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
};

USTRUCT(BlueprintType)
struct FItemMoveData 
{
	GENERATED_BODY()

	UPROPERTY()
	UItemBase* SourceItem;
	UPROPERTY()
	UBaseInventoryWidget* SourceInventory;
	UPROPERTY()
	TObjectPtr<UBaseInventorySlot> SourceItemPivotSlot;
	UPROPERTY()
	TObjectPtr<UBaseInventoryWidget> TargetInventory;
	UPROPERTY()
	UBaseInventorySlot* TargetSlot;
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