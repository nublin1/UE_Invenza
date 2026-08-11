//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/VendorComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/Simulator/InventorySimulator.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Factory/ItemFactory.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "Utility/InterfaceUtils.h"
#include "Utility/InvenzayUtility.h"

UVendorComponent::UVendorComponent()
{
	SetIsReplicatedByDefault(true);
}

void UVendorComponent::OnRegister()
{
	Super::OnRegister();

	if (AActor* Owner = GetOwner())
	{
		Owner->SetReplicates(true);
		Owner->SetReplicateMovement(true);
	}
}

void UVendorComponent::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateInteractableData();

	if (auto ItemCollection = GetOwner()->FindComponentByClass<UItemCollection>())
		ItemCollectionRef = ItemCollection;

	if (auto InventoryManager = GetOwner()->FindComponentByClass<UIInventoryManager>())
	{
		InventoryManager->OnInitializationCompleteDelegate.AddDynamic(this, &UVendorComponent::InitializeVendorStartupData);
	}
}

void UVendorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVendorComponent, TradeSettings);
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
	
	if (!bIsInteracting)
	{
		SetInteracting(true);
		return;
	}
	
	SetInteracting(false);
}

void UVendorComponent::StopInteract(UInteractionComponent* InteractionComponent)
{
	Super::StopInteract(InteractionComponent);
	
	SetInteracting(false);
}

FTradeResult UVendorComponent::ProcessTradeRequest(const FItemMoveData& TradeData)
{
	FTradeResult Result;
	
	if (!GetOwner()->HasAuthority())
	{
		Server_ProcessTradeRequest(TradeData);
	}
	else
	{
		Result = HandleProcessTrade(TradeData);
	}
	
	return Result;
}

void UVendorComponent::Server_ProcessTradeRequest_Implementation(const FItemMoveData& TradeData)
{
	HandleProcessTrade(TradeData);
}

FTradeResult UVendorComponent::HandleProcessTrade(const FItemMoveData& TradeData)
{
	FTradeResult Result;

	if (!TradeData.SourceItem ||
		!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(TradeData.SourceItem, TEXT("HandleProcessTrade")))
	{
		return FTradeResult::Failed(FText::FromString("Invalid item"));
	}

	const auto* MySettings = UInvenzaInventorySettingsSubsystem::GetSettingsStatic(this);
	if (MySettings &&
		IObjectDataProvider::Execute_GetItemRef(TradeData.SourceItem).ItemCategory == MySettings->CurrencyGameplayTag)
	{
		return FTradeResult::Failed(FText::FromString("Try transfer money"));
	}
	
	bool bIsBuyingFromVendor = (TradeData.SourceInventory == MainVendorLootInventory);
	int Price = bIsBuyingFromVendor
		? CalculateTotalSellPrice(TradeData.SourceItem)
		: CalculateTotalBuyPrice(TradeData.SourceItem);
	
	int AvailableMoney = bIsBuyingFromVendor
		? TradePartnerItemCollection->CalculateAvailableMoney()
		: ItemCollectionRef->CalculateAvailableMoney();

	if (Price > 0 && AvailableMoney < Price)
	{
		FString Who = bIsBuyingFromVendor ? TEXT("Player") : TEXT("Vendor");
		float Deficit = Price - AvailableMoney;
		return FTradeResult::NotEnoughMoney(
			FText::FromString(FString::Printf(TEXT("%s doesn't have enough money. Need: %.0i, Has: %.0i, Missing: %.0f"),
				*Who, Price, AvailableMoney, Deficit)));
	}

	auto* Settings = UInvenzayUtility::GetInvenzaGlobalSettings(GetWorld());
	if (!Settings)
	{
		return FTradeResult::Failed(FText::FromString("Settings not found"));
	}
	
	const FDataTableRowHandle& ItemHandle = Settings->CurrencyItemClass;
	if (!ItemHandle.DataTable || ItemHandle.RowName.IsNone())
	{
		return FTradeResult::Failed(FText::FromString("CurrencyItemClass is not set"));
	}
	UObject* CurrencyItem = UItemFactory::CreateItemByHandle(this, ItemHandle, Price);
	
	
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

void UVendorComponent::InitializeVendorStartupData()
{
	if (!MainVendorContainerInvTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("MainVendorContainerInvTag is not set!"));
		return;
	}

	if (!ItemCollectionRef)
		return;

	if (auto InventoryManager = GetOwner()->FindComponentByClass<UIInventoryManager>())
	{
		if (auto FindResult = ItemCollectionRef->GetInventoryByTag(MainVendorContainerInvTag))
		{
			MainVendorLootInventory = FindResult;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MainVendorContainerInvTag is invalid"));
		}
	}
}

bool UVendorComponent::SimulateTrade(const FItemMoveData& TradeData, int32 Price, bool bIsBuyingFromVendor,
	UObject* CurrencyItem, FTradeResult& OutResult)
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
	ItemMoveData.TargetSlotID = TradeData.TargetSlotID;
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
	UInventoryBase* PlayerInventory, UObject* CurrencyItem)
{
	FTradeTransaction Transaction;
	Transaction.bSuccess = true;
	Transaction.bIsBuyingFromVendor = bIsBuyingFromVendor;
	Transaction.TotalPrice = Price;

	if (!TradeData.SourceItem ||
		!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(TradeData.SourceItem, TEXT("ExecuteTrade")))
		return Transaction;

	const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(TradeData.SourceItem);

    if (bIsBuyingFromVendor)
    {
        // PLAYER PAYS
        PlayerInventory->HandleRemoveItemsBySample(CurrencyItem, Price);

        Transaction.Entries.Add({
            PlayerInventory,
            CurrencyItem,
            -Price,
            true
        });

        // VENDOR RECEIVES
        UInvenzayUtility::AddItemQuantityBySample(this, MainVendorLootInventory, CurrencyItem, Price);

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
        UInvenzayUtility::AddItemQuantityBySample(this, PlayerInventory, CurrencyItem, Price);

        Transaction.Entries.Add({
            PlayerInventory,
            CurrencyItem,
            +Price,
            true
        });

    	// VENDOR RECEIVES ITEM
    	if (TradeSettings.bAddPurchasedItemsToVendorDisplay)
    	{
    		UInvenzayUtility::AddItemQuantityBySample(this, MainVendorLootInventory, TradeData.SourceItem, Quantity);

    		Transaction.Entries.Add({
				MainVendorLootInventory,
				TradeData.SourceItem,
				+Quantity,
				false
			});
    	}
    	
    }
	
	OnTradeExecuted.Broadcast(Transaction);

    return Transaction;
}

float UVendorComponent::CalculateTotalBuyPrice(UObject* ItemToBuy)
{
	if (!ItemToBuy ||
	   !UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemToBuy, TEXT("CalculateTotalBuyPrice")))
		return 0.f;

	const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(ItemToBuy);
	const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(ItemToBuy);

	return Meta.ItemTradeData.BasePrice * TradeSettings.BuyPriceFactor * Quantity;
}

float UVendorComponent::CalculateTotalSellPrice(UObject* ItemsToSell)
{
	if (!ItemsToSell ||
	   !UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemsToSell, TEXT("CalculateTotalSellPrice")))
		return 0.f;

	const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(ItemsToSell);
	const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(ItemsToSell);

	return Meta.ItemTradeData.BasePrice * TradeSettings.SellPriceFactor * Quantity;
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

