//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/VendorComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Trade/TradeComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Factory/ItemFactory.h"

UVendorComponent::UVendorComponent()
{
}

void UVendorComponent::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateInteractableData();

	if (auto TradeComp = GetOwner()->FindComponentByClass<UTradeComponent>())
	{
		TradeComponentRef = TradeComp;
	}

	if (auto ItemCollection = GetOwner()->FindComponentByClass<UItemCollection>())
		ItemCollectionRef = ItemCollection;
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

	if (!TradeComponentRef ||!ItemCollectionRef)
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
	float Price = bIsBuyingFromVendor ? CalculateTotalSellPrice(TradeData.SourceItem) : CalculateTotalBuyPrice(TradeData.SourceItem);
	float AvailableMoney = bIsBuyingFromVendor ? TradePartnerItemCollection->CalculateAvailableMoney() : ItemCollectionRef->CalculateAvailableMoney();

	if (Price > 0 && AvailableMoney < Price)
	{
		FString Who = bIsBuyingFromVendor ? TEXT("Player") : TEXT("Vendor");
		float Deficit = Price - AvailableMoney;
		return FTradeResult::NotEnoughMoney(
			FText::FromString(FString::Printf(TEXT("%s doesn't have enough money. Need: %.0f, Has: %.0f, Missing: %.0f"),
				*Who, Price, AvailableMoney, Deficit)));
	}
	
	UInventoryBase* PlayerInventory = ResolvePlayerInventory(TradeData, bIsBuyingFromVendor);
	UInventoryBase* MoneyTargetInventory = bIsBuyingFromVendor ? MainVendorLootInventory : PlayerInventory;
	UInventoryBase* ItemTargetInventory = bIsBuyingFromVendor ? PlayerInventory : MainVendorLootInventory;

	// test money transfer
	UItemBase* CurrencyItem = UItemFactory::CreateItemByHandle(this, TradeSettings.CurrencyItemClass, Price);

	FItemMoveData MoneyMoveData;
	MoneyMoveData.SourceItem = CurrencyItem;
	MoneyMoveData.TargetInventory = MoneyTargetInventory;

	if (!CanTransferItem(MoneyMoveData))
	{
		FString Who = bIsBuyingFromVendor ? TEXT("Vendor") : TEXT("Player");
		return FTradeResult::InventoryFull(
			FText::FromString(FString::Printf(TEXT("%s inventory is full — no room for money"), *Who)));
	}

	// test item transfer
	FItemMoveData ItemMoveData;
	ItemMoveData.SourceItem = TradeData.SourceItem;
	ItemMoveData.TargetInventory = ItemTargetInventory;

	if (!CanTransferItem(ItemMoveData))
	{
		FString Who = bIsBuyingFromVendor ? TEXT("Player") : TEXT("Vendor");
		return FTradeResult::InventoryFull(
			FText::FromString(FString::Printf(TEXT("%s inventory is full — no room for item"), *Who)));
	}

	// --- ЕСЛИ ВСЕ ПРОВЕРКИ ПРОЙДЕНЫ: АТОМАРНАЯ СДЕЛКА ---

	if (bIsBuyingFromVendor)
	{
		RemoveCurrencyFromInventory(PlayerInventory, Price);
		ExecutePhysicalTransfer(TradeData); // Перемещаем товар
	}
	else
	{
		ExecutePhysicalTransfer(TradeData); // Сначала забираем товар
		AddCurrencyToInventory(PlayerInventory, Price); // Выдаем золото
	}

	Result.OperationResult = ETradeResult::TR_Success;
	return Result;
}

bool UVendorComponent::CanTransferItem(FItemMoveData ItemMoveData)
{
	auto Result = ItemMoveData.TargetInventory->HandleAddItem(ItemMoveData, true);
	return Result.OperationResult == EItemAddResult::IAR_AllItemAdded;
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

		int32 RemainingAmount = InitResource.Amount;
		while (RemainingAmount > 0)
		{
			UItemBase* NewItem = UItemFactory::CreateItemByHandle(this, InitResource.Item, RemainingAmount);
			if (!NewItem) break;

			RemainingAmount -= NewItem->GetQuantity();

			const EItemOrientationType InitOrientation = NewItem->GetInitialItemOrientation();
                
			FItemMoveData Data;
			Data.TargetInventory  = MainVendorLootInventory;
			Data.SourceItem       = NewItem;
			Data.SavedOrientation = InitOrientation;
			Data.TargetOrientation = InitOrientation;

			MainVendorLootInventory->HandleAddItem(Data);
		}
	}
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
	if (TradeData.SourceInventory && !TradeData.SourceInventory->GetInventorySettings().bIsReferenceContainer())
	{
		return TradeData.SourceInventory;
	}

	return TradePartnerMainInventory;
	
}

