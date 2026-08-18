//  Nublin Studio 2026 All Rights Reserved.

#include "Data/Inventory/Simulator/InventorySimulator.h"

#include "ActorComponents/SaveLoad/SaveLoadStructs.h"
#include "ActorComponents/ItemCollection.h"
#include "Data/Inventory/InventoryBase.h"
#include "Factory/ItemFactory.h"
#include "Utility/InterfaceUtils.h"

UInventorySimulator::UInventorySimulator()
{
}

void UInventorySimulator::DuplicateInventoryForSimulation(UInventoryBase* InInventory)
{
	SourceInventory = InInventory;

	auto NewInv = SourceInventory->DuplicateInventory(this);
	if (!NewInv)
		return;

	SimulationInventory = NewInv;
	SimulationInventory->SetInventoryContainerID(InInventory->GetInventoryContainerID() + "_Sim");

	SimulationCollection = NewObject<UItemCollection>(this);
	SimulationInventory->SetItemCollectionLink(SimulationCollection);
	
	//SimulationInventory->InitInventory();

	TArray<FItemSaveEntry> SavedData;
	TArray<FString> InventoryFilter;
	InventoryFilter.Add(SourceInventory->GetInventoryContainerID());
	TMap<FString, FString> IDMapping;

	SourceInventory->GetItemCollectionLinked()->SerializeForSave(SavedData, InventoryFilter);
	SimulationCollection->DeserializeFromSave(SavedData, SimulationInventory, IDMapping);
}

void UInventorySimulator::TransferRequestSimulateQuantity(UObject* ItemSample, int32 TotalQuantity)
{
	if (!SimulationInventory || !ItemSample || TotalQuantity <= 0)
		return;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemSample, TEXT("TransferRequestSimulateQuantity")))
		return;

	int32 Remaining = TotalQuantity;

	while (Remaining > 0)
	{
		// Получаем row через интерфейс
		auto ItemRow = IObjectDataProvider::Execute_GetItemRow(ItemSample);

		UObject* Item = UItemFactory::CreateItemByHandle(this, ItemRow, Remaining);
		if (!Item)
			return;

		if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("TransferRequestSimulateQuantity")))
			return;

		const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(Item);
		const int32 MaxStack = Meta.ItemNumeraticData.MaxStackSizeInCharacter;

		const int32 AddAmount = FMath::Min(Remaining, MaxStack);

		IObjectDataProvider::Execute_SetQuantity(Item, AddAmount);

		FItemMoveData MoveData;
		MoveData.SourceItem = Item;
		MoveData.TargetInventory = SimulationInventory;

		TransferRequestSimulate(MoveData);

		Remaining -= AddAmount;
	}
}

void UInventorySimulator::TransferRequestSimulate(FItemMoveData ItemMoveData)
{
	if (!SimulationInventory)
		return;

	if (ItemMoveData.TargetInventory != SimulationInventory)
		return;

	FItemAddResult Result = ItemMoveData.TargetInventory->HandleAddItem(ItemMoveData, false);

	FInventorySimulationOperation Operation;
	Operation.MoveData = ItemMoveData;
	Operation.Result = Result;

	OperationHistory.Add(Operation);
}

bool UInventorySimulator::AreAllOperationsSuccessful() const
{
	if (OperationHistory.Num() == 0)
		return false;
	
	for (const FInventorySimulationOperation& Operation : OperationHistory)
	{
		auto OpResult = Operation.Result.OperationResult;
		if (OpResult == EItemAddResult::IAR_NoItemAdded || OpResult == EItemAddResult::IAR_PartialAmountItemAdded )
		{
			return false;
		}
	}

	return true;
}

bool UInventorySimulator::WasLastOperationSuccessful()
{
	if (OperationHistory.Num() == 0)
		return false;

	auto LastOpResult = OperationHistory.Last().Result.OperationResult;
	
	if (LastOpResult == EItemAddResult::IAR_NoItemAdded || LastOpResult == EItemAddResult::IAR_PartialAmountItemAdded)
	{
		UE_LOG(LogTemp, Warning, TEXT("WasLastOperationSuccessful: FAILED"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("WasLastOperationSuccessful: SUCCESS"));
	return true;
}
