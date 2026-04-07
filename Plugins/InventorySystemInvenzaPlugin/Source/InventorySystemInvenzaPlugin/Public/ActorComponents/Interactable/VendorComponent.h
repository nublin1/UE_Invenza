//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "Data/Inventory/InventoryTypes.h"
#include "Interface/Interaction/VendorProvider.h"
#include "VendorComponent.generated.h"

class UTradeComponent;
class UInvBaseContainerWidget;
class UUInventoryWidgetBase;
class UItemCollection;

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UVendorComponent : public UInteractableComponent, public IVendorProvider
{
	GENERATED_BODY()

public:
	UVendorComponent();

protected:
	virtual void BeginPlay() override;
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void BeginFocus() override;
	virtual void EndFocus() override;
	
	virtual void Interact(UInteractionComponent* InteractionComponent) override;
	virtual void StopInteract(UInteractionComponent* InteractionComponent) override;

	virtual FTradeResult ProcessTradeRequest(const FItemMoveData& TradeData) override;

	virtual bool CanTransferItem(FItemMoveData ItemMoveData);

	UFUNCTION()
	FTradeSettings GetTradeSettings() const {return TradeSettings;}

	virtual const TObjectPtr<UInventoryBase>& GetVendorLootContainer() const override;

	virtual void SetTradePartnerInventory(UInventoryBase* InInventory) override {TradePartnerMainInventory = InInventory;}
	virtual void SetTradePartnerItemCollection(UItemCollection* InCollection) override {TradePartnerItemCollection = InCollection;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY()
	bool bIsInteract = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	FInventoryStartupData InventoryStartupData;

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vendor|Config")
	FGameplayTag MainVendorContainerInvTag;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor|Config")
	FTradeSettings TradeSettings;

	// Data
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInventoryBase> MainVendorLootInventory;

	// Refs
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vendor")
	TObjectPtr<UItemCollection> ItemCollectionRef;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category="Vendor|Components")
	TObjectPtr<UTradeComponent> TradeComponentRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInventoryBase> TradePartnerMainInventory;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UItemCollection> TradePartnerItemCollection;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void InitializeInteractionComponent() override;
	virtual void UpdateInteractableData() override;

	UFUNCTION(BlueprintCallable)
	void InitializeInventoryStartupData();
	UFUNCTION(BlueprintCallable)
	void SetupStartingResources();

	UFUNCTION(BlueprintCallable)
	float CalculateTotalBuyPrice(UItemBase* ItemToBuy);
	UFUNCTION(BlueprintCallable)
	float CalculateTotalSellPrice(UItemBase* ItemsToSell);

	UFUNCTION(BlueprintCallable)
	UInventoryBase* ResolvePlayerInventory(const FItemMoveData& TradeData, bool bIsBuyingFromVendor) const;
	
};
