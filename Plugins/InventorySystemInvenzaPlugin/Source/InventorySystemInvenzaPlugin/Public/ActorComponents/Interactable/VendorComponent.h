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
class UInventoryCollectionComponent;

/**
 *
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UVendorComponent : public UInteractableComponent, public IVendorProvider
{
	GENERATED_BODY()

public:
	UVendorComponent();

protected:
	// Unreal overrides
	virtual void OnRegister() override;

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTradeTransaction, const FTradeTransaction&, Transaction);
#pragma endregion Delegates

	// Properties and variables
	UPROPERTY(BlueprintAssignable, Category = "Trade")
	FOnTradeTransaction OnTradeExecuted;

	// Functions
	virtual void BeginFocus() override;

	virtual void EndFocus() override;

	virtual void HandleInteract(UInteractionComponent* InteractionComponent) override;

	virtual void HandleStopInteract(UInteractionComponent* InteractionComponent, EInteractionType Type) override;

	virtual FTradeResult ProcessTradeRequest(const FItemMoveData& TradeData) override;

	UFUNCTION(Category = "Invenza|Interaction", BlueprintCallable, Server, Reliable)
	virtual void Server_ProcessTradeRequest(const FItemMoveData& TradeData);

	UFUNCTION(Category = "Invenza|Interaction", BlueprintCallable)
	virtual FTradeResult HandleProcessTrade(const FItemMoveData& TradeData);

	UFUNCTION()
	virtual FTradeSettings GetTradeSettings() const override { return TradeSettings; }

	virtual const TObjectPtr<UInventoryBase>& GetVendorLootContainer() const override;

	virtual void SetTradePartnerInventory(UInventoryBase* InInventory) override
	{
		TradePartnerMainInventory = InInventory;
	}

	virtual void SetTradePartnerItemCollection(UInventoryCollectionComponent* InCollection) override
	{
		TradePartnerItemCollection = InCollection;
	}

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vendor|Config")
	FGameplayTag MainVendorContainerInvTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Vendor|Config")
	FTradeSettings TradeSettings;

	// Data
	UPROPERTY(Category = "Invenza|Interaction", VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInventoryBase> MainVendorLootInventory;

	// Refs
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Vendor")
	TObjectPtr<UInventoryCollectionComponent> ItemCollectionRef;

	UPROPERTY(Category = "Invenza|Interaction", VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInventoryBase> TradePartnerMainInventory;

	UPROPERTY(Category = "Invenza|Interaction", VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInventoryCollectionComponent> TradePartnerItemCollection;

	// Functions
	virtual void InitializeInteractionComponent() override;

	virtual void UpdateInteractableData() override;

	UFUNCTION(Category = "Invenza|Interaction", BlueprintCallable)
	void InitializeVendorStartupData();

	UFUNCTION(Category = "Invenza|Interaction", BlueprintCallable)
	bool SimulateTrade(const FItemMoveData& TradeData, int32 Price, bool bIsBuyingFromVendor, UObject* CurrencyItem,
					   FTradeResult& OutResult);

	UFUNCTION(Category = "Invenza|Interaction", BlueprintCallable)
	FTradeTransaction ExecuteTrade(const FItemMoveData& TradeData, int32 Price, bool bIsBuyingFromVendor,
								   UInventoryBase* PlayerInventory, UObject* CurrencyItem);

	UFUNCTION(Category = "Invenza|Interaction", BlueprintCallable)
	float CalculateTotalBuyPrice(UObject* ItemToBuy);

	UFUNCTION(Category = "Invenza|Interaction", BlueprintCallable)
	float CalculateTotalSellPrice(UObject* ItemsToSell);

	UFUNCTION(Category = "Invenza|Interaction", BlueprintCallable)
	UInventoryBase* ResolvePlayerInventory(const FItemMoveData& TradeData, bool bIsBuyingFromVendor) const;
};
