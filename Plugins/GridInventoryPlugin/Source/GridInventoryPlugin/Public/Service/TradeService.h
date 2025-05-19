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

USTRUCT(BlueprintType)
struct FTradeRequest
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	bool bIsSaleOperation = false;
	UPROPERTY()
	TObjectPtr<UTradeComponent> Vendor;
	UPROPERTY()
	TObjectPtr<UInvBaseContainerWidget> VendorContainer;
	UPROPERTY()
	TObjectPtr<UInvBaseContainerWidget> BuyerContainer;
	UPROPERTY()
	TObjectPtr<UItemBase> Item;
	UPROPERTY()
	int32 Quantity;

	FTradeRequest(): Quantity(0)
	{
	}

	FTradeRequest(
		TObjectPtr<UTradeComponent> InVendor,
		TObjectPtr<UInvBaseContainerWidget> InVendorContainer,
		TObjectPtr<UInvBaseContainerWidget> InBuyerContainer,
		TObjectPtr<UItemBase> InItem,
		int32 InQuantity,
		bool bInIsSaleOperation)

		: bIsSaleOperation(bInIsSaleOperation)
		  , Vendor(InVendor)
		  , VendorContainer(InVendorContainer)
		  , BuyerContainer(InBuyerContainer)
		  , Item(InItem)
		  , Quantity(InQuantity)
	{
	}
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
