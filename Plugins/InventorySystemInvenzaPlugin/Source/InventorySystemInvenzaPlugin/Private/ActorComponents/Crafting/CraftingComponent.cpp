//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/Crafting/CraftingComponent.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/ItemData.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "Utility/InventoryUtility.h"

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
	DOREPLIFETIME(UCraftingComponent, CachedRecipeResults);
	DOREPLIFETIME(UCraftingComponent, AvailableRecipes);
}

void UCraftingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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
	if (Config.StartingRecipes.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("StartingRecipes is empty on %s."), *GetOwner()->GetName());
		return;
	}
    
	AvailableRecipes.Empty();
    
	for (auto RecipeHandle : Config.StartingRecipes)
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
}

void UCraftingComponent::SetInputInventory_Implementation(UInventoryBase* NewInputInventory)
{
	if (!GetOwner()) return;

	if (InputInventory != NewInputInventory)
	{
		InputInventory = NewInputInventory;
		OnRep_InventoryUpdated();
	}
}

void UCraftingComponent::SetOutputInventory_Implementation(UInventoryBase* NewOutputInventory)
{
	if (!GetOwner()) return;

	if (OutputInventory != NewOutputInventory)
	{
		OutputInventory = NewOutputInventory;
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
	FRecipeCheckResult Result;

	if (InputInventory == nullptr)
		return Result;

	auto InvItems = InputInventory->GetItemCollectionLinked()->CollectItemsAggregated(InputInventory->GetInventoryContainerID());

	if (SelectedOptions.IsEmpty())
		return CanCraftWithItems(RecipeRow, InvItems, Amount);
	
	return CanCraftWithItemsOptions(RecipeRow, InvItems, SelectedOptions, Amount);
}

FRecipeCheckResult UCraftingComponent::CanCraftWithItems(const FItemRecipeRow& RecipeRow,
                                                         const TArray<FItemIDEntry>& InventoryItems, int32 Amount)
{
	FRecipeCheckResult Result;

	auto GetItemAmountByID = [&InventoryItems](const FName& ItemID) -> int32
	{
		const FItemIDEntry* Item = InventoryItems.FindByPredicate(
			[&](const FItemIDEntry& I) { return I.ItemID.IsValid() && I.ItemID == ItemID; }
		);
		return Item ? Item->Amount : 0;
	};

	auto HasItemWithQuantity = [&InventoryItems](const FName& ItemID, int32 Quantity)
	{
		const FItemIDEntry* Item = InventoryItems.FindByPredicate(
			[&](const FItemIDEntry& I) { return I.ItemID == ItemID; }
		);
		return Item && Item->Amount >= Quantity;
	};

	for (const FRecipeItemRequirement& Req : RecipeRow.RequiredItems)
	{
		if (!Req.Item.DataTable) continue;

		FRecipeItemRequirementCheck ReqCheck;

		// Primary
		const FItemData* ItemRow = Req.Item.DataTable->FindRow<FItemData>(Req.Item.RowName, TEXT("Context_MainItem"));
		if (!ItemRow)
		{
			UE_LOG(LogTemp, Warning, TEXT("Main recipe item %s not found in DataTable!"), *Req.Item.RowName.ToString());
			continue;
		}

		ReqCheck.Primary.RequiredItemID = ItemRow->ID;
		ReqCheck.Primary.ItemMetaData = ItemRow->ItemMetaData;
		ReqCheck.Primary.AmountNeed = Req.Quantity * Amount;
		ReqCheck.Primary.AmountHave = GetItemAmountByID(ItemRow->ID);
		ReqCheck.Primary.bIsSatisfied = HasItemWithQuantity(ItemRow->ID, ReqCheck.Primary.AmountNeed);

		if (ReqCheck.Primary.bIsSatisfied)
		{
			FInitItemsEntry ConsumeEntry;
			ConsumeEntry.Item = Req.Item;
			ConsumeEntry.Amount = ReqCheck.Primary.AmountNeed;
			Result.ResourcesToConsume.Add(ConsumeEntry);
		}

		bool bFoundAlternativeSatisfy = false;
		FInitItemsEntry SelectedAlternative;

		// Alternatives
		for (const FAlternativeItem& Alt : Req.Alternatives)
		{
			if (!Alt.Item.DataTable) continue;

			const FItemData* AltItemRow = Alt.Item.DataTable->FindRow<FItemData>(
				Alt.Item.RowName, TEXT("Context_AltItem"));
			if (!AltItemRow)
			{
				UE_LOG(LogTemp, Warning, TEXT("Alternative recipe item %s not found in DataTable!"),
				       *Alt.Item.RowName.ToString());
				continue;
			}

			FRecipeRequirementResult AltCheck;
			AltCheck.RequiredItemID = AltItemRow->ID;
			AltCheck.ItemMetaData = AltItemRow->ItemMetaData;
			AltCheck.AmountNeed = Alt.Quantity * Amount;
			AltCheck.AmountHave = GetItemAmountByID(AltItemRow->ID);
			AltCheck.bIsSatisfied = HasItemWithQuantity(AltItemRow->ID, AltCheck.AmountNeed);

			if (!ReqCheck.Primary.bIsSatisfied && AltCheck.bIsSatisfied && !bFoundAlternativeSatisfy)
			{
				bFoundAlternativeSatisfy = true;
				SelectedAlternative.Item = Alt.Item;
				SelectedAlternative.Amount = AltCheck.AmountNeed;
			}

			ReqCheck.Alternatives.Add(AltCheck);
		}

		if (!ReqCheck.Primary.bIsSatisfied && bFoundAlternativeSatisfy)
		{
			Result.ResourcesToConsume.Add(SelectedAlternative);
		}
		
		Result.Requirements.Add(ReqCheck);
	}

	Result.bCanCraft = Result.Requirements.FindByPredicate([](const FRecipeItemRequirementCheck& R)
	{
	   if (R.Primary.bIsSatisfied) return false;
       
	   return R.Alternatives.FindByPredicate([](const FRecipeRequirementResult& Alt) { return Alt.bIsSatisfied; }) == nullptr;
	}) == nullptr;

	if (!Result.bCanCraft)
	{
		Result.ResourcesToConsume.Empty();
	}

	return Result;
}

FRecipeCheckResult UCraftingComponent::CanCraftWithItemsOptions(const FItemRecipeRow& RecipeRow,
                                                                const TArray<FItemIDEntry>& InventoryItems,
                                                                const TArray<int32>& SelectedOptions, int32 Amount)
{
	FRecipeCheckResult Result;
	Result.bCanCraft = true;

	auto GetItemAmountByID = [&InventoryItems](const FName& ItemID) -> int32
	{
		const FItemIDEntry* Item = InventoryItems.FindByPredicate(
			[&](const FItemIDEntry& I) { return I.ItemID.IsValid() && I.ItemID == ItemID; }
		);
		return Item ? Item->Amount : 0;
	};

	auto HasItemWithQuantity = [&InventoryItems](const FName& ItemID, int32 Quantity)
	{
		const FItemIDEntry* Item = InventoryItems.FindByPredicate(
			[&](const FItemIDEntry& I) { return I.ItemID == ItemID; }
		);
		return Item && Item->Amount >= Quantity;
	};

	for (int32 ReqIndex = 0; ReqIndex < RecipeRow.RequiredItems.Num(); ++ReqIndex)
	{
		const FRecipeItemRequirement& Req = RecipeRow.RequiredItems[ReqIndex];
		if (!Req.Item.DataTable) continue;

		FRecipeItemRequirementCheck ReqCheck;

		const FItemData* ItemRow = Req.Item.DataTable->FindRow<FItemData>(Req.Item.RowName, TEXT("Context_MainItem"));
		if (!ItemRow)
		{
			UE_LOG(LogTemp, Warning, TEXT("Main recipe item %s not found in DataTable!"), *Req.Item.RowName.ToString());
			continue;
		}

		ReqCheck.Primary.RequiredItemID = ItemRow->ID;
		ReqCheck.Primary.ItemMetaData = ItemRow->ItemMetaData;
		ReqCheck.Primary.AmountNeed = Req.Quantity * Amount;
		ReqCheck.Primary.AmountHave = GetItemAmountByID(ItemRow->ID);
		ReqCheck.Primary.bIsSatisfied = HasItemWithQuantity(ItemRow->ID, ReqCheck.Primary.AmountNeed);

		for (const FAlternativeItem& Alt : Req.Alternatives)
		{
			if (!Alt.Item.DataTable) continue;

			const FItemData* AltItemRow = Alt.Item.DataTable->FindRow<FItemData>(
				Alt.Item.RowName, TEXT("Context_AltItem"));
			if (!AltItemRow) continue;

			FRecipeRequirementResult AltCheck;
			AltCheck.RequiredItemID = AltItemRow->ID;
			AltCheck.ItemMetaData = AltItemRow->ItemMetaData;
			AltCheck.AmountNeed = Alt.Quantity * Amount;
			AltCheck.AmountHave = GetItemAmountByID(AltItemRow->ID);
			AltCheck.bIsSatisfied = HasItemWithQuantity(AltItemRow->ID, AltCheck.AmountNeed);

			ReqCheck.Alternatives.Add(AltCheck);
		}

		int32 ChosenOption = SelectedOptions.IsValidIndex(ReqIndex) ? SelectedOptions[ReqIndex] : 0;
		bool bCurrentSlotSatisfied = false;

		if (ChosenOption == 0)
		{
			bCurrentSlotSatisfied = ReqCheck.Primary.bIsSatisfied;
			if (bCurrentSlotSatisfied)
			{
				FInitItemsEntry ConsumeEntry;
				ConsumeEntry.Item = Req.Item;
				ConsumeEntry.Amount = ReqCheck.Primary.AmountNeed;
				Result.ResourcesToConsume.Add(ConsumeEntry);
			}
		}
		else
		{
			int32 AltIndex = ChosenOption - 1;
			if (ReqCheck.Alternatives.IsValidIndex(AltIndex) && Req.Alternatives.IsValidIndex(AltIndex))
			{
				bCurrentSlotSatisfied = ReqCheck.Alternatives[AltIndex].bIsSatisfied;
				if (bCurrentSlotSatisfied)
				{
					FInitItemsEntry ConsumeEntry;
					ConsumeEntry.Item = Req.Alternatives[AltIndex].Item;
					ConsumeEntry.Amount = ReqCheck.Alternatives[AltIndex].AmountNeed;
					Result.ResourcesToConsume.Add(ConsumeEntry);
				}
			}
		}

		if (!bCurrentSlotSatisfied)
		{
			Result.bCanCraft = false;
		}

		Result.Requirements.Add(ReqCheck);
	}


	if (!Result.bCanCraft)
	{
		Result.ResourcesToConsume.Empty();
	}

	return Result;
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

void UCraftingComponent::HandleEnqueueRecipe(FItemRecipeRow ItemRecipeRow, const TArray<int32>& SelectedOptions, int32 Count)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	
	//LogQueueState(TEXT("BEFORE_Enqueue"));

	const bool bConsumeOnQueueAdd = ConsumePolicy == ECraftingResourceConsumePolicy::OnQueueAdd;
	FQueuedRecipe NewQueuedRecipe = FQueuedRecipe(ItemRecipeRow, Count, bConsumeOnQueueAdd);
	
	FQueuedRecipe& QueuedRecipeInArray = RecipeQueue.Items.Add_GetRef(NewQueuedRecipe);
	QueuedRecipeInArray.SortOrder = RecipeQueue.Items.Num() - 1;
	RecipeQueue.MarkItemDirty(QueuedRecipeInArray);
	
	FCraftAdditionalData AddData(QueuedRecipeInArray.ReplicationID, SelectedOptions);
	int32 DataIndex = QueueAdditionalData.Add(AddData);

	if (bConsumeOnQueueAdd)
	{
		auto Result = ConsumeResourcesForRecipe(QueuedRecipeInArray, Count, QueueAdditionalData[DataIndex]);
		if (!Result)
		{
			RecipeQueue.Items.RemoveAt(RecipeQueue.Items.Num() - 1);
			RecipeQueue.MarkArrayDirty();
			QueueAdditionalData.RemoveAt(DataIndex);
			return;
		}
	}
    
	OnRep_Queue();
	//LogQueueState(TEXT("AFTER_Enqueue")); 
	if (CurrentCraftingRecipe.ItemRecipeRow.ID.IsNone() || CurrentCraftingRecipe.Count == 0)
	{
		TryStartNext();
	}
}

void UCraftingComponent::Server_CancelRecipe_Implementation(int32 QueueIndex)
{
	HandleCancelRecipe(QueueIndex);
}

void UCraftingComponent::HandleCancelRecipe(int32 QueueIndex)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	if (!RecipeQueue.Items.IsValidIndex(QueueIndex)) return;

	auto RecipeToDelete = RecipeQueue.Items[QueueIndex];
	const int32 TargetID = RecipeToDelete.ReplicationID;

	if (RecipeToDelete.bResourcesWasConsumed)
	{
		FCraftAdditionalData* FoundData = QueueAdditionalData.FindByPredicate([TargetID](const FCraftAdditionalData& Data) {
		   return Data.TargetRepID == TargetID;
	   });

		if (FoundData)
		{
			RefundResourcesForRecipe(RecipeToDelete, RecipeToDelete.Count, *FoundData);
		}
	}

	if (QueueIndex == 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(CraftTimerHandle);
		CurrentCraftingRecipe = FQueuedRecipe();
       
		RecipeQueue.Items.RemoveAt(QueueIndex);
		RecalculateSortOrders();
		
		QueueAdditionalData.RemoveAll([TargetID](const FCraftAdditionalData& Data) {
		   return Data.TargetRepID == TargetID;
	   });

		if (ActiveBlocksReasons.IsEmpty())
		{
			TryStartNext();
		}
	}
	else
	{
		RecipeQueue.Items.RemoveAt(QueueIndex);
		RecalculateSortOrders();
		
		QueueAdditionalData.RemoveAll([TargetID](const FCraftAdditionalData& Data) {
		   return Data.TargetRepID == TargetID;
	   });
	}
    
	OnRep_Queue();
	OnRep_CurrentRecipe();
	Multicast_OnCraftCanceled();
}

void UCraftingComponent::ProcessCraftTick()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	if (CurrentCraftingRecipe.ItemRecipeRow.ID.IsNone()) return;
    
	auto EffectiveSpeed = CraftingSpeed * ProcessCraftTickTime;
	CurrentCraftingRecipe.CurrentProgress += EffectiveSpeed;

	if (RecipeQueue.Items.Num() > 0 && RecipeQueue.Items[0].ItemRecipeRow.ID == CurrentCraftingRecipe.ItemRecipeRow.ID)
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
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	if (!ActiveBlocksReasons.IsEmpty())
		return;
	
	//if (!CurrentCraftingRecipe.ItemRecipeRow.ID.IsNone() || CurrentCraftingRecipe.Count > 0)
	//	return;

	if (RecipeQueue.Items.Num() == 0)
		return;
	
	StartCurrentRecipe(RecipeQueue.Items[0]);
}

void UCraftingComponent::StartCurrentRecipe(FQueuedRecipe& Item)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	const bool bConsumeOnCraftStart = ConsumePolicy == ECraftingResourceConsumePolicy::OnCraftStart;
	if (bConsumeOnCraftStart && Item.bResourcesWasConsumed == false)
	{
		const int32 TargetID = Item.ReplicationID;
		FCraftAdditionalData* FoundData = QueueAdditionalData.FindByPredicate([TargetID](const FCraftAdditionalData& Data) {
			return Data.TargetRepID == TargetID;
		});
		
		FCraftAdditionalData DummyData;
		auto Result = ConsumeResourcesForRecipe(Item, 1, FoundData ? *FoundData : DummyData);
		if (!Result)
		{
			SetNoResourcesRequest(true);
			return;
		}
	}
	
	GetWorld()->GetTimerManager().ClearTimer(CraftTimerHandle);

	CurrentCraftingRecipe = Item;
	
	UE_LOG(LogTemp, Warning,TEXT("SERVER CurrentRecipe Progress = %f"),	CurrentCraftingRecipe.CurrentProgress);
	
	GetWorld()->GetTimerManager().SetTimer(
		CraftTimerHandle,
		this,
		&UCraftingComponent::ProcessCraftTick,
		ProcessCraftTickTime,
		true
		);
	
	OnRep_CurrentRecipe();
}

void UCraftingComponent::FinishCurrentRecipe()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	GetWorld()->GetTimerManager().ClearTimer(CraftTimerHandle);
	
	int32 TargetID = CurrentCraftingRecipe.ReplicationID;
	FCraftAdditionalData* FoundData = QueueAdditionalData.FindByPredicate([TargetID](const FCraftAdditionalData& Data) {
		return Data.TargetRepID == TargetID;
	});

	const bool bConsumeOnCraftFinish = ConsumePolicy == ECraftingResourceConsumePolicy::OnCraftFinish;
	if (bConsumeOnCraftFinish)
	{
		FCraftAdditionalData DummyData;
		auto Result = ConsumeResourcesForRecipe(CurrentCraftingRecipe, 1, FoundData ? *FoundData : DummyData);
		if (!Result)
		{
			CurrentCraftingRecipe.CurrentProgress = CurrentCraftingRecipe.ItemRecipeRow.CraftVolume - 0.01f;
			SaveCurrentProgressToQueue();
			SetNoResourcesRequest(true);
			return;
		}
	}

	GiveCraftedItemToInventory(CurrentCraftingRecipe.ItemRecipeRow);
	
	if (CurrentCraftingRecipe.Count > 1)
	{
		CurrentCraftingRecipe.Count--;
		CurrentCraftingRecipe.CurrentProgress = 0.f;
		if (RecipeQueue.Items.Num() > 0 && RecipeQueue.Items[0].ItemRecipeRow.ID == CurrentCraftingRecipe.ItemRecipeRow.ID)
		{
			RecipeQueue.Items[0].Count = CurrentCraftingRecipe.Count;
			RecipeQueue.Items[0].CurrentProgress = 0.f;
			RecipeQueue.MarkItemDirty(RecipeQueue.Items[0]); 
			
		}

		OnRep_Queue();
		OnRep_CurrentRecipe();
		
		if (ActiveBlocksReasons.IsEmpty())
		{
			GetWorld()->GetTimerManager().SetTimer(
			CraftTimerHandle,
			this,
			&UCraftingComponent::ProcessCraftTick,
			ProcessCraftTickTime,
			true);
		}

		OnRep_CurrentRecipe();
	}
	else
	{
		if (RecipeQueue.Items.Num() > 0)
		{
			RecipeQueue.Items.RemoveAt(0);
			RecalculateSortOrders();
		}
		
		QueueAdditionalData.RemoveAll([TargetID](const FCraftAdditionalData& Data) {
		   return Data.TargetRepID == TargetID;
	   });

		CurrentCraftingRecipe = FQueuedRecipe();
		OnRep_Queue();
		OnRep_CurrentRecipe();

		TryStartNext();
	}
	
	//LogQueueState(TEXT("FinishCurrentRecipe"));
}

bool UCraftingComponent::ConsumeResourcesForRecipe(FQueuedRecipe& Item, int32 Count, FCraftAdditionalData& AddData) 
{
	auto CheckResult = CanCraft(Item.ItemRecipeRow, AddData.SelectedOptions, Count);
	if (!CheckResult.bCanCraft)
		return false;

	for (const FInitItemsEntry& ResourceToConsume : CheckResult.ResourcesToConsume)
	{
		AddData.ConsumedResources.Add(ResourceToConsume);
		InputInventory->HandleRemoveItemsByID( ResourceToConsume.Item.RowName, ResourceToConsume.Amount);
	}
	
	Item.bResourcesWasConsumed = true;
	RecipeQueue.MarkItemDirty(Item);
	
	OnRep_InventoryUpdated();

	return true;
}

void UCraftingComponent::RefundResourcesForRecipe(const FQueuedRecipe& Item, int32 Count, FCraftAdditionalData& AddData)
{
	if (Item.bResourcesWasConsumed)
	{
		for (auto Element : AddData.ConsumedResources)
		{
			UInventoryUtility::AddItemQuantity(this, InputInventory, Element);
		}
		
		OnRep_InventoryUpdated();
	}
}

void UCraftingComponent::GiveCraftedItemToInventory(FItemRecipeRow CraftedRow)
{
	for (auto Element : CraftedRow.OutputItems)
	{
		UInventoryUtility::AddItemQuantity(this, OutputInventory, Element);
	}
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
		RequestRecalculateAvailableRecipes();
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
