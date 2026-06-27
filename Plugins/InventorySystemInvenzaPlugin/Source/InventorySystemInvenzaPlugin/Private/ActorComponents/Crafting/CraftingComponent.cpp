//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/Crafting/CraftingComponent.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/ItemData.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "Data/Inventory/InventoryBase.h"
#include "Net/UnrealNetwork.h"
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

	DOREPLIFETIME(UCraftingComponent, BlocksReasons);
	DOREPLIFETIME(UCraftingComponent, RecipeQueue);
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

void UCraftingComponent::SetManualPauseRequest(bool bNewPaused)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	if (bNewPaused)
	{
		BlocksReasons.AddUnique(Block_ManualPause);
		GetWorld()->GetTimerManager().ClearTimer(CraftTimerHandle);
		IsManualPaused = true;
	}
	else
	{
		if (BlocksReasons.Contains(Block_ManualPause))
			BlocksReasons.Remove(Block_ManualPause);
		
		IsManualPaused = false;
		
		if (!CurrentCraftingRecipe.ItemRecipeRow.ID.IsNone())
		{
			GetWorld()->GetTimerManager().SetTimer(
			   CraftTimerHandle, this, &UCraftingComponent::ProcessCraftTick, ProcessCraftTickTime, true
			);
		}
		else
		{
			TryStartNext();
		}
	}
}

void UCraftingComponent::SetNoResourcesRequest(bool bNewValue)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (bNewValue)
	{
		BlocksReasons.AddUnique(Block_NoResources);
		GetWorld()->GetTimerManager().ClearTimer(CraftTimerHandle);
	}
	else
	{
		if (BlocksReasons.Contains(Block_NoResources))
		{
			BlocksReasons.Remove(Block_NoResources);
		}
		
		if (BlocksReasons.Num() == 0)
		{
			if (!CurrentCraftingRecipe.ItemRecipeRow.ID.IsNone())
			{
				GetWorld()->GetTimerManager().SetTimer(
					CraftTimerHandle,
					this,
					&UCraftingComponent::ProcessCraftTick,
					ProcessCraftTickTime,
					true
				);
			}
			else
			{
				TryStartNext();
			}
		}
	}
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
	if (GetOwner()->HasAuthority())
	{
		Handle_MoveQueueItem(RecipeID, QueueIndex, bMoveUp);
	}
	else
	{
		Server_MoveQueueItem(RecipeID, QueueIndex, bMoveUp);
	}
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
	Multicast_OnBlocksUpdated(BlocksReasons);
}

void UCraftingComponent::Server_EnqueueRecipe_Implementation(FItemRecipeRow ItemRecipeRow, const TArray<int32>& SelectedOptions, int32 Count)
{
	HandleEnqueueRecipe(ItemRecipeRow,SelectedOptions, Count);
}

void UCraftingComponent::HandleEnqueueRecipe(FItemRecipeRow ItemRecipeRow, const TArray<int32>& SelectedOptions, int32 Count)
{
	const bool bConsumeOnQueueAdd =	ConsumePolicy == ECraftingResourceConsumePolicy::OnQueueAdd;
	auto NewQueuedRecipe = FQueuedRecipe(ItemRecipeRow, Count, bConsumeOnQueueAdd);
	NewQueuedRecipe.SelectedOptions = SelectedOptions;
	int32 AddedIndex = RecipeQueue.Add(NewQueuedRecipe);

	if (bConsumeOnQueueAdd)
	{
		FQueuedRecipe& QueuedRecipeInArray = RecipeQueue[AddedIndex];
		auto Result = ConsumeResourcesForRecipe(QueuedRecipeInArray, Count);
		if (!Result)
		{
			RecipeQueue.RemoveAt(RecipeQueue.Num() - 1);
			return;
		}
	}
	
	OnRep_Queue();
	
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
	if (!RecipeQueue.IsValidIndex(QueueIndex)) return;

	auto RecipeToDelete = RecipeQueue[QueueIndex];

	if (RecipeToDelete.bResourcesWasConsumed)
		RefundResourcesForRecipe(RecipeToDelete, RecipeToDelete.Count);

	if (QueueIndex == 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(CraftTimerHandle);
		CurrentCraftingRecipe = FQueuedRecipe();
		RecipeQueue.RemoveAt(QueueIndex);

		if (BlocksReasons.IsEmpty())
		{
			TryStartNext();
		}
	}
	else
	{
		RecipeQueue.RemoveAt(QueueIndex);
	}
	
	OnRep_Queue();

	Multicast_OnCraftCanceled();
}

void UCraftingComponent::ProcessCraftTick()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (CurrentCraftingRecipe.ItemRecipeRow.ID.IsNone())
		return;
	
	auto EffectiveSpeed = CraftingSpeed  * ProcessCraftTickTime;
	CurrentCraftingRecipe.CurrentProgress += EffectiveSpeed ;

	if (RecipeQueue.Num() > 0 && RecipeQueue[0].ItemRecipeRow.ID == CurrentCraftingRecipe.ItemRecipeRow.ID)
	{
		RecipeQueue[0].CurrentProgress = CurrentCraftingRecipe.CurrentProgress;
	}

	CurrentRecipeProgressChanged();

	if (CurrentCraftingRecipe.CurrentProgress >= CurrentCraftingRecipe.ItemRecipeRow.CraftVolume)
	{
		FinishCurrentRecipe();
	}
}

void UCraftingComponent::TryStartNext()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	if (!CurrentCraftingRecipe.ItemRecipeRow.ID.IsNone() || CurrentCraftingRecipe.Count > 0)
		return;

	if (RecipeQueue.Num() == 0)
		return;
	
	FQueuedRecipe Next = RecipeQueue[0];

	StartCurrentRecipe(Next);
}

void UCraftingComponent::StartCurrentRecipe(FQueuedRecipe& Item)
{
	if (!GetOwner())
		return;

	const bool bConsumeOnCraftStart = ConsumePolicy == ECraftingResourceConsumePolicy::OnCraftStart;
	if (bConsumeOnCraftStart && Item.bResourcesWasConsumed == false)
	{
		auto Result = ConsumeResourcesForRecipe(Item, 1);
		if (!Result)
		{
			SetNoResourcesRequest(true);
			return;
		}
	}
	
	GetWorld()->GetTimerManager().ClearTimer(CraftTimerHandle);

	CurrentCraftingRecipe = Item;
	
	if (BlocksReasons.IsEmpty())
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

void UCraftingComponent::FinishCurrentRecipe()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	GetWorld()->GetTimerManager().ClearTimer(CraftTimerHandle);

	const bool bConsumeOnCraftFinish = ConsumePolicy == ECraftingResourceConsumePolicy::OnCraftFinish;
	if (bConsumeOnCraftFinish)
	{
		auto Result = ConsumeResourcesForRecipe(CurrentCraftingRecipe, 1);
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
		if (RecipeQueue.Num() > 0)
		{
			RecipeQueue[0].Count = CurrentCraftingRecipe.Count;
			RecipeQueue[0].CurrentProgress = 0.f;
			OnRep_Queue();
		}

		CurrentRecipeProgressChanged();
		if (BlocksReasons.IsEmpty())
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
		if (RecipeQueue.Num() > 0)
		{
			RecipeQueue.RemoveAt(0);
			OnRep_Queue();
		}

		CurrentCraftingRecipe = FQueuedRecipe();
		OnRep_CurrentRecipe();

		TryStartNext();
	}
}

bool UCraftingComponent::ConsumeResourcesForRecipe( FQueuedRecipe& Item, int32 Count) 
{
	auto CheckResult = CanCraft(Item.ItemRecipeRow, Item.SelectedOptions, Count);
	if (!CheckResult.bCanCraft)
		return false;

	for (const FInitItemsEntry& ResourceToConsume : CheckResult.ResourcesToConsume)
	{
		Item.ConsumedResources.Add(ResourceToConsume);
		InputInventory->HandleRemoveItemsByID( ResourceToConsume.Item.RowName, ResourceToConsume.Amount);
	}
	
	Item.bResourcesWasConsumed = true;
	
	OnRep_InventoryUpdated();

	return true;
}

void UCraftingComponent::RefundResourcesForRecipe(const FQueuedRecipe& Item, int32 Count)
{
	if (Item.bResourcesWasConsumed)
	{
		for (auto Element : Item.ConsumedResources)
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
	OnCraftQueueChanged.Broadcast(RecipeQueue);
}

void UCraftingComponent::OnRep_CurrentRecipe()
{
	//OnCurrentRecipeChanged.Broadcast(CurrentCraftingRecipe);
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
	if (!RecipeQueue.IsValidIndex(QueueIndex)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_MoveQueueItem: Invalid queue index received: %d"), QueueIndex);
		return;
	}

	SaveCurrentProgressToQueue();

	GetWorld()->GetTimerManager().ClearTimer(CraftTimerHandle);

	if (bMoveUp)
	{
		RecipeQueue.Swap(QueueIndex, QueueIndex - 1);
		if (QueueIndex == 1) 
		{
			CurrentCraftingRecipe = FQueuedRecipe();
			OnRep_CurrentRecipe();
		}
		
		OnRep_Queue();
		TryStartNext();
	}
	else if (!bMoveUp && QueueIndex < RecipeQueue.Num() - 1)
	{
		RecipeQueue.Swap(QueueIndex, QueueIndex + 1);
		if (QueueIndex == 0)
		{
			CurrentCraftingRecipe = FQueuedRecipe();
			OnRep_CurrentRecipe();
		}
		
		OnRep_Queue();
		TryStartNext();
	}

	CurrentCraftingRecipe = RecipeQueue[0];

	/*for (auto Rec : RecipeQueue)
	{
		UE_LOG(LogTemp, Warning, TEXT("ID is %s RecipeQueue count is %i"), *Rec.ItemRecipeRow.ID.ToString(), Rec.Count);
	}*/
}

void UCraftingComponent::CurrentRecipeProgressChanged()
{
	OnCraftDataChanged.Broadcast(CurrentCraftingRecipe);
}

void UCraftingComponent::SaveCurrentProgressToQueue()
{
	if (!CurrentCraftingRecipe.ItemRecipeRow.ID.IsNone() && RecipeQueue.Num() > 0)
	{
		if (RecipeQueue[0].ItemRecipeRow.ID == CurrentCraftingRecipe.ItemRecipeRow.ID)
		{
			RecipeQueue[0].CurrentProgress = CurrentCraftingRecipe.CurrentProgress;
			//UE_LOG(LogTemp, Log, TEXT("SaveCurrentProgressToQueue: Saved progress (%f) for recipe '%s'"), 
			//	CurrentCraftingRecipe.CurrentProgress, *CurrentCraftingRecipe.ItemRecipeRow.ID.ToString());
		}
	}
}
