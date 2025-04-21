//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TradeService.generated.h"


class UUInventoryWidgetBase;
class UItemBase;
class UTradeComponent;

UENUM()
enum class ETradeResult : uint8
{
	Success,
	NotEnoughMoney,
	NoSpaceInInventory,
	VendorOutOfStock,
	UnknownError
};

USTRUCT()
struct FTradeRequest
{
	GENERATED_USTRUCT_BODY()
	
	TObjectPtr<UTradeComponent> Vendor;			// Торговый компонент продавца
	TObjectPtr<UUInventoryWidgetBase> VendorInv;
	TObjectPtr<UUInventoryWidgetBase> BuyerInv;
	TObjectPtr<UItemBase> Item;					// Что покупаем / продаём
	int32 Quantity;						// Сколько
};

/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UTradeService : public UObject
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	static ETradeResult ExecuteBuy(const FTradeRequest& Request);
	UFUNCTION()
	ETradeResult ExecuteSell(const FTradeRequest& Request);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
};
