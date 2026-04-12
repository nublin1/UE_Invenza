// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "TradeTypes.generated.h"


class UInventoryBase;
class UItemBase;

UENUM(BlueprintType)
enum class ETradeResult : uint8
{
	TR_Success UMETA(DisplayName = "Trade Success"),
	TR_NotEnoughMoney UMETA(DisplayName = "Not Enough Money"),
	TR_VendorDoesNotBuyItems UMETA(DisplayName = "Vendor Does Not Buy Items"),
	TR_InventoryFull UMETA(DisplayName = "Inventory Full"),
	TR_InvalidItem UMETA(DisplayName = "Invalid Item"),
	TR_Failed UMETA(DisplayName = "Trade Failed")
};

USTRUCT(BlueprintType, Blueprintable)
struct FTradeSettings
{
	GENERATED_BODY()

	UPROPERTY(Category = "Trade Settings", EditAnywhere, BlueprintReadOnly)
	float BuyPriceFactor = 1.0f;
	UPROPERTY(Category = "Trade Settings", EditAnywhere, BlueprintReadOnly)
	float SellPriceFactor = 1.0f;
	UPROPERTY(Category = "Trade Settings", EditAnywhere, BlueprintReadOnly)
	bool RemoveItemAfterPurchase = false;
	UPROPERTY(Category = "Trade Settings", EditAnywhere, BlueprintReadOnly)
	bool bAddPurchasedItemsToVendorDisplay = true;
	UPROPERTY(Category = "Trade Settings", EditAnywhere, BlueprintReadOnly)
	bool bSellOnly = false;
	
};

USTRUCT(BlueprintType)
struct FTradeContext
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	AActor* Buyer = nullptr;
	UPROPERTY(BlueprintReadWrite)
	AActor* Vendor = nullptr;
	
	UPROPERTY(BlueprintReadWrite)
	FTradeSettings TradeSettings;
};

USTRUCT(BlueprintType, meta=(ScriptName="FTradeResult"))
struct FTradeResult
{
	GENERATED_BODY()

	FTradeResult():
		ItemsTraded(0),
		MoneySpent(0),
		MoneyReceived(0),
		OperationResult(ETradeResult::TR_Failed),
		ResultMessage(FText::GetEmpty())
	{
	}

	// Amount of items traded
	UPROPERTY(BlueprintReadOnly, Category="Trade Result")
	int32 ItemsTraded;

	// Money player spent
	UPROPERTY(BlueprintReadOnly, Category="Trade Result")
	int32 MoneySpent;

	// Money player received
	UPROPERTY(BlueprintReadOnly, Category="Trade Result")
	int32 MoneyReceived;

	// Trade result
	UPROPERTY(BlueprintReadOnly, Category="Trade Result")
	ETradeResult OperationResult;

	// Describes the result
	UPROPERTY(BlueprintReadOnly, Category="Trade Result")
	FText ResultMessage;

	static FTradeResult Success(const int32 InItemsTraded, const int32 InMoneySpent, const int32 InMoneyReceived,
	                            const FText& Message)
	{
		FTradeResult Result;
		Result.ItemsTraded = InItemsTraded;
		Result.MoneySpent = InMoneySpent;
		Result.MoneyReceived = InMoneyReceived;
		Result.OperationResult = ETradeResult::TR_Success;
		Result.ResultMessage = Message;
		return Result;
	}

	static FTradeResult NotEnoughMoney(const FText& ErrorText)
	{
		FTradeResult Result;
		Result.OperationResult = ETradeResult::TR_NotEnoughMoney;
		Result.ResultMessage = ErrorText;
		return Result;
	}

	static FTradeResult VendorDoesNotBuy(const FText& ErrorText)
	{
		FTradeResult Result;
		Result.OperationResult = ETradeResult::TR_VendorDoesNotBuyItems;
		Result.ResultMessage = ErrorText;
		return Result;
	}

	static FTradeResult InventoryFull(const FText& ErrorText)
	{
		FTradeResult Result;
		Result.OperationResult = ETradeResult::TR_InventoryFull;
		Result.ResultMessage = ErrorText;
		return Result;
	}

	static FTradeResult Failed(const FText& ErrorText)
	{
		FTradeResult Result;
		Result.OperationResult = ETradeResult::TR_Failed;
		Result.ResultMessage = ErrorText;
		return Result;
	}
};

USTRUCT(BlueprintType)
struct FTradeEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UInventoryBase> Inventory = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UItemBase> Item = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 QuantityDelta = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bIsCurrency = false;

	FTradeEntry(){};

	FTradeEntry(UInventoryBase* InInventory, UItemBase* InItem, int32 InQuantityDelta, bool bInIsCurrency)
		: Inventory(InInventory)
		, Item(InItem)
		, QuantityDelta(InQuantityDelta)
		, bIsCurrency(bInIsCurrency)
	{}
};

USTRUCT(BlueprintType)
struct FTradeTransaction
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsBuyingFromVendor = false;

	UPROPERTY(BlueprintReadOnly)
	float TotalPrice = 0;

	UPROPERTY(BlueprintReadOnly)
	TArray<FTradeEntry> Entries;

	UPROPERTY(BlueprintReadOnly)
	FText Message;
};
