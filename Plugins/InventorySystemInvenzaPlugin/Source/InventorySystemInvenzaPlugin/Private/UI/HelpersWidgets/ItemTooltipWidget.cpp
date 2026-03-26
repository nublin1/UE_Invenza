//  Nublin Studio 2026 All Rights Reserved.

#include "UI/HelpersWidgets/ItemTooltipWidget.h"
#include "Data/Items/itemBase.h"
#include "Components/TextBlock.h"
#include "Data/ItemDataStructures.h"


UItemTooltipWidget::UItemTooltipWidget()
{
}

void UItemTooltipWidget::SetTooltipData(UItemBase* Item, UInventoryBase* Inventory)
{
	if (!Item || !ItemName || !ItemType || !ItemDescription)
		return;
	
	auto ItemData = Item->GetItemRef();
	
	ItemName->SetText(ItemData.ItemTextData.DisplayName);
	ItemType->SetText(FText::FromString(Item->CategoryToString()));
	ItemDescription->SetText(ItemData.ItemTextData.ItemDescription);
	StackWeightValue->SetText(FText::AsNumber(Item->GetItemStackWeight()));

	const FString WeightInfo = {"Weight: " + FString::SanitizeFloat(Item->GetItemStackWeight())};
	StackWeightValue->SetText(FText::FromString(WeightInfo));

	if (Item->IsStackable())
	{
		const FString StackInfo = {FString::FromInt(ItemData.ItemNumeraticData.MaxStackSizeInCharacter)};
		MaxStackSize->SetText(FText::FromString("Max Stack Size: "));
		StackSizeValue->SetText(FText::FromString(StackInfo));
		MaxStackSize->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		MaxStackSize->SetVisibility(ESlateVisibility::Collapsed);
		StackSizeValue->SetText(FText::FromString("Unstackable"));
	}

	/*if (PriceText)
	{
		PriceText->SetText(FText::AsNumber(Item->GetItemRef().ItemTradeData.BasePrice * Item->GetQuantity()));

		UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
		if (!InventoryManager || !InventoryManager->GetCurrentInteractInvWidget())
			return;

		if (InventoryManager->GetCurrentInteractInvWidget()->GetInventoryType() != EInventoryType::VendorInventory)
			return;
	
		if (!Inventory || !Inventory->GetItemCollectionLinked()) return;		
		auto OwnerAc = InventoryManager->GetCurrentInteractInvWidget()->GetInventoryFromContainerSlot()->GetInventoryData().ItemCollectionLink->GetOwner();
		if (!OwnerAc) return;
		auto TradeComp = OwnerAc->FindComponentByClass<UTradeComponent>();
		if (!TradeComp)
		{
			return;
		}
		
		if (Inventory == InventoryManager->GetCurrentInteractInvWidget()->GetInventoryFromContainerSlot())
		{
			FText FullPriceText = FText::AsNumber(TradeComp->GetTotalSellPrice(Item));
			PriceText->SetText(FullPriceText);
			return;
		}

		else if (InventoryManager->GetMainInventory()->GetInventoryFromContainerSlot() == Inventory)
		{
			FText FullPriceText = FText::AsNumber(TradeComp->GetTotalBuyPrice(Item));
			PriceText->SetText(FullPriceText);
		}
	}*/
}
