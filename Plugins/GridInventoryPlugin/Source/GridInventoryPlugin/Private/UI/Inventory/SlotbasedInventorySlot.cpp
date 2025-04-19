//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/SlotbasedInventorySlot.h"

#include "Components/TextBlock.h"

USlotbasedInventorySlot::USlotbasedInventorySlot()
{
}

void USlotbasedInventorySlot::SetItemUseKeyText(FString InUseKeyText)
{
	Super::SetItemUseKeyText(InUseKeyText);
	if (UseAction)
	{
		ItemUseKey->SetText(FText::FromString(InUseKeyText));
		ItemUseKey->SetVisibility(ESlateVisibility::Visible);
	}
}

void USlotbasedInventorySlot::NativeConstruct()
{
	Super::NativeConstruct();

	SetItemUseKeyText(InUseKeyTextByDefault);
}
