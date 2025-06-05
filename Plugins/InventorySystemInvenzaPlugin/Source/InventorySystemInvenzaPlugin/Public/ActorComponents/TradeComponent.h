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
class UIInventoryManager;
class UInvBaseContainerWidget;
class UItemCollection;
class UItemBase;

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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INVENTORYSYSTEMINVENZAPLUGIN_API UTradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable, Category = "Trade|Events")
	FOnSoldItem OnSoldItemDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Trade|Events")
	FOnBoughtItem OnBoughtItemDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Trade|Events")
	FOnFaildToBuyItem OnFaildToBuyItemDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Trade|Events")
	FOnFaildToSellItem OnFaildToSellItemDelegate;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Trade|UI")
	void OpenTradeMenu(AActor* Vendor, AActor* Buyer);
	UFUNCTION(BlueprintCallable, Category = "Trade|UI")
	void CloseTradeMenu();

	UFUNCTION()
	virtual bool TryBuyItem(UItemBase* ItemToBuy);
	UFUNCTION()
	virtual void BuyItem(UItemBase* ItemToBuy);
	UFUNCTION()
	virtual bool TrySellItem(UItemBase* ItemForSale);
	UFUNCTION()
	virtual void Selltem(UItemBase* ItemsToSell);

	UFUNCTION()
	virtual float GetTotalBuyPrice(UItemBase* ItemToBuy);
	UFUNCTION()
	virtual float GetTotalSellPrice(UItemBase* ItemsToSell);

	UFUNCTION()
	FTradeSettings GetTradeSettings() const {return TradeSettings;}

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trade|Config")
	FTradeSettings TradeSettings;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	static float CalculateAvailableMoney(UItemCollection* Collection);
	UFUNCTION()
	static FMoneyCalculationResult AccumulatePayment(UItemCollection* ItemCollection, float FullPrice);
};
