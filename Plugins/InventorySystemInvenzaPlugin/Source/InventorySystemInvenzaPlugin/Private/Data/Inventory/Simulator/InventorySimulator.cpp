//  Nublin Studio 2026 All Rights Reserved.

#include "Data/Inventory/Simulator/InventorySimulator.h"

#include "ActorComponents/SaveLoad/SaveLoadStructs.h"
#include "ActorComponents/ItemCollection.h"
#include "Data/Inventory/InventoryBase.h"
#include "Factory/ItemFactory.h"

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

	SimulationCollection = NewObject<UItemCollection>(this);
	SimulationInventory->SetItemCollectionLink(SimulationCollection);
	SimulationInventory->InitInventory();

	TArray<FItemSaveEntry> SavedData;
	TArray<FString> InventoryFilter;
	InventoryFilter.Add(SourceInventory->GetInventoryContainerID());

	SourceInventory->GetItemCollectionLinked()->SerializeForSave(SavedData, InventoryFilter);
	SimulationCollection->DeserializeFromSave(SavedData, SimulationInventory);
}

void UInventorySimulator::TransferRequestSimulateQuantity(UItemBase* ItemSample, int32 TotalQuantity)
{
	if (!SimulationInventory || !ItemSample ||TotalQuantity <= 0)
		return;
	
	int32 Remaining = TotalQuantity;

	while (Remaining > 0)
	{
		auto ItemClass = ItemSample->GetItemRow();
		UItemBase* Item = UItemFactory::CreateItemByHandle(this, ItemClass, Remaining);
		if (!Item)
			return;

		int32 MaxStack = Item->GetItemRef().ItemNumeraticData.MaxStackSizeInCharacter;
		int32 AddAmount = FMath::Min(Remaining, MaxStack);

		Item->SetQuantity(AddAmount);

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
	UE_LOG(LogTemp, Warning, TEXT("WasLastOperationSuccessful: LastOpResult=%d, Num=%d"),
		   (int32)LastOpResult, OperationHistory.Num());
	
	if (LastOpResult == EItemAddResult::IAR_NoItemAdded || LastOpResult == EItemAddResult::IAR_PartialAmountItemAdded)
	{
		UE_LOG(LogTemp, Warning, TEXT("WasLastOperationSuccessful: FAILED"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("WasLastOperationSuccessful: SUCCESS"));
	return true;
}
