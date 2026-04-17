//  Nublin Studio 2025 All Rights Reserved.


#include "Factory/ItemFactory.h"

#include "Data/ItemData.h"
#include "Data/Items/itemBase.h"

UItemBase* UItemFactory::CreateItemByHandle(UObject* Outer, FDataTableRowHandle Handle, int32 Quantity)
{
	if (!Outer) return nullptr;
	
	if (!Handle.DataTable || Handle.RowName.IsNone()) return nullptr;

	FItemData* Row = Handle.GetRow<FItemData>(TEXT("CreateItemByHandle"));
	if (!Row) return nullptr;

	UItemBase* NewItem = NewObject<UItemBase>(Outer);
	NewItem->SetItemRow(Handle);
	NewItem->InitItem(Handle.RowName, *Row, Quantity);
	return NewItem;
}