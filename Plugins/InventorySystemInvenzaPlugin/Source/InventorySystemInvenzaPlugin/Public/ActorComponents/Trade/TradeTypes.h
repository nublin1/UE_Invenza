// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "TradeTypes.generated.h"

USTRUCT(BlueprintType)
struct FMoneyCalculationResult
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Category = "Money Calculation", VisibleAnywhere, BlueprintReadWrite)
	float AvailableMoney = 0.0f;

	UPROPERTY(Category = "Money Calculation", VisibleAnywhere, BlueprintReadWrite)
	bool bHasEnough = false;
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
	bool bSellOnly = false;
};