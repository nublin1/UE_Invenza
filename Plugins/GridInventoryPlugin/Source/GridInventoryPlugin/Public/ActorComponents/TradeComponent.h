//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TradeComponent.generated.h"

#pragma region Delegates
class UUInventoryWidgetBase;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoldItem, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBoughtItem, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFaildToBuyItem, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFaildToSellItem, UItemBase*, Item);
#pragma endregion Delegates

struct FItemMapping;
class AUIManagerActor;
class UInvBaseContainerWidget;
class UItemCollection;
class UItemBase;

USTRUCT()
struct FMoneyCalculationResult
{
	GENERATED_USTRUCT_BODY()
	
	float AvailableMoney = 0.0f;
	float AccumulatedRequiredValue = 0.0f;
	bool bHasEnough = false;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GRIDINVENTORYPLUGIN_API UTradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	FOnSoldItem OnSoldItemDelegate;
	FOnBoughtItem OnBoughtItemDelegate;
	FOnFaildToBuyItem OnFaildToBuyItemDelegate;
	FOnFaildToSellItem OnFaildToSellItemDelegate;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Trade")
	void OpenTradeMenu(AActor* Vendor, AActor* Buyer);
	UFUNCTION(BlueprintCallable, Category = "Trade")
	void CloseTradeMenu();

	UFUNCTION()
	virtual bool TryBuyItem(UItemBase* ItemToBuy);
	UFUNCTION()
	virtual void BuyItem(UItemBase* ItemToBuy, UUInventoryWidgetBase* VendorInv, UUInventoryWidgetBase* BuyerInv);
	UFUNCTION()
	virtual bool TrySellItem(UItemBase* ItemForSale);
	UFUNCTION()
	virtual void Selltem(UItemBase* ItemsToSell, FMoneyCalculationResult Result);

	UFUNCTION()
	virtual float GetTotalBuyPrice(UItemBase* ItemToBuy);
	UFUNCTION()
	virtual float GetTotalSellPrice(UItemBase* ItemsToSell);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY()
	AActor* VendorActor = nullptr;
	UPROPERTY()
	AActor* BuyerActor = nullptr;
	UPROPERTY()
	UItemCollection* VendorItemCollection = nullptr;
	UPROPERTY()
	UItemCollection* BuyerItemCollection = nullptr;
	
	//Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BuyPriceFactor = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SellPriceFactor = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool RemoveItemAfterPurchase = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSellOnly = false;

	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION()
	float CalculateAvailableMoney(UItemCollection* Collection);
	UFUNCTION()
	FMoneyCalculationResult AccumulatePayment(UItemCollection* ItemCollection, float FullPrice);
	
	
	UFUNCTION()
	virtual void AbortDeal(UItemBase* Item);
		
};
