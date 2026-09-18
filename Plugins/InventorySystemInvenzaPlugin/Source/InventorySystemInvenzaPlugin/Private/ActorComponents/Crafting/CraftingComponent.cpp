//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/Crafting/CraftingComponent.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/ItemData.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Factory/ItemFactory.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "Utility/InterfaceUtils.h"
#include "Utility/InvenzayUtility.h"

UCraftingComponent::UCraftingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCraftingComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (InputInventory)
		OnRep_InventoryUpdated();
}

void UCraftingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCraftingComponent, ActiveBlocksReasons);
	DOREPLIFETIME(UCraftingComponent, RecipeQueue);
	DOREPLIFETIME(UCraftingComponent, QueueAdditionalData);
	DOREPLIFETIME(UCraftingComponent, CurrentCraftingRecipe);
	DOREPLIFETIME(UCraftingComponent, InputInventory);
	DOREPLIFETIME(UCraftingComponent, OutputInventory);
	DOREPLIFETIME(UCraftingComponent, FuelInventory);
	DOREPLIFETIME(UCraftingComponent, InteractorInventory);
	DOREPLIFETIME(UCraftingComponent, CachedRecipeResults);
	DOREPLIFETIME(UCraftingComponent, AvailableRecipes);
}

void UCraftingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!GetOwner() || !GetOwner()->HasAuthority()
		|| bCraftMutation || PendingDeliveries.IsEmpty())
	{
		return;
	}

	DeliveryRetryElapsed += DeltaTime;
	if (DeliveryRetryElapsed < 0.25f)
	{
		return;
	}

	DeliveryRetryElapsed = 0.f;

	{
		TGuardValue<bool> MutationGuard(bCraftMutation, true);
		FlushPendingDeliveries();
		HandleRecalculateAvailableRecipes();
	}

	if (PendingDeliveries.IsEmpty()
		&& !GetWorld()->GetTimerManager().IsTimerActive(CraftTimerHandle))
	{
		TryStartNext();
	}
}

void UCraftingComponent::RequestInitCraftingComponent()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		HandleInitCraftingComponent();
	}
	else
	{
		Server_InitCraftingComponent();
	}
}

void UCraftingComponent::Server_InitCraftingComponent_Implementation()
{
	HandleInitCraftingComponent();
}

void UCraftingComponent::HandleInitCraftingComponent()
{
	if (StartingRecipes.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("StartingRecipes is empty on %s."), *GetOwner()->GetName());
		return;
	}
    
	AvailableRecipes.Empty();
    
	for (auto RecipeHandle : StartingRecipes)
	{
		if (!RecipeHandle.DataTable)
			continue;

		const FItemRecipeRow* RecipeRow = RecipeHandle.DataTable->FindRow<FItemRecipeRow>(
		   RecipeHandle.RowName, TEXT("CanCraft"));
		if (!RecipeRow)
			continue;

		AvailableRecipes.Add(*RecipeRow);
	}
	
	RequestRecalculateAvailableRecipes();
	UpdateFuelBlockState();
}

void UCraftingComponent::AddOperator()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	OperatorCount++;
	if (OperatorCount >= 1)
		UpdateOperatorBlockState();
}

void UCraftingComponent::RemoveOperator()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	OperatorCount = FMath::Max(0, OperatorCount - 1);
	if (OperatorCount == 0)
		UpdateOperatorBlockState();
}

bool UCraftingComponent::HasFuelAvailable() const
{
	if (!bRequiresFuel) return true;
	if (!FuelInventory || !FuelInventory->GetItemCollectionLinked()) return false;

	auto Aggregated = FuelInventory->GetItemCollectionLinked()->CollectItemsAggregated(
		FuelInventory->GetInventoryContainerID());

	for (const FItemIDEntry& Entry : Aggregated)
	{
		if (Entry.Amount > 0) return true;
	}
	return false;
}

void UCraftingComponent::SetInventories_Implementation(UInventoryBase* NewInputInventory, UInventoryBase* NewOutputInventory, UInventoryBase* NewFuelInventory)
{
	if (bCraftMutation)
	{
		return;
	}
	
	if (!GetOwner())
		return;

	const bool bChanged =
		InputInventory != NewInputInventory ||
		OutputInventory != NewOutputInventory ||
		FuelInventory != NewFuelInventory;

	if (!bChanged)
		return;

	InputInventory = NewInputInventory;
	OutputInventory = NewOutputInventory;
	FuelInventory = NewFuelInventory;

	OnRep_InventoryUpdated();
}

void UCraftingComponent::SetInputInventory_Implementation(UInventoryBase* NewInputInventory)
{
	if (!GetOwner()) return;
	
	if (bCraftMutation)
	{
		return;
	}

	if (InputInventory != NewInputInventory)
	{
		InputInventory = NewInputInventory;
		OnRep_InventoryUpdated();
	}
}

void UCraftingComponent::SetOutputInventory_Implementation(UInventoryBase* NewOutputInventory)
{
	if (!GetOwner()) return;
	if (bCraftMutation)
	{
		return;
	}

	if (OutputInventory != NewOutputInventory)
	{
		OutputInventory = NewOutputInventory;
		OnRep_InventoryUpdated();
	}
}

void UCraftingComponent::SetFuelInventory_Implementation(UInventoryBase* NewFuelInventory)
{
	if (!GetOwner()) return;
	if (bCraftMutation)
	{
		return;
	}

	if (FuelInventory != NewFuelInventory)
	{
		FuelInventory = NewFuelInventory;
		OnRep_InventoryUpdated();
	}
}

void UCraftingComponent::SetInteractorInventory_Implementation(UInventoryBase* NewInteractorInventory)
{
	if (!GetOwner()) return;
	if (bCraftMutation)
	{
		return;
	}

	if (InteractorInventory != NewInteractorInventory)
	{
		InteractorInventory = NewInteractorInventory;
		OnRep_InventoryUpdated();
	}
}

void UCraftingComponent::RequestRecalculateAvailableRecipes()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		HandleRecalculateAvailableRecipes();
	}
	else
	{
		Server_RecalculateAvailableRecipes();
	}
}

void UCraftingComponent::Server_RecalculateAvailableRecipes_Implementation()
{
	HandleRecalculateAvailableRecipes();
}

void UCraftingComponent::HandleRecalculateAvailableRecipes()
{
	CachedRecipeResults.Empty();
	for (const FItemRecipeRow& Recipe : AvailableRecipes)
	{
		TArray<int32> EmptyOptions;
		FRecipeCheckResult CheckResult = CanCraft(Recipe, EmptyOptions, 1); 
		CachedRecipeResults.Add(FCachedRecipeResult(Recipe.ID, CheckResult));
	}
	
	OnRep_CachedRecipes();
}

void UCraftingComponent::SetBlockStateRequest(const FBlockReasonData& BlockReason, bool bBlocked)
{
	if (!BlockReason.Tag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SetBlockState: Invalid block tag."));
		return;
	}
	
	if (!GetOwner())
		return;
	
	if (GetOwner()->HasAuthority())
		HandleSetBlockState(BlockReason, bBlocked);
	else
	{
		Server_SetBlockState(BlockReason, bBlocked);
	}
}

void UCraftingComponent::Server_SetBlockState_Implementation(const FBlockReasonData& BlockReason, bool bBlocked)
{
	HandleSetBlockState(BlockReason, bBlocked);
}

void UCraftingComponent::HandleSetBlockState(const FBlockReasonData& BlockReason, bool bBlocked)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	if (!BlockReason.Tag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleSetBlockState: Invalid block tag."));
		return;
	}
	
	if (bBlocked)
	{
		ActiveBlocksReasons.AddUnique(BlockReason);
		GetWorld()->GetTimerManager().ClearTimer(CraftTimerHandle);
	}
	else
	{
		ActiveBlocksReasons.Remove(BlockReason);
	}
	
	OnRep_Blocks();
}

void UCraftingComponent::SetNoResourcesRequest(bool bNewValue)
{
	if (!GetOwner())
		return;

	const auto* MySettings = UInvenzaInventorySettingsSubsystem::GetSettingsStatic(this);
	if (!MySettings)
		return;
	
	const FGameplayTag BlockTag = MySettings->Block_NoResources;
	if (!BlockTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SetNoResourcesRequest: Invalid block tag."));
		return;
	}

	if (const FBlockReasonData* BlockReason = MySettings->FindBlockReason(BlockTag))
		SetBlockStateRequest(*BlockReason, bNewValue);
}

bool UCraftingComponent::GetCachedResultForRecipe(FName RecipeID, FRecipeCheckResult& OutResult) const
{
	const FCachedRecipeResult* Found = CachedRecipeResults.FindByPredicate([RecipeID](const FCachedRecipeResult& Item) {
		return Item.RecipeID == RecipeID;
	});

	if (Found)
	{
		OutResult = Found->CheckResult;
		return true;
	}

	return false;
}

FRecipeCheckResult UCraftingComponent::CanCraft(const FItemRecipeRow& RecipeRow, const TArray<int32>& SelectedOptions, int32 Amount) const
{
	TArray<FItemIDEntry> Items;

	for (UInventoryBase* Inventory : GetResourceInventories())
	{
		Items.Append(Inventory->GetItemCollectionLinked()->CollectItemsAggregated(Inventory->GetInventoryContainerID()));
	}

	return CheckRecipe(RecipeRow, Items, SelectedOptions, Amount);
}

FRecipeCheckResult UCraftingComponent::CanCraftWithItems(const FItemRecipeRow& RecipeRow, const TArray<FItemIDEntry>& InventoryItems, int32 Amount)
{
	return CheckRecipe(RecipeRow, InventoryItems, TArray<int32>(), Amount);
}

FRecipeCheckResult UCraftingComponent::CanCraftWithItemsOptions(const FItemRecipeRow& RecipeRow,
                                                                const TArray<FItemIDEntry>& InventoryItems,
                                                                const TArray<int32>& SelectedOptions, int32 Amount)
{
	return CheckRecipe(RecipeRow, InventoryItems, SelectedOptions, Amount);
}

void UCraftingComponent::EnqueueRecipeRequest(FItemRecipeRow ItemRecipeRow, const TArray<int32>& SelectedOptions, int32 Count)
{
	if (!GetOwner()) return;

	if (Count <= 0)
		return;

	if (GetOwner()->HasAuthority())
	{
		HandleEnqueueRecipe(ItemRecipeRow,SelectedOptions, Count);
	}
	else
	{
		Server_EnqueueRecipe(ItemRecipeRow, SelectedOptions, Count);
	}
}

void UCraftingComponent::CancelRecipeRequest(int32 QueueIndex)
{
	if (!GetOwner()) return;

	if (!GetOwner()->HasAuthority())
	{
		Server_CancelRecipe(QueueIndex);
	}
	else
	{
		HandleCancelRecipe(QueueIndex);
	}
}

void UCraftingComponent::RequestMoveQueueItem(FName RecipeID, int32 QueueIndex, bool bMoveUp)
{
	if (!GetOwner()) return;
	if (GetOwner()->HasAuthority())
	{
		Handle_MoveQueueItem(RecipeID, QueueIndex, bMoveUp);
	}
	else
	{
		Server_MoveQueueItem(RecipeID, QueueIndex, bMoveUp);
	}
}

void UCraftingComponent::Server_EnqueueRecipe_Implementation(FItemRecipeRow ItemRecipeRow, const TArray<int32>& SelectedOptions, int32 Count)
{
	HandleEnqueueRecipe(ItemRecipeRow,SelectedOptions, Count);
}

void UCraftingComponent::HandleEnqueueRecipe(FItemRecipeRow ItemRecipeRow, const TArray<int32>& SelectedOptions,
                                             int32 Count)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()
		|| bCraftMutation || Count <= 0
		|| ItemRecipeRow.ID.IsNone())
	{
		return;
	}
	
	{
		TGuardValue<bool> MutationGuard(bCraftMutation, true);

		// The flag becomes true only after successful payment.
		FQueuedRecipe NewItem(ItemRecipeRow, Count, false);

		FQueuedRecipe& Item = RecipeQueue.Items.Add_GetRef(NewItem);
		Item.SortOrder = RecipeQueue.Items.Num() - 1;
		RecipeQueue.MarkItemDirty(Item);

		const int32 DataIndex = QueueAdditionalData.Add(FCraftAdditionalData(Item.ReplicationID, SelectedOptions));
		if (ConsumePolicy == ECraftingResourceConsumePolicy::OnQueueAdd)
		{
			if (!ConsumeResourcesForRecipe(Item, Count, QueueAdditionalData[DataIndex]))
			{
				RecipeQueue.Items.RemoveAt(RecipeQueue.Items.Num() - 1);
				RecipeQueue.MarkArrayDirty();
				QueueAdditionalData.RemoveAt(DataIndex);

				// A failed partial removal may have created refunds.
				FlushPendingDeliveries();
				return;
			}
		}

		HandleRecalculateAvailableRecipes();
		OnRep_Queue();
	}

	TryStartNext();
}

void UCraftingComponent::Server_CancelRecipe_Implementation(int32 QueueIndex)
{
	HandleCancelRecipe(QueueIndex);
}

void UCraftingComponent::HandleCancelRecipe(int32 QueueIndex)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()
		|| bCraftMutation
		|| !RecipeQueue.Items.IsValidIndex(QueueIndex))
	{
		return;
	}

	{
		TGuardValue<bool> MutationGuard(bCraftMutation, true);

		const FQueuedRecipe Item = RecipeQueue.Items[QueueIndex];
		const int32 TargetRepID = Item.ReplicationID;

		const bool bWasCurrent =
			CurrentCraftingRecipe.QueueEntryId == Item.QueueEntryId;

		if (bWasCurrent)
		{
			GetWorld()->GetTimerManager().ClearTimer(CraftTimerHandle);
			CurrentCraftingRecipe = FQueuedRecipe();
		}

		FCraftAdditionalData* AddData =	QueueAdditionalData.FindByPredicate([TargetRepID](const FCraftAdditionalData& Data)
		{
			return Data.TargetRepID == TargetRepID;
		});

		FCraftAdditionalData DummyData;
		RefundResourcesForRecipe(Item, Item.Count, AddData ? *AddData : DummyData);
		RecipeQueue.Items.RemoveAt(QueueIndex);
		RecalculateSortOrders();

		QueueAdditionalData.RemoveAll([TargetRepID](const FCraftAdditionalData& Data)
		{
			return Data.TargetRepID == TargetRepID;
		});

		FlushPendingDeliveries();
		HandleRecalculateAvailableRecipes();

		OnRep_Queue();
		OnRep_CurrentRecipe();
		Multicast_OnCraftCanceled();
	}

	TryStartNext();
}

void UCraftingComponent::ProcessCraftTick()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	if (bCraftMutation)
	{
		return;
	}

	// Do not spend additional fuel/progress while delivery is blocked.
	if (!PendingDeliveries.IsEmpty())
	{
		return;
	}
	
	if (CurrentCraftingRecipe.ItemRecipeRow.ID.IsNone()) return;
	
	if (bRequiresFuel)
	{
		FuelAccumulatedTime += ProcessCraftTickTime;

		while (FuelAccumulatedTime >= SecondsPerFuelUnit)
		{
			if (!ConsumeFuelUnit())
			{
				FuelAccumulatedTime = 0.f;
				UpdateFuelBlockState();
				return;
			}

			FuelAccumulatedTime -= SecondsPerFuelUnit;
		}
	}
    
	auto EffectiveSpeed = CraftingSpeed * ProcessCraftTickTime;
	CurrentCraftingRecipe.CurrentProgress += EffectiveSpeed;

	if (RecipeQueue.Items.Num() > 0	&& RecipeQueue.Items[0].QueueEntryId == CurrentCraftingRecipe.QueueEntryId)
	{
		RecipeQueue.Items[0].CurrentProgress = CurrentCraftingRecipe.CurrentProgress;
		RecipeQueue.MarkItemDirty(RecipeQueue.Items[0]);
	}
	
	OnRep_CurrentRecipe();

	if (CurrentCraftingRecipe.CurrentProgress >= CurrentCraftingRecipe.ItemRecipeRow.CraftVolume)
	{
		FinishCurrentRecipe();
	}
}

void UCraftingComponent::TryStartNext()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()
		|| bCraftMutation
		|| !ActiveBlocksReasons.IsEmpty()
		|| !PendingDeliveries.IsEmpty()
		|| RecipeQueue.Items.IsEmpty()
		|| GetWorld()->GetTimerManager().IsTimerActive(CraftTimerHandle))
	{
		return;
	}

	StartCurrentRecipe(RecipeQueue.Items[0]);
}

void UCraftingComponent::StartCurrentRecipe(FQueuedRecipe& Item)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()
		|| bCraftMutation || Item.Count <= 0
		|| ProcessCraftTickTime <= 0.f)
	{
		return;
	}

	TGuardValue<bool> MutationGuard(bCraftMutation, true);

	FCraftAdditionalData* AddData =	QueueAdditionalData.FindByPredicate(
			[&Item](const FCraftAdditionalData& Data)
			{
				return Data.TargetRepID == Item.ReplicationID;
			});

	if (!AddData)
	{
		return;
	}

	if (ConsumePolicy == ECraftingResourceConsumePolicy::OnCraftStart
		&& !CraftReservations.Contains(Item.QueueEntryId))
	{
		if (!ConsumeResourcesForRecipe(Item, 1, *AddData))
		{
			SetNoResourcesRequest(true);
			return;
		}
	}

	if (ConsumePolicy == ECraftingResourceConsumePolicy::OnQueueAdd
		&& !CraftReservations.Contains(Item.QueueEntryId))
	{
		// Do not manufacture an unpaid queued batch.
		return;
	}

	if (!ActiveBlocksReasons.IsEmpty()
		|| !PendingDeliveries.IsEmpty())
	{
		return;
	}

	CurrentCraftingRecipe = Item;
	GetWorld()->GetTimerManager().SetTimer(
		CraftTimerHandle,
		this,
		&UCraftingComponent::ProcessCraftTick,
		ProcessCraftTickTime,
		true);

	OnRep_CurrentRecipe();
}

void UCraftingComponent::FinishCurrentRecipe()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || bCraftMutation)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(CraftTimerHandle);
	{
		TGuardValue<bool> MutationGuard(bCraftMutation, true);

		const int32 QueueIndex = RecipeQueue.Items.IndexOfByPredicate([this](const FQueuedRecipe& Item)
		{
			return Item.QueueEntryId == CurrentCraftingRecipe.QueueEntryId;
		});

		if (QueueIndex == INDEX_NONE)
		{
			return;
		}

		FQueuedRecipe& Item = RecipeQueue.Items[QueueIndex];
		FCraftAdditionalData* AddData =	QueueAdditionalData.FindByPredicate([&Item](const FCraftAdditionalData& Data)
		{
			return Data.TargetRepID == Item.ReplicationID;
		});

		if (!AddData)
		{
			return;
		}

		TArray<FCraftItemBatch> Output;

		// Prepare every output before committing the paid resources.
		if (!PrepareCraftOutput(Item.ItemRecipeRow, Output))
		{
			UE_LOG(LogTemp, Error,
			       TEXT("Cannot prepare output for recipe %s"),
			       *Item.ItemRecipeRow.ID.ToString());
			return;
		}

		if (!CraftReservations.Contains(Item.QueueEntryId))
		{
			if (ConsumePolicy != ECraftingResourceConsumePolicy::OnCraftFinish || !ConsumeResourcesForRecipe(Item, 1, *AddData))
			{
				SetNoResourcesRequest(true);
				return;
			}
		}

		// Output now belongs to the player/station.
		// Cancelling remaining iterations must not refund this iteration.
		CommitReservedIteration(Item);
		PendingDeliveries.Append(Output);

		const int32 TargetRepID = Item.ReplicationID;
		--Item.Count;
		Item.CurrentProgress = 0.f;

		if (Item.Count <= 0)
		{
			RecipeQueue.Items.RemoveAt(QueueIndex);
			RecalculateSortOrders();

			QueueAdditionalData.RemoveAll(
				[TargetRepID](const FCraftAdditionalData& Data)
				{
					return Data.TargetRepID == TargetRepID;
				});
		}
		else
		{
			RecipeQueue.MarkItemDirty(Item);
		}

		CurrentCraftingRecipe = FQueuedRecipe();

		FlushPendingDeliveries();
		HandleRecalculateAvailableRecipes();

		OnRep_Queue();
		OnRep_CurrentRecipe();
	}

	// A new iteration passes through StartCurrentRecipe again.
	// Unpaid iterations cannot reuse the previous payment.
	TryStartNext();

	//LogQueueState(TEXT("FinishCurrentRecipe"));
}

bool UCraftingComponent::ConsumeFuelUnit()
{
	if (!FuelInventory || !FuelInventory->GetItemCollectionLinked())
		return false;

	TArray<UObject*> FuelItems = FuelInventory->GetItemCollectionLinked()->GetAllItemsByContainer(
		FuelInventory->GetInventoryContainerID());

	for (UObject* Item : FuelItems)
	{
		if (!UInterfaceUtils::ImplementsObjectDataProvider(Item)) continue;
		if (IObjectDataProvider::Execute_GetQuantity(Item) <= 0) continue;

		FuelInventory->HandleRemoveItem(Item, 1);
		return true;
	}

	return false;
}

bool UCraftingComponent::ConsumeResourcesForRecipe(FQueuedRecipe& Item, int32 Count, FCraftAdditionalData& AddData) 
{
	if (Count <= 0)
    {
        return false;
    }

    if (const FCraftReservation* Existing = CraftReservations.Find(Item.QueueEntryId))
    {
        return Existing->PaidIterations >= Count;
    }

    const FRecipeCheckResult Check = CanCraft(Item.ItemRecipeRow, AddData.SelectedOptions, Count);
	if (!Check.bCanCraft)
    {
        return false;
    }

    FCraftReservation Reservation;
    const TArray<UInventoryBase*> Sources = GetResourceInventories();

    auto Rollback = [&]()
    {
        // Keep actual removed resources until they can be returned.
        PendingDeliveries.Append(Reservation.Resources);
        return false;
    };

    for (const FInitItemsEntry& Cost : Check.ResourcesToConsume)
    {
        const FItemData* Row = Cost.Item.DataTable
            ? Cost.Item.DataTable->FindRow<FItemData>(Cost.Item.RowName, TEXT("CraftConsume"))
            : nullptr;

        if (!Row || Cost.Amount <= 0 || Cost.Amount % Count != 0)
        {
            return Rollback();
        }

        FInitItemsEntry UnitCost = Cost;
        UnitCost.Amount /= Count;
        Reservation.UnitCost.Add(UnitCost);

        int32 Remaining = Cost.Amount;

        for (UInventoryBase* Source : Sources)
        {
            const TArray<UObject*> SourceItems = Source->GetItemCollectionLinked()->GetAllItemsByContainer(
                    Source->GetInventoryContainerID());

            TSet<UObject*> Seen;
            for (UObject* SourceItem : SourceItems)
            {
                if (Remaining == 0)
                {
                    break;
                }

                if (!IsValid(SourceItem) || Seen.Contains(SourceItem)
                    || !UInterfaceUtils::ImplementsObjectDataProvider(
                        SourceItem))
                {
                    continue;
                }

                Seen.Add(SourceItem);

                if (IObjectDataProvider::Execute_GetItemID(SourceItem)
                    != Row->ID)
                {
                    continue;
                }

                const int32 Requested = FMath::Min(
                    Remaining,FMath::Max(0,IObjectDataProvider::Execute_GetQuantity(SourceItem)));

                if (Requested == 0)
                {
                    continue;
                }

                UObject* Snapshot = IObjectDataProvider::Execute_DuplicateItem(SourceItem);

                if (!IsValid(Snapshot)
                    || Snapshot == SourceItem
                    || !UInterfaceUtils::ImplementsObjectDataProvider(
                        Snapshot))
                {
                    return Rollback();
                }

                const int32 Before = CountItem(Source, Row->ID);

                Source->HandleRemoveItem(SourceItem, Requested);

                const int32 After = CountItem(Source, Row->ID);

                const int32 Removed = FMath::Clamp(Before - After, 0, Requested);

                if (Removed > 0)
                {
                    IObjectDataProvider::Execute_SetQuantity(Snapshot, Removed);

                    FCraftItemBatch Batch;
                    Batch.Inventory = Source;
                    Batch.Sample = Snapshot;
                    Batch.ItemID = Row->ID;
                    Batch.Amount = Removed;

                    Reservation.Resources.Add(Batch);
                    Remaining -= Removed;
                }

                if (Removed != Requested)
                {
                    return Rollback();
                }
            }

            if (Remaining == 0)
            {
                break;
            }
        }

        if (Remaining != 0)
        {
            return Rollback();
        }
    }

    Reservation.PaidIterations = Count;
    CraftReservations.Add(Item.QueueEntryId, MoveTemp(Reservation));

    Item.bResourcesWasConsumed = true;
    RecipeQueue.MarkItemDirty(Item);

    // The old array no longer represents refundable resources.
    AddData.ConsumedResources.Reset();

    return true;
}

void UCraftingComponent::RefundResourcesForRecipe(const FQueuedRecipe& Item, int32 Count, FCraftAdditionalData& AddData)
{
	if (FCraftReservation* Reservation = CraftReservations.Find(Item.QueueEntryId))
	{
		// Only uncommitted resources remain here.
		PendingDeliveries.Append(Reservation->Resources);
		CraftReservations.Remove(Item.QueueEntryId);
	}

	AddData.ConsumedResources.Reset();
}

TArray<UInventoryBase*> UCraftingComponent::GetResourceInventories() const
{
	TArray<UInventoryBase*> Result;

	if (IsResourceInventory(InputInventory))
	{
		Result.Add(InputInventory);
	}

	if (IsResourceInventory(InteractorInventory))
	{
		Result.AddUnique(InteractorInventory);
	}

	return Result;
}

bool UCraftingComponent::PrepareCraftOutput(const FItemRecipeRow& Recipe, TArray<FCraftItemBatch>& OutBatches)
{
	OutBatches.Reset();

	if (!IsResourceInventory(OutputInventory))
	{
		return false;
	}

	for (const FInitItemsEntry& Output : Recipe.OutputItems)
	{
		if (Output.Amount <= 0 || Output.Item.IsNull())
		{
			OutBatches.Reset();
			return false;
		}

		UObject* Sample = UItemFactory::CreateItemByHandle(this, Output.Item, 1);

		if (!IsValid(Sample)
			|| !UInterfaceUtils::ImplementsObjectDataProvider(Sample))
		{
			OutBatches.Reset();
			return false;
		}

		FCraftItemBatch Batch;
		Batch.Inventory = OutputInventory;
		Batch.Sample = Sample;
		Batch.ItemID = IObjectDataProvider::Execute_GetItemID(Sample);
		Batch.Amount = Output.Amount;

		OutBatches.Add(Batch);
	}

	return true;
}

void UCraftingComponent::FlushPendingDeliveries()
{
	for (FCraftItemBatch& Batch : PendingDeliveries)
	{
		if (!IsResourceInventory(Batch.Inventory)
			|| !IsValid(Batch.Sample))
		{
			continue;
		}

		// Bound the work per retry for large batches.
		for (int32 Attempt = 0;
			 Attempt < 32 && Batch.Amount > 0;
			 ++Attempt)
		{
			const bool bStackable = IObjectDataProvider::Execute_IsStackable(Batch.Sample);
			const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(Batch.Sample);
			const int32 MaxStack = bStackable
				? FMath::Max(1, Meta.ItemNumeraticData.MaxStackSizeInCharacter)
				: 1;

			const int32 Requested = FMath::Min(Batch.Amount, MaxStack);
			UObject* Item = IObjectDataProvider::Execute_DuplicateItem(Batch.Sample);

			if (!IsValid(Item) || Item == Batch.Sample
				|| !UInterfaceUtils::ImplementsObjectDataProvider(Item))
			{
				break;
			}

			IObjectDataProvider::Execute_SetQuantity(Item, Requested);

			FItemMoveData Move;
			Move.SourceItem = Item;
			Move.TargetInventory = Batch.Inventory;

			const FItemAddResult Result = Batch.Inventory->HandleAddItem(Move);
			const int32 Added = FMath::Clamp(Result.ActualAmountAdded, 0, Requested);
			Batch.Amount -= Added;

			if (Added < Requested)
			{
				break;
			}
		}
	}

	PendingDeliveries.RemoveAll(
		[](const FCraftItemBatch& Batch)
		{
			return Batch.Amount <= 0;
		});
}


void UCraftingComponent::CommitReservedIteration(FQueuedRecipe& Item)
{
	FCraftReservation* Reservation = CraftReservations.Find(Item.QueueEntryId);

	check(Reservation && Reservation->PaidIterations > 0);

	for (const FInitItemsEntry& Cost : Reservation->UnitCost)
	{
		const FItemData* Row = Cost.Item.DataTable
			? Cost.Item.DataTable->FindRow<FItemData>(Cost.Item.RowName, TEXT("CraftCommit"))
			: nullptr;

		check(Row);
		int32 Remaining = Cost.Amount;
		for (FCraftItemBatch& Batch : Reservation->Resources)
		{
			if (Batch.ItemID != Row->ID)
			{
				continue;
			}

			const int32 Used = FMath::Min(Remaining, Batch.Amount);
			Batch.Amount -= Used;
			Remaining -= Used;

			if (Remaining == 0)
			{
				break;
			}
		}

		check(Remaining == 0);
	}

	Reservation->Resources.RemoveAll(
		[](const FCraftItemBatch& Batch)
		{
			return Batch.Amount <= 0;
		});

	--Reservation->PaidIterations;
	Item.bResourcesWasConsumed = Reservation->PaidIterations > 0;

	if (Reservation->PaidIterations == 0)
	{
		CraftReservations.Remove(Item.QueueEntryId);
	}

	RecipeQueue.MarkItemDirty(Item);
}

void UCraftingComponent::OnRep_Queue()
{
	/*RecipeQueue.Items.Sort([](const FQueuedRecipe& A, const FQueuedRecipe& B) {
		return A.SortOrder < B.SortOrder;
	});*/
	OnCraftQueueChanged.Broadcast(RecipeQueue.Items);
}

void UCraftingComponent::OnRep_CurrentRecipe()
{
	OnCurrentCraftDataChanged.Broadcast(CurrentCraftingRecipe);
}

void UCraftingComponent::OnRep_InventoryUpdated()
{
	if (InputInventory)
	{
		if (GetOwner() && GetOwner()->HasAuthority())
		{
			HandleRecalculateAvailableRecipes();
		}
	}
	
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		UpdateFuelBlockState();
	}
}

void UCraftingComponent::OnRep_CachedRecipes()
{
	OnAvailableRecipesChanged.Broadcast();
}

void UCraftingComponent::OnRep_AvailableRecipes()
{
	if (bDebugMode)
	{
		UE_LOG(LogTemp, Log, TEXT("Available Recipes replicated to client. Count: %d"), AvailableRecipes.Num());
	}
	
	OnAvailableRecipesChanged.Broadcast();
}

void UCraftingComponent::OnRep_Blocks()
{
	Multicast_OnBlocksUpdated(ActiveBlocksReasons);
	if (ActiveBlocksReasons.IsEmpty())
		TryStartNext();
}

void UCraftingComponent::Multicast_OnBlocksUpdated_Implementation(const TArray<FBlockReasonData>& BlocksActive)
{
	OnBlocksUpdated.Broadcast(BlocksActive);
}

void UCraftingComponent::Multicast_OnCraftStarted_Implementation()
{
	if (OnNewCraftStarted.IsBound())
	{
		OnNewCraftStarted.Broadcast(CurrentCraftingRecipe);
	}
}

void UCraftingComponent::Multicast_OnCraftFinished_Implementation(const FName RecipeID)
{
	OnCraftFinished.Broadcast(RecipeID);
}

void UCraftingComponent::Multicast_OnCraftCanceled_Implementation()
{
	OnCraftCanceled.Broadcast();
}

void UCraftingComponent::Server_MoveQueueItem_Implementation(FName RecipeID, int32 QueueIndex, bool bMoveUp)
{
	Handle_MoveQueueItem(RecipeID, QueueIndex, bMoveUp);
}

void UCraftingComponent::Handle_MoveQueueItem(FName RecipeID, int32 QueueIndex, bool bMoveUp)
{
	if (bCraftMutation)
	{
		return;
	}
	
	if (!RecipeQueue.Items.IsValidIndex(QueueIndex)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Handle_MoveQueueItem: Invalid queue index received: %d"), QueueIndex);
		return;
	}
	
	const int32 NewIndex = bMoveUp ? QueueIndex - 1 : QueueIndex + 1;
	if (!RecipeQueue.Items.IsValidIndex(NewIndex))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Handle_MoveQueueItem: Cannot move recipe '%s' %s. It is already at the %s of the queue."),
			*RecipeID.ToString(),
			bMoveUp ? TEXT("up") : TEXT("down"),
			bMoveUp ? TEXT("top") : TEXT("bottom"));

		return;
	}

	//SaveCurrentProgressToQueue();

	GetWorld()->GetTimerManager().ClearTimer(CraftTimerHandle);
	
	FQueuedRecipe MovingItem = RecipeQueue.Items[QueueIndex];
	RecipeQueue.Items.RemoveAt(QueueIndex);
	RecipeQueue.Items.Insert(MovingItem, NewIndex);
	RecalculateSortOrders();

	OnRep_Queue();

	TryStartNext();
}

void UCraftingComponent::SaveCurrentProgressToQueue()
{
	FQueuedRecipe* QueueRecipe = RecipeQueue.Items.FindByPredicate(
		[this](const FQueuedRecipe& Recipe)
		{
			return Recipe.QueueEntryId == CurrentCraftingRecipe.QueueEntryId;
		});

	if (QueueRecipe)
	{
		QueueRecipe->CurrentProgress = CurrentCraftingRecipe.CurrentProgress;
		RecipeQueue.MarkItemDirty(*QueueRecipe);
	}
}

void UCraftingComponent::LogQueueState(const FString& Context) const
{
	const bool bIsServer = GetOwner() && GetOwner()->HasAuthority();
	const FString Role = bIsServer ? TEXT("SERVER") : TEXT("CLIENT");
	const FColor Color = bIsServer ? FColor::Green : FColor::Cyan;

	FString ItemsStr;
	for (int32 i = 0; i < RecipeQueue.Items.Num(); ++i)
	{
		const FQueuedRecipe& Item = RecipeQueue.Items[i];
		ItemsStr += FString::Printf(TEXT("\n   [%d] ID=%s Count=%d Progress=%.2f Consumed=%d RepID=%d RepKey=%d"),
			i, *Item.ItemRecipeRow.ID.ToString(), Item.Count, Item.CurrentProgress, Item.bResourcesWasConsumed,
			Item.ReplicationID, Item.ReplicationKey);
	}

	const FString Msg = FString::Printf(TEXT("[%s][%s] %s | Num=%d%s"),
		*Role, *Context, *GetOwner()->GetName(), RecipeQueue.Items.Num(), *ItemsStr);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);

	if (GEngine)
	{
		// Уникальный key на каждый Role+Context, чтобы строки не перетирали друг друга,
		// но обновлялись (а не плодились бесконечно) при повторном вызове.
		const int32 OnScreenKey = GetTypeHash(Role + Context);
		GEngine->AddOnScreenDebugMessage(OnScreenKey, 8.f, Color, Msg);
	}
}

void UCraftingComponent::RecalculateSortOrders()
{
	for (int32 i = 0; i < RecipeQueue.Items.Num(); ++i)
	{
		if (RecipeQueue.Items[i].SortOrder != i)
		{
			RecipeQueue.Items[i].SortOrder = i;
			RecipeQueue.MarkItemDirty(RecipeQueue.Items[i]);
		}
	}
	RecipeQueue.MarkArrayDirty();
}

void UCraftingComponent::UpdateOperatorBlockState()
{
	if (bAllowAutomaticCrafting)
		return; 

	const auto* MySettings = UInvenzaInventorySettingsSubsystem::GetSettingsStatic(this);
	if (!MySettings) return;

	const FGameplayTag BlockTag = MySettings->Block_NoOperator; 
	if (!BlockTag.IsValid()) return;

	if (const FBlockReasonData* BlockReason = MySettings->FindBlockReason(BlockTag))
	{
		HandleSetBlockState(*BlockReason, OperatorCount == 0);
	}
}

void UCraftingComponent::UpdateFuelBlockState()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !bRequiresFuel)
		return;

	const auto* MySettings = UInvenzaInventorySettingsSubsystem::GetSettingsStatic(this);
	if (!MySettings) return;

	const FBlockReasonData* BlockReason = MySettings->FindBlockReason(MySettings->Block_NoFuel);
	if (!BlockReason) return;

	HandleSetBlockState(*BlockReason, !HasFuelAvailable());
}

bool UCraftingComponent::IsResourceInventory(UInventoryBase* Inventory)
{
	return IsValid(Inventory) && IsValid(Inventory->GetItemCollectionLinked())
			&& !Inventory->GetInventorySettings().bIsReferenceContainer;
}

int32 UCraftingComponent::CountItem(UInventoryBase* Inventory, FName ItemID)
{
	if (!IsResourceInventory(Inventory))
	{
		return 0;
	}

	int64 Total = 0;
	TSet<UObject*> Seen;

	for (UObject* Item : Inventory->GetItemCollectionLinked()->GetAllItemsByContainer(
			Inventory->GetInventoryContainerID()))
	{
		if (!IsValid(Item) || Seen.Contains(Item)
			|| !UInterfaceUtils::ImplementsObjectDataProvider(Item))
		{
			continue;
		}

		Seen.Add(Item);
		if (IObjectDataProvider::Execute_GetItemID(Item) == ItemID)
		{
			Total += FMath::Max(0, IObjectDataProvider::Execute_GetQuantity(Item));
		}
	}

	return static_cast<int32>(FMath::Min<int64>(Total, MAX_int32));
}

FRecipeCheckResult UCraftingComponent::CheckRecipe(const FItemRecipeRow& Recipe, const TArray<FItemIDEntry>& Items,
                                                   const TArray<int32>& SelectedOptions, int32 Amount)
{
	FRecipeCheckResult Result;
	if (Amount <= 0)
	{
		return Result;
	}

	TMap<FName, int64> Available;

	for (const FItemIDEntry& Item : Items)
	{
		if (!Item.ItemID.IsNone() && Item.Amount > 0)
		{
			Available.FindOrAdd(Item.ItemID) += Item.Amount;
		}
	}

	TArray<TArray<FCraftIngredientOption>> Options;
	bool bInvalidPrimary = false;

	for (const FRecipeItemRequirement& Requirement : Recipe.RequiredItems)
	{
		FRecipeItemRequirementCheck Display;
		TArray<FCraftIngredientOption> RowOptions;

		auto AddOption = [&](const FDataTableRowHandle& Handle, int32 Quantity, FRecipeRequirementResult& DisplayOption)
		{
			FCraftIngredientOption Option;
			Option.Handle = Handle;
			const FItemData* Row = Handle.DataTable ? Handle.DataTable->FindRow<FItemData>(Handle.RowName, TEXT("CraftCheck")): nullptr;
			const int64 Required = static_cast<int64>(Quantity) * Amount;

			if (Row && !Row->ID.IsNone()
				&& Quantity > 0
				&& Required > 0
				&& Required <= MAX_int32)
			{
				Option.bValid = true;
				Option.ItemID = Row->ID;
				Option.Amount = static_cast<int32>(Required);

				DisplayOption.RequiredItemID = Row->ID;
				DisplayOption.ItemMetaData = Row->ItemMetaData;
				DisplayOption.AmountNeed = Option.Amount;
				DisplayOption.AmountHave = static_cast<int32>(FMath::Min<int64>(Available.FindRef(Row->ID), MAX_int32));

				DisplayOption.bIsSatisfied = Available.FindRef(Row->ID) >= Required;
			}

			// Keep indexes stable even for an invalid alternative.
			RowOptions.Add(Option);
		};

		AddOption(Requirement.Item,	Requirement.Quantity,Display.Primary);
		bInvalidPrimary |= !RowOptions[0].bValid;

		for (const FAlternativeItem& Alternative : Requirement.Alternatives)
		{
			FRecipeRequirementResult AlternativeDisplay;
			AddOption(Alternative.Item, Alternative.Quantity,AlternativeDisplay);
			Display.Alternatives.Add(AlternativeDisplay);
		}

		Result.Requirements.Add(Display);
		Options.Add(MoveTemp(RowOptions));
	}

	if (bInvalidPrimary)
	{
		return Result;
	}

	TArray<int32> Chosen;
	Chosen.Init(INDEX_NONE, Options.Num());

	// Reserve quantities while choosing alternatives.
	// Backtracking also handles overlapping alternatives correctly.
	TFunction<bool(int32)> Choose = [&](int32 RequirementIndex)
	{
		if (RequirementIndex == Options.Num())
		{
			return true;
		}

		const auto& Candidates = Options[RequirementIndex];
		auto TryOption = [&](int32 OptionIndex)
		{
			if (!Candidates.IsValidIndex(OptionIndex))
			{
				return false;
			}

			const FCraftIngredientOption& Option = Candidates[OptionIndex];

			if (!Option.bValid
				|| Available.FindRef(Option.ItemID) < Option.Amount)
			{
				return false;
			}

			Available.FindOrAdd(Option.ItemID) -= Option.Amount;
			Chosen[RequirementIndex] = OptionIndex;

			if (Choose(RequirementIndex + 1))
			{
				return true;
			}

			Available.FindOrAdd(Option.ItemID) += Option.Amount;
			Chosen[RequirementIndex] = INDEX_NONE;
			return false;
		};

		if (!SelectedOptions.IsEmpty())
		{
			// Preserve the existing convention:
			// missing selections mean Primary.
			const int32 Selected =
				SelectedOptions.IsValidIndex(RequirementIndex)
					? SelectedOptions[RequirementIndex]
					: 0;

			return TryOption(Selected);
		}

		for (int32 Index = 0; Index < Candidates.Num(); ++Index)
		{
			if (TryOption(Index))
			{
				return true;
			}
		}

		return false;
	};

	Result.bCanCraft = Choose(0);

	if (Result.bCanCraft)
	{
		for (int32 Index = 0; Index < Options.Num(); ++Index)
		{
			const FCraftIngredientOption& Option = Options[Index][Chosen[Index]];

			FInitItemsEntry Entry;
			Entry.Item = Option.Handle;
			Entry.Amount = Option.Amount;

			Result.ResourcesToConsume.Add(Entry);
		}
	}

	return Result;
}

