//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/VendorComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/Simulator/InventorySimulator.h"
#include "Data/Settings/InvenzaInventoryUISettingsAsset.h"
#include "Factory/ItemFactory.h"
#include "Utility/InventoryUtility.h"

UVendorComponent::UVendorComponent()
{
}

void UVendorComponent::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateInteractableData();

	if (auto ItemCollection = GetOwner()->FindComponentByClass<UItemCollection>())
		ItemCollectionRef = ItemCollection;

	InitializeInventoryStartupData();
}

void UVendorComponent::BeginFocus()
{
	Super::BeginFocus();
}

void UVendorComponent::EndFocus()
{
	Super::EndFocus();
}

void UVendorComponent::Interact(UInteractionComponent* InteractionComponent)
{
	Super::Interact(InteractionComponent);

	if (!ItemCollectionRef)
		return;
	
	if (!bIsInteract)
	{
		bIsInteract = true;
		return;
	}
	
	bIsInteract = false;
}

void UVendorComponent::StopInteract(UInteractionComponent* InteractionComponent)
{
	Super::StopInteract(InteractionComponent);
	
	bIsInteract = false;
}

FTradeResult UVendorComponent::ProcessTradeRequest(const FItemMoveData& TradeData)
{
	FTradeResult Result;
	
	if (TradeData.SourceItem->GetItemRef().ItemCategory == EItemCategory::Money)
	{
		return FTradeResult::Failed(FText::FromString("Try transfer money"));
	}
	
	bool bIsBuyingFromVendor = (TradeData.SourceInventory == MainVendorLootInventory);
	float Price = bIsBuyingFromVendor
		? CalculateTotalSellPrice(TradeData.SourceItem)
		: CalculateTotalBuyPrice(TradeData.SourceItem);
	
	float AvailableMoney = bIsBuyingFromVendor
		? TradePartnerItemCollection->CalculateAvailableMoney()
		: ItemCollectionRef->CalculateAvailableMoney();

	if (Price > 0 && AvailableMoney < Price)
	{
		FString Who = bIsBuyingFromVendor ? TEXT("Player") : TEXT("Vendor");
		float Deficit = Price - AvailableMoney;
		return FTradeResult::NotEnoughMoney(
			FText::FromString(FString::Printf(TEXT("%s doesn't have enough money. Need: %.0f, Has: %.0f, Missing: %.0f"),
				*Who, Price, AvailableMoney, Deficit)));
	}

	UItemBase* CurrencyItem = UItemFactory::CreateItemByHandle(this,
		UInventoryUtility::GetInvenzaGlobalSettings(GetWorld())->CurrencyItemClass, Price);
	if (!CurrencyItem)
		return FTradeResult::Failed(FText::FromString("CurrencyItemClass is not set"));
	
	FTradeResult SimulationResult;
	if (!SimulateTrade(TradeData, Price, bIsBuyingFromVendor, CurrencyItem, SimulationResult))
	{
		return SimulationResult;
	}

	// FINAL TRADE EXECUTION
	UInventoryBase* PlayerInventory = ResolvePlayerInventory(TradeData, bIsBuyingFromVendor);
	ExecuteTrade(TradeData, Price, bIsBuyingFromVendor, PlayerInventory, CurrencyItem);

	Result.OperationResult = ETradeResult::TR_Success;
	return Result;
}

const TObjectPtr<UInventoryBase>& UVendorComponent::GetVendorLootContainer() const
{
	return MainVendorLootInventory;
}

void UVendorComponent::InitializeInteractionComponent()
{
	Super::InitializeInteractionComponent();
	
	UpdateInteractableData();
}

void UVendorComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();
	InteractableData.DefaultInteractableType = EInteractableType::Vendor;
	InteractableData.Action = FText::FromString(TEXT("Trade"));
	InteractableData.Quantity = -1;
}

void UVendorComponent::InitializeInventoryStartupData()
{
	if (!MainVendorContainerInvTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("MainVendorContainerInvTag is not set!"));
		return;
	}

	InventoryStartupData.Settings.InventoryType = EInventoryType::VendorInventory;
	InventoryStartupData.Settings.bIsReferenceContainer = false;

	if (InventoryStartupData.Settings.InventoryTag != MainVendorContainerInvTag)
	{
		return;
	}

	UInventoryBase* Inventory =	UInventoryBase::CreateInventory(this, InventoryStartupData);
	if (!Inventory)
		return;

	MainVendorLootInventory = Inventory;
	
	MainVendorLootInventory->InitInventory();	
	MainVendorLootInventory->SetItemCollectionLink(ItemCollectionRef);
	MainVendorLootInventory->SetInventorySettings(InventoryStartupData.Settings);

	SetupStartingResources();
}

void UVendorComponent::SetupStartingResources()
{
	if (!MainVendorLootInventory)
		return;
	
	for (const auto& InitResource : InventoryStartupData.StartItems)
	{
		if (InitResource.Item.RowName.IsNone()) continue;

		UItemBase* NewItemSample = UItemFactory::CreateItemByHandle(this, InitResource.Item, 1);

		UInventoryUtility::AddItemQuantity(this, MainVendorLootInventory, NewItemSample, InitResource.Amount);
	}
}

bool UVendorComponent::SimulateTrade(const FItemMoveData& TradeData, int32 Price, bool bIsBuyingFromVendor,
	UItemBase* CurrencyItem, FTradeResult& OutResult)
{
	UInventoryBase* PlayerInventory = ResolvePlayerInventory(TradeData, bIsBuyingFromVendor);
	UInventoryBase* MoneyTargetInventory = bIsBuyingFromVendor ? MainVendorLootInventory.Get() : PlayerInventory;
	UInventoryBase* ItemTargetInventory = bIsBuyingFromVendor ? PlayerInventory : MainVendorLootInventory.Get();

	// create simulators
	UInventorySimulator* MoneySim = NewObject<UInventorySimulator>(this);
	UInventorySimulator* ItemSim = NewObject<UInventorySimulator>(this);

	MoneySim->DuplicateInventoryForSimulation(MoneyTargetInventory);
	ItemSim->DuplicateInventoryForSimulation(ItemTargetInventory);
	
	// MONEY TRANSFER SIMULATION
	MoneySim->TransferRequestSimulateQuantity(CurrencyItem, Price);
	
	if (!MoneySim->AreAllOperationsSuccessful())
	{
		FString Who = bIsBuyingFromVendor ? TEXT("Vendor") : TEXT("Player");

		OutResult = FTradeResult::InventoryFull(
			FText::FromString(FString::Printf(TEXT("%s inventory is full — no room for money"), *Who)));

		return false;
	}
	
	// ITEM TRANSFER SIMULATION
	FItemMoveData ItemMoveData;
	ItemMoveData.SourceItem = TradeData.SourceItem;
	ItemMoveData.TargetInventory = ItemSim->GetSimulationInventory();
	ItemMoveData.TargetSlotCoordinate = TradeData.TargetSlotCoordinate;
	ItemMoveData.SavedOrientation = TradeData.SavedOrientation;
	ItemMoveData.TargetOrientation = TradeData.TargetOrientation;

	ItemSim->TransferRequestSimulate(ItemMoveData);

	if (!ItemSim->WasLastOperationSuccessful())
	{
		FString Who = bIsBuyingFromVendor ? TEXT("Player") : TEXT("Vendor");

		OutResult = FTradeResult::InventoryFull(
			FText::FromString(FString::Printf(TEXT("%s inventory is full — no room for item"), *Who)));

		return false;
	}

	return true;
}

FTradeTransaction UVendorComponent::ExecuteTrade(const FItemMoveData& TradeData, int32 Price, bool bIsBuyingFromVendor,
	UInventoryBase* PlayerInventory, UItemBase* CurrencyItem)
{
	FTradeTransaction Transaction;
    Transaction.bSuccess = true;
    Transaction.bIsBuyingFromVendor = bIsBuyingFromVendor;
    Transaction.TotalPrice = Price;

    const int32 Quantity = TradeData.SourceItem->GetQuantity();

    if (bIsBuyingFromVendor)
    {
        // PLAYER PAYS
        PlayerInventory->HandleRemoveItemsByType(CurrencyItem, Price);

        Transaction.Entries.Add({
            PlayerInventory,
            CurrencyItem,
            -Price,
            true
        });

        // VENDOR RECEIVES
        UInventoryUtility::AddItemQuantity(this, MainVendorLootInventory, CurrencyItem, Price);

        Transaction.Entries.Add({
            MainVendorLootInventory,
            CurrencyItem,
            +Price,
            true
        });

        // REMOVE ITEM FROM VENDOR
        if (TradeSettings.RemoveItemAfterPurchase)
        {
            MainVendorLootInventory->HandleRemoveItem(TradeData.SourceItem, Quantity);

            Transaction.Entries.Add({
                MainVendorLootInventory,
                TradeData.SourceItem,
                -Quantity,
                false
            });
        }

        // ADD ITEM TO PLAYER
        PlayerInventory->HandleAddItem(TradeData);

        Transaction.Entries.Add({
            PlayerInventory,
            TradeData.SourceItem,
            +Quantity,
            false
        });
    }
    else
    {
        // REMOVE ITEM FROM PLAYER
        PlayerInventory->HandleRemoveItem(TradeData.SourceItem, Quantity);

        Transaction.Entries.Add({
            PlayerInventory,
            TradeData.SourceItem,
            -Quantity,
            false
        });

        // PLAYER RECEIVES MONEY
        UInventoryUtility::AddItemQuantity(this, PlayerInventory, CurrencyItem, Price);

        Transaction.Entries.Add({
            PlayerInventory,
            CurrencyItem,
            +Price,
            true
        });

        // VENDOR RECEIVES ITEM
        UInventoryUtility::AddItemQuantity(this, MainVendorLootInventory, TradeData.SourceItem, Quantity);

        Transaction.Entries.Add({
            MainVendorLootInventory,
            TradeData.SourceItem,
            +Quantity,
            false
        });
    }
	
	OnTradeExecuted.Broadcast(Transaction);

    return Transaction;
}

float UVendorComponent::CalculateTotalBuyPrice(UItemBase* ItemToBuy)
{
	auto FullPrice = ItemToBuy->GetItemRef().ItemTradeData.BasePrice * TradeSettings.BuyPriceFactor * ItemToBuy->GetQuantity();
	return FullPrice;
}

float UVendorComponent::CalculateTotalSellPrice(UItemBase* ItemsToSell)
{
	auto FullPrice = ItemsToSell->GetItemRef().ItemTradeData.BasePrice * TradeSettings.SellPriceFactor * ItemsToSell->GetQuantity();
	return FullPrice;
}

UInventoryBase* UVendorComponent::ResolvePlayerInventory(const FItemMoveData& TradeData, bool bIsBuyingFromVendor) const
{
	if (bIsBuyingFromVendor)
	{
		// Player buys → item goes to TargetInventory
		if (TradeData.TargetInventory && !TradeData.TargetInventory->GetInventorySettings().bIsReferenceContainer)
		{
			return TradeData.TargetInventory;
		}

		return TradePartnerMainInventory;
	}
	
	// Player sells → item comes from SourceInventory
	if (TradeData.SourceInventory && !TradeData.SourceInventory->GetInventorySettings().bIsReferenceContainer)
	{
		return TradeData.SourceInventory;
	}

	return TradePartnerMainInventory;
	
}

