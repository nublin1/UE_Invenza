//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/Crafting/CraftingComponent.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Items/itemBase.h"
#include "SaveLoad/InventorySaveTypes.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "Factory/ItemFactory.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

bool UCraftingComponent::CapturePersistence(FInventorySavedCraft& Data, FString& Error) const
{
	if (bCraftMutation)
	{
		Error = TEXT("Crafting transaction is in progress");
		return false;
	}
	Data.ComponentName = GetFName();
	Data.FuelTime = FuelAccumulatedTime;
	Data.Blocks = ActiveBlocksReasons;
	if (!CurrentCraftingRecipe.ItemRecipeRow.ID.IsNone())
	{
		Data.CurrentEntry = CurrentCraftingRecipe.QueueEntryId;
	}
	auto CaptureBatch = [&](const FCraftItemBatch& Batch, FInventorySavedCraftBatch& Saved) -> bool
	{
		auto* Collection = Batch.Inventory ? Batch.Inventory->GetItemCollectionLinked() : nullptr;
		auto* Item = Cast<UItemBase>(Batch.Sample);
		if (!Collection || !Item || Batch.Amount <= 0)
		{
			Error = TEXT("Crafting has an unresolved delivery or reservation");
			return false;
		}
		Saved.OwnerKey = Collection->GetPersistentOwnerKey();
		Saved.InventoryKey = Collection->GetPersistentInventoryKey(Batch.Inventory);
		Saved.Amount = Batch.Amount;
		Saved.Sample.InstanceID = Item->GetPersistentInstanceID();
		Saved.Sample.InstanceClass = FSoftClassPath(Item->GetClass());
		Saved.Sample.Definition = IObjectDataProvider::Execute_GetItemRow(Item);
		Saved.Sample.DefinitionTable = FSoftObjectPath(Saved.Sample.Definition.DataTable);
		Saved.Sample.DefinitionRow = Saved.Sample.Definition.RowName;
		Saved.Sample.Quantity = IObjectDataProvider::Execute_GetQuantity(Item);
		Saved.Sample.SetCustomState(Item->CaptureSaveState());
		return !Saved.InventoryKey.IsEmpty();
	};
	for (const auto& Item : RecipeQueue.Items)
	{
		auto& Saved = Data.Queue.AddDefaulted_GetRef();
		Saved.Recipe = Item;
		if (Item.QueueEntryId == Data.CurrentEntry)
		{
			Saved.Recipe.CurrentProgress = CurrentCraftingRecipe.CurrentProgress;
		}
		if (const auto* Additional = QueueAdditionalData.FindByPredicate(
				[&](const auto& Entry)
				{
					return Entry.TargetRepID == Item.ReplicationID;
				}))
		{
			Saved.SelectedOptions = Additional->SelectedOptions;
		}
		if (const auto* Reservation = CraftReservations.Find(Item.QueueEntryId))
		{
			Saved.PaidIterations = Reservation->PaidIterations;
			Saved.UnitCost = Reservation->UnitCost;
			for (const auto& Batch : Reservation->Resources)
			{
				if (!CaptureBatch(Batch, Saved.Resources.AddDefaulted_GetRef()))
				{
					return false;
				}
			}
		}
	}
	for (const auto& Batch : PendingDeliveries)
	{
		if (!CaptureBatch(Batch, Data.PendingDeliveries.AddDefaulted_GetRef()))
		{
			return false;
		}
	}
	return true;
}

bool UCraftingComponent::PreparePersistence(const FInventorySavedCraft& Data, FInventoryPreparedCraft& Plan,
											FString& Error)
{
	if (!FMath::IsFinite(Data.FuelTime) || Data.FuelTime < 0 || Data.Queue.Num() > 4096)
	{
		Error = TEXT("Invalid crafting progress");
		return false;
	}
	Plan.Component = this;
	Plan.FuelTime = Data.FuelTime;
	Plan.Blocks = Data.Blocks;
	auto RestoreBatch = [&](const FInventorySavedCraftBatch& Saved, FCraftItemBatch& Batch) -> bool
	{
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			TArray<UInventoryCollectionComponent*> Collections;
			It->GetComponents(Collections);
			for (auto* Collection : Collections)
			{
				if (Collection->GetPersistentOwnerKey() == Saved.OwnerKey)
				{
					for (auto* Inventory : Collection->GetActorInventories())
					{
						if (Collection->GetPersistentInventoryKey(Inventory) == Saved.InventoryKey)
						{
							Batch.Inventory = Inventory;
						}
					}
				}
			}
		}
		auto* Item = UItemFactory::CreateItemFromSave(this, Saved.Sample, true);
		if (!Batch.Inventory || !Item || Saved.Amount <= 0 || Saved.Sample.Quantity <= 0)
		{
			Error = TEXT("Crafting resource definition or destination is unavailable; load was not applied");
			return false;
		}
		Plan.CreatedObjects.Emplace(Item);
		Batch.Sample = Item;
		Batch.ItemID = IObjectDataProvider::Execute_GetItemID(Item);
		Batch.Amount = Saved.Amount;
		return true;
	};
	TSet<FGuid> QueueIDs;
	for (const auto& Saved : Data.Queue)
	{
		if (!Saved.Recipe.QueueEntryId.IsValid() || QueueIDs.Contains(Saved.Recipe.QueueEntryId) ||
			Saved.Recipe.ItemRecipeRow.ID.IsNone() || Saved.Recipe.Count <= 0 ||
			!FMath::IsFinite(Saved.Recipe.CurrentProgress) || Saved.Recipe.CurrentProgress < 0 ||
			Saved.PaidIterations < 0 || Saved.PaidIterations > Saved.Recipe.Count)
		{
			Error = TEXT("Invalid crafting queue entry");
			return false;
		}
		QueueIDs.Add(Saved.Recipe.QueueEntryId);
		auto& Entry = Plan.Queue.Add_GetRef(Saved.Recipe);
		Entry.ReplicationID = INDEX_NONE;
		Entry.ReplicationKey = INDEX_NONE;
		Entry.MostRecentArrayReplicationKey = INDEX_NONE;
		Entry.bResourcesWasConsumed = Saved.PaidIterations > 0;
		Plan.AdditionalData.Emplace(INDEX_NONE, Saved.SelectedOptions);
		if (Saved.PaidIterations > 0)
		{
			auto& Reservation = Plan.Reservations.Add(Entry.QueueEntryId);
			Reservation.PaidIterations = Saved.PaidIterations;
			Reservation.UnitCost = Saved.UnitCost;
			for (const auto& Batch : Saved.Resources)
			{
				if (!RestoreBatch(Batch, Reservation.Resources.AddDefaulted_GetRef()))
				{
					return false;
				}
			}
		}
		if (Entry.QueueEntryId == Data.CurrentEntry)
		{
			Plan.Current = Entry;
		}
	}
	if (Data.CurrentEntry.IsValid() && !QueueIDs.Contains(Data.CurrentEntry))
	{
		Error = TEXT("Current craft is not in the queue");
		return false;
	}
	for (const auto& Batch : Data.PendingDeliveries)
	{
		if (!RestoreBatch(Batch, Plan.Deliveries.AddDefaulted_GetRef()))
		{
			return false;
		}
	}
	return true;
}

void UCraftingComponent::CommitPersistence(FInventoryPreparedCraft& Plan)
{
	GetWorld()->GetTimerManager().ClearTimer(CraftTimerHandle);
	RecipeQueue.Items = MoveTemp(Plan.Queue);
	QueueAdditionalData = MoveTemp(Plan.AdditionalData);
	CraftReservations = MoveTemp(Plan.Reservations);
	PendingDeliveries = MoveTemp(Plan.Deliveries);
	ActiveBlocksReasons = Plan.Blocks;
	FuelAccumulatedTime = Plan.FuelTime;
	CurrentCraftingRecipe = FQueuedRecipe();
	OperatorCount = 0;
	InteractorInventory = nullptr;
	for (int32 Index = 0; Index < RecipeQueue.Items.Num(); ++Index)
	{
		auto& Entry = RecipeQueue.Items[Index];
		RecipeQueue.MarkItemDirty(Entry);
		QueueAdditionalData[Index].TargetRepID = Entry.ReplicationID;
		if (Entry.QueueEntryId == Plan.Current.QueueEntryId)
		{
			CurrentCraftingRecipe = Entry;
		}
	}
	RecipeQueue.MarkArrayDirty();
}

void UCraftingComponent::ResumeAfterPersistence()
{
	if (!GetOwner()->HasAuthority() || GetWorld()->bIsTearingDown)
	{
		return;
	}
	OnRep_Queue();
	OnRep_CurrentRecipe();
	UpdateOperatorBlockState();
	UpdateFuelBlockState();
	HandleRecalculateAvailableRecipes();
	OnRep_Blocks();
	TryStartNext();
}
