//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TradeComponent.generated.h"

#pragma region Delegates
class UItemBase;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoldItem, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBoughtItem, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFaildToBuyItem, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFaildToSellItem, UItemBase*, Item);
#pragma endregion Delegates

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
	void OpenTradeMenu(AActor* Seller, AActor* Buyer);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY()
	AActor* SellerActor = nullptr;
	UPROPERTY()
	AActor* BuyerActor = nullptr;

	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION()
	virtual void BuyItem(UItemBase* Item);
	UFUNCTION()
	virtual void Selltem(UItemBase* Item);

		
};
