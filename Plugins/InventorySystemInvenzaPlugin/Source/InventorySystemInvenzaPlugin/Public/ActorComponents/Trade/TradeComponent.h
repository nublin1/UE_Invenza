//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Trade/TradeTypes.h"
#include "TradeComponent.generated.h"

class UUInventoryWidgetBase;

struct FItemMapping;
class UIInventoryManager;
class UInventoryContainerWidget;
class UItemCollection;
class UItemBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INVENTORYSYSTEMINVENZAPLUGIN_API UTradeComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoldItem, UItemBase*, Item);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBoughtItem, UItemBase*, Item);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFaildToBuyItem, UItemBase*, Item);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFaildToSellItem, UItemBase*, Item);
#pragma endregion Delegates

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

	/*UFUNCTION()
	static FMoneyCalculationResult AccumulatePayment(UItemCollection* ItemCollection, float FullPrice);*/
};
