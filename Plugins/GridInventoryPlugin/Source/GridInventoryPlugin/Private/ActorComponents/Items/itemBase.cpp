// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/Items/itemBase.h"

#include "Data/ItemData.h"

UItemBase::UItemBase(): Quantity(0)
{
}

UItemBase* UItemBase::CreateFromDataTable(UDataTable* ItemDataTable, const FName& DesiredItemID, int32 InitQuantity)
{
	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemDataTable is null!"));
		return nullptr;
	}

	FItemData* ItemData = ItemDataTable->FindRow<FItemData>(DesiredItemID, TEXT("CreateFromDataTable"));
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item data not found for ID: %s"), *DesiredItemID.ToString());
		return nullptr;
	}
	
	UItemBase* NewItem = NewObject<UItemBase>();
	if (!NewItem)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create UItemBase object!"));
		return nullptr;
	}
	
	NewItem->SetItemRef(ItemData->ItemMetaData);
	int32 ClampedQuantity = FMath::Clamp(InitQuantity, 1, ItemData->ItemMetaData.ItemNumeraticData.MaxStackSize);
	NewItem->SetQuantity(ClampedQuantity);

	return NewItem;
}

bool UItemBase::bIsSameitems(UItemBase* FirstItem, UItemBase* SecondItem)
{
	if (FirstItem->GetItemRef().ItemTextData.Name.EqualTo(SecondItem->GetItemRef().ItemTextData.Name))
		return true;

	return false;
}

UItemBase* UItemBase::DuplicateItem()
{
	UItemBase* NewItem = NewObject<UItemBase>();
	if (NewItem)
	{
		NewItem->ItemRef = this->ItemRef;
		NewItem->Quantity = this->Quantity;
	}
	return NewItem;
}
