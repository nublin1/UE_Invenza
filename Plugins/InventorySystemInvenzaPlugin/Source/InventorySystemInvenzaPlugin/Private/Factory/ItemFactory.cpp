//  Nublin Studio 2025 All Rights Reserved.

#include "Factory/ItemFactory.h"
#include "SaveLoad/InventorySaveTypes.h"

#include "Data/ItemData.h"
#include "Data/Items/itemBase.h"

UItemBase* UItemFactory::CreateItemByHandle(UObject* Outer, FDataTableRowHandle Handle, int32 Quantity)
{
	if (!Outer)
	{
		return nullptr;
	}

	if (!Handle.DataTable || Handle.RowName.IsNone())
	{
		return nullptr;
	}

	FItemData* Row = Handle.GetRow<FItemData>(TEXT("CreateItemByHandle"));
	if (!Row)
	{
		return nullptr;
	}

	UItemBase* NewItem = NewObject<UItemBase>(Outer);
	NewItem->SetItemRow(Handle);
	NewItem->InitItem(Handle.RowName, *Row, Quantity);
	return NewItem;
}
UItemBase* UItemFactory::CreateItemFromSave(UObject* Outer, const FInventorySavedItem& Saved, bool bBatchSample)
{
	if (!Outer || Saved.Quantity <= 0)
	{
		return nullptr;
	}
	auto Definition = Saved.Definition;
	if (!Definition.DataTable && !Saved.DefinitionTable.IsNull())
	{
		Definition.DataTable = Cast<UDataTable>(Saved.DefinitionTable.TryLoad());
	}
	if (Definition.RowName.IsNone())
	{
		Definition.RowName = Saved.DefinitionRow;
	}
	const auto* Row =
		Definition.DataTable
			? Definition.DataTable->FindRow<FItemData>(Definition.RowName, TEXT("Restore inventory item"), false)
			: nullptr;
	if (!Row || (!bBatchSample && Saved.Quantity > Row->ItemMetaData.ItemNumeraticData.MaxStackSizeInCharacter))
	{
		return nullptr;
	}
	UClass* Class =
		Saved.InstanceClass.IsNull() ? UItemBase::StaticClass() : Saved.InstanceClass.TryLoadClass<UItemBase>();
	if (!Class || Class->HasAnyClassFlags(CLASS_Abstract))
	{
		return nullptr;
	}
	auto* Item = NewObject<UItemBase>(Outer, Class);
	Item->SetItemRow(Definition);
	Item->InitItem(Definition.RowName, *Row, Saved.Quantity);
	if (bBatchSample)
	{
		IObjectDataProvider::Execute_SetQuantity(Item, Saved.Quantity);
	}
	Item->RestoreSaveState(Saved.GetCustomState());
	Item->SetPersistentInstanceID(Saved.InstanceID.IsValid() ? Saved.InstanceID : FGuid::NewGuid());
	return IObjectDataProvider::Execute_GetQuantity(Item) == Saved.Quantity ? Item : nullptr;
}
