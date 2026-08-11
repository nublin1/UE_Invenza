//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "Data/Inventory/InventoryTypes.h"
#include "Interface/Interaction/VendorProvider.h"
#include "VendorComponent.generated.h"

class UTradeComponent;
class UInventoryContainerWidget;
class UUInventoryBaseWidget;
class UItemCollection;

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UVendorComponent : public UInteractableComponent, public IVendorProvider
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTradeTransaction, const FTradeTransaction&, Transaction);
#pragma endregion Delegates

public:
	UVendorComponent();

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable, Category="Trade")
	FOnTradeTransaction OnTradeExecuted;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void BeginFocus() override;
	virtual void EndFocus() override;
	
	virtual void Interact(UInteractionComponent* InteractionComponent) override;
	virtual void StopInteract(UInteractionComponent* InteractionComponent) override;

	virtual FTradeResult ProcessTradeRequest(const FItemMoveData& TradeData) override;
	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void Server_ProcessTradeRequest(const FItemMoveData& TradeData);
	UFUNCTION(BlueprintCallable)
	virtual FTradeResult HandleProcessTrade(const FItemMoveData& TradeData);

	UFUNCTION()
	virtual FTradeSettings GetTradeSettings() const override {return TradeSettings;}

	virtual const TObjectPtr<UInventoryBase>& GetVendorLootContainer() const override;

	virtual void SetTradePartnerInventory(UInventoryBase* InInventory) override {TradePartnerMainInventory = InInventory;}
	virtual void SetTradePartnerItemCollection(UItemCollection* InCollection) override {TradePartnerItemCollection = InCollection;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vendor|Config")
	FGameplayTag MainVendorContainerInvTag;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Vendor|Config")
	FTradeSettings TradeSettings;

	// Data
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInventoryBase> MainVendorLootInventory;

	// Refs
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Vendor")
	TObjectPtr<UItemCollection> ItemCollectionRef;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInventoryBase> TradePartnerMainInventory;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UItemCollection> TradePartnerItemCollection;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void InitializeInteractionComponent() override;
	virtual void UpdateInteractableData() override;

	UFUNCTION(BlueprintCallable)
	void InitializeVendorStartupData();

	UFUNCTION(BlueprintCallable)
	bool SimulateTrade(const FItemMoveData& TradeData,
		int32 Price, bool bIsBuyingFromVendor, UObject* CurrencyItem, FTradeResult& OutResult);

	UFUNCTION(BlueprintCallable)
	FTradeTransaction ExecuteTrade(const FItemMoveData& TradeData, int32 Price,
		bool bIsBuyingFromVendor, UInventoryBase* PlayerInventory, UObject* CurrencyItem);

	UFUNCTION(BlueprintCallable)
	float CalculateTotalBuyPrice(UObject* ItemToBuy);
	UFUNCTION(BlueprintCallable)
	float CalculateTotalSellPrice(UObject* ItemsToSell);

	UFUNCTION(BlueprintCallable)
	UInventoryBase* ResolvePlayerInventory(const FItemMoveData& TradeData, bool bIsBuyingFromVendor) const;
	
};
