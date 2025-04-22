//  Nublin Studio 2025 All Rights Reserved.


#include "Factory/ItemFactory.h"

#include "ActorComponents/Items/itemBase.h"
#include "Data/ItemData.h"

UDataTable* UItemFactory::ItemTable = nullptr;

void UItemFactory::Init(UDataTable* InTable)
{
	ItemTable = InTable;
}

UItemBase* UItemFactory::CreateItemByID(UObject* Outer, FName ID, int32 Quantity)
{
	if (!ItemTable) return nullptr;

	FItemData* Row = ItemTable->FindRow<FItemData>(ID, TEXT("CreateItemByID"));
	if (!Row) return nullptr;

	UItemBase* NewItem = NewObject<UItemBase>(Outer);
	NewItem->InitItem(*Row, Quantity);
	return NewItem;
}
