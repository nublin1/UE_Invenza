//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/Crafting/CraftingComponent.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/ItemData.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "Data/Inventory/InventoryBase.h"
#include "Net/UnrealNetwork.h"

UCraftingComponent::UCraftingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCraftingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCraftingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCraftingComponent, RecipeQueue);
	DOREPLIFETIME(UCraftingComponent, CurrentCraftingRecipe);
	DOREPLIFETIME(UCraftingComponent, InputInventory);
	DOREPLIFETIME(UCraftingComponent, CachedRecipeResults);
	DOREPLIFETIME(UCraftingComponent, AvailableRecipes);
}

void UCraftingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCraftingComponent::InitCraftingComponent()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

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
	
	RecalculateAvailableRecipes();
}

void UCraftingComponent::SetInputInventory(UInventoryBase* NewInputInventory)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	if (InputInventory != NewInputInventory)
	{
		InputInventory = NewInputInventory;
		OnRep_InputInventory();
	}
}

void UCraftingComponent::RecalculateAvailableRecipes()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	CachedRecipeResults.Empty();
	for (const FItemRecipeRow& Recipe : AvailableRecipes)
	{
		FRecipeCheckResult CheckResult = CanCraft(Recipe, 1); 
		CachedRecipeResults.Add(FCachedRecipeResult(Recipe.ID, CheckResult));
	}
	
	OnRep_CachedRecipes();
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

FRecipeCheckResult UCraftingComponent::CanCraft(const FItemRecipeRow& RecipeRow, int32 Amount) const
{
	FRecipeCheckResult Result;

	if (InputInventory == nullptr)
		return Result;

	auto InvItems = InputInventory->GetItemCollectionLinked()->CollectItemsAggregated(InputInventory->GetInventoryContainerID());

	return CanCraftWithItems(RecipeRow, InvItems, Amount);
}

FRecipeCheckResult UCraftingComponent::CanCraftWithItems(const FItemRecipeRow& RecipeRow , const TArray<FItemIDEntry>& InventoryItems, int32 Amount)
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
    	
       const FItemData* ItemRow = Req.Item.DataTable->FindRow<FItemData>(Req.Item.RowName, TEXT("Context_MainItem"));
       if (!ItemRow)
       {
          UE_LOG(LogTemp, Warning, TEXT("Main recipe item %s not found in DataTable!"), *Req.Item.RowName.ToString());
          continue;
       }
       
       ReqCheck.RequiredItemID = ItemRow->ID;
       ReqCheck.ItemMetaData = ItemRow->ItemMetaData;
       ReqCheck.AmountNeed = Req.Quantity * Amount;
       ReqCheck.AmountHave = GetItemAmountByID(ItemRow->ID);
    	
       bool bSatisfied = HasItemWithQuantity(ItemRow->ID, ReqCheck.AmountNeed);
       bool bFoundAlternativeSatisfy = false;
    	
       for (const FAlternativeItem& Alt : Req.Alternatives)
       {
          if (!Alt.Item.DataTable) continue;
       	
          const FItemData* AltItemRow = Alt.Item.DataTable->FindRow<FItemData>(Alt.Item.RowName, TEXT("Context_AltItem"));
          if (!AltItemRow)
          {
             UE_LOG(LogTemp, Warning, TEXT("Alternative recipe item %s not found in DataTable!"), *Alt.Item.RowName.ToString());
             continue;
          }

          FAlternativeItemRequirementCheck AltCheck;
          AltCheck.RequiredItemID = AltItemRow->ID;
          AltCheck.ItemMetaData = ItemRow->ItemMetaData;
          AltCheck.AmountNeed = Alt.Quantity * Amount;
          AltCheck.AmountHave = GetItemAmountByID(AltItemRow->ID);
          AltCheck.bIsSatisfied = HasItemWithQuantity(AltItemRow->ID, AltCheck.AmountNeed);

          if (AltCheck.bIsSatisfied)
          {
	          bFoundAlternativeSatisfy = true;
          }

          ReqCheck.AlternativeRequirementCheck.Add(AltCheck);
       }
    	
       if (!bSatisfied)
       {
          bSatisfied = bFoundAlternativeSatisfy;
       }

       ReqCheck.bIsSatisfied = bSatisfied;
       Result.Requirements.Add(ReqCheck);
    }
	
    Result.bCanCraft = Result.Requirements.FindByPredicate([](const FRecipeItemRequirementCheck& R) { return !R.bIsSatisfied; }) == nullptr;

    return Result;
}

void UCraftingComponent::EnqueueRecipeRequest(FItemRecipeRow ItemRecipeRow, int32 Count)
{
	if (!GetOwner()) return;

	if (Count <= 0)
		return;

	if (GetOwner()->HasAuthority())
	{
		Server_EnqueueRecipe(ItemRecipeRow, Count);
	}
	else
	{
		HandleEnqueueRecipe(ItemRecipeRow, Count);
	}
}

void UCraftingComponent::CancelCurrentCraft()
{
	if (!GetOwner()) return;

	if (!GetOwner()->HasAuthority())
	{
		Server_CancelCurrentCraft();
		return;
	}
}

void UCraftingComponent::RequestMoveQueueItem(FName RecipeID, bool bMoveUp)
{
	int32 Index = RecipeQueue.IndexOfByPredicate([RecipeID](const FQueuedRecipe& Item) {
		return Item.ItemRecipeRow.ID == RecipeID;
	});

	if (Index == INDEX_NONE) return;

	if (bMoveUp && Index > 0)
	{
		RecipeQueue.Swap(Index, Index - 1);
		OnRep_Queue();
	}
	else if (!bMoveUp && Index < RecipeQueue.Num() - 1)
	{
		RecipeQueue.Swap(Index, Index + 1);
		OnRep_Queue();
	}

	CurrentCraftingRecipe = RecipeQueue[0];
}

void UCraftingComponent::OnRep_InputInventory()
{
	if (bDebugMode)
	{
		UE_LOG(LogTemp, Log, TEXT("Input Inventory updated on %s"), 
			GetOwner()->HasAuthority() ? TEXT("Server") : TEXT("Client"));
	}
	
	RecalculateAvailableRecipes();
}

void UCraftingComponent::OnRep_CachedRecipes()
{
	OnAvailableRecipesChanged.Broadcast(CachedRecipeResults);
}

void UCraftingComponent::OnRep_AvailableRecipes()
{
	if (bDebugMode)
	{
		UE_LOG(LogTemp, Log, TEXT("Available Recipes replicated to client. Count: %d"), AvailableRecipes.Num());
	}
}

void UCraftingComponent::ProcessCraftTick()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (CurrentCraftingRecipe.ItemRecipeRow.ID.IsNone())
		return;
	
	auto EffectiveSpeed  = 1.0f * ProcessCraftTickTime;

	CurrentCraftingRecipe.CurrentProgress += EffectiveSpeed ;

	CurrentRecipeProgressChanged();

	if (CurrentCraftingRecipe.CurrentProgress >= CurrentCraftingRecipe.ItemRecipeRow.CraftTime)
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

void UCraftingComponent::StartCurrentRecipe(const FQueuedRecipe& Item)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (ConsumePolicy == ECraftingResourceConsumePolicy::OnCraftStart)
	{
		auto ResultCanCraft = CanCraft(Item.ItemRecipeRow, 1);
		if (!ResultCanCraft.bCanCraft)
			return;
	}
	
	GetWorld()->GetTimerManager().ClearTimer(CraftTimerHandle);

	CurrentCraftingRecipe = Item;
	CurrentCraftingRecipe.CurrentProgress = 0.f;

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
	
	if (CurrentCraftingRecipe.Count > 1)
	{
		CurrentCraftingRecipe.Count--;
		CurrentCraftingRecipe.CurrentProgress = 0.f;
		CurrentRecipeProgressChanged();
		
		GetWorld()->GetTimerManager().SetTimer(
			CraftTimerHandle,
			this,
			&UCraftingComponent::ProcessCraftTick,
			ProcessCraftTickTime,
			true
		);

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

bool UCraftingComponent::ConsumeResourcesForRecipe(FName RecipeID, int32 Count)
{
	return false;
}

void UCraftingComponent::RefundResourcesForRecipe(FName RecipeID, int32 Count)
{
}

void UCraftingComponent::OnRep_Queue()
{
	OnCraftQueueChanged.Broadcast(RecipeQueue);
}

void UCraftingComponent::OnRep_CurrentRecipe()
{
	//OnCurrentRecipeChanged.Broadcast(CurrentCraftingRecipe);
}

void UCraftingComponent::CurrentRecipeProgressChanged()
{
	OnCraftDataChanged.Broadcast(CurrentCraftingRecipe);
}

void UCraftingComponent::Server_EnqueueRecipe_Implementation(FItemRecipeRow ItemRecipeRow, int32 Count)
{
	HandleEnqueueRecipe(ItemRecipeRow, Count);
}

void UCraftingComponent::HandleEnqueueRecipe(FItemRecipeRow ItemRecipeRow, int32 Count)
{
	if (ConsumePolicy == ECraftingResourceConsumePolicy::OnQueueAdd)
	{
		auto ResultCanCraft = CanCraft(ItemRecipeRow, Count);
		if (!ResultCanCraft.bCanCraft)
			return;
	}

	RecipeQueue.Add(FQueuedRecipe(ItemRecipeRow, Count));
	OnRep_Queue();
	
	if (CurrentCraftingRecipe.ItemRecipeRow.ID.IsNone() || CurrentCraftingRecipe.Count == 0)
	{
		TryStartNext();
	}
}

void UCraftingComponent::Server_CancelCurrentCraft_Implementation()
{
	CancelCurrentCraft();
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

void UCraftingComponent::Multicast_OnCraftCanceled_Implementation(FName RecipeID)
{
	OnCraftCanceled.Broadcast(RecipeID);
}

