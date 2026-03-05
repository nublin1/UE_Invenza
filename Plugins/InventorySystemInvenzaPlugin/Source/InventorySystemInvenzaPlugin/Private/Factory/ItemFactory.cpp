//  Nublin Studio 2025 All Rights Reserved.


#include "Factory/ItemFactory.h"

#include "Data/ItemData.h"
#include "Data/Items/itemBase.h"

UItemBase* UItemFactory::CreateItemByID(UObject* Outer, UDataTable* InTable, FName ID, int32 Quantity)
{
	if (!InTable) return nullptr;

	FItemData* Row = InTable->FindRow<FItemData>(ID, TEXT("CreateItemByID"));
	if (!Row) return nullptr;

	UItemBase* NewItem = NewObject<UItemBase>(Outer);
	NewItem->InitItem(ID, *Row, Quantity);
	return NewItem;
}

UItemBase* UItemFactory::CreateItemByHandle(UObject* Outer, FDataTableRowHandle Handle, int32 Quantity)
{
	if (!Handle.DataTable || Handle.RowName.IsNone()) return nullptr;

	FItemData* Row = Handle.GetRow<FItemData>(TEXT("CreateItemByHandle"));
	if (!Row) return nullptr;

	UItemBase* NewItem = NewObject<UItemBase>(Outer);
	NewItem->InitItem(Handle.RowName, *Row, Quantity);
	return NewItem;
}