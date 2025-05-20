//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TradeService.generated.h"


class UInvBaseContainerWidget;
class UUInventoryWidgetBase;
class UItemBase;
class UTradeComponent;

UENUM(NotBlueprintType)
enum class ETradeResult : uint8
{
	Success,
	ItemCantBeSold,
	NotEnoughMoney,
	NoSpaceInInventory,
	VendorOutOfStock,
	VendorDoesNotBuy,
	UnknownError
};

USTRUCT()
struct FTradeRequest
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TObjectPtr<UTradeComponent> Vendor;			// Торговый компонент продавца
	TObjectPtr<UInvBaseContainerWidget> VendorContainer;
	TObjectPtr<UInvBaseContainerWidget> BuyerContainer;
	UPROPERTY()
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
	static ETradeResult ExecuteSell(const FTradeRequest& Request);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
};
