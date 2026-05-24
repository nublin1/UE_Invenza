//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/Crafting/CraftingComponent.h"

#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/ItemData.h"
#include "Data/CraftSystem/ItemRecipe.h"
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
	//DOREPLIFETIME(UCraftingComponent, WorkSpeed);
}

void UCraftingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCraftingComponent::InitStartingRecipes()
{
	if (StartingRecipes.IsEmpty())
		return;
	
	for (auto RecipeHandle : StartingRecipes)
	{
		if (!RecipeHandle.DataTable)
			continue;

		const FItemRecipeRow* RecipeRow = RecipeHandle.DataTable->FindRow<FItemRecipeRow>(
			RecipeHandle.RowName,TEXT("CanCraft"));
		if (!RecipeRow)
			continue;

		FoundRecipes.Add(*RecipeRow);
	}
	
	for (auto RecipeHandle : StartingRecipes)
	{
		//Recipe->
	}
}

FRecipeCheckResult UCraftingComponent::CanCraft(const FItemRecipeRow& RecipeRow, int32 Amount) const
{
	FRecipeCheckResult Result;

	if (bDebugMode == false)
	{
		if (InputInventory == nullptr)
			return Result;
	}
	else
	{
		TArray<FItemIDEntry> InventoryItems;
		return CanCraftWithItems(RecipeRow, InventoryItems, Amount);
	}

	auto InvItems = InputInventory->CollectItemsAggregated();

	return CanCraftWithItems(RecipeRow, InvItems, Amount);
}

FRecipeCheckResult UCraftingComponent::CanCraftWithItems(const FItemRecipeRow& RecipeRow , const TArray<FItemIDEntry>& InventoryItems, int32 Amount)
{
	FRecipeCheckResult Result;

	auto GetItemAmountByID = [&InventoryItems](const FName& ItemID) -> int32
	{
		const FItemIDEntry* Item = InventoryItems.FindByPredicate(
			[&](const FItemIDEntry& I)
			{
				return I.ItemID.IsValid() && I.ItemID == ItemID;
			}
		);
		return Item ? Item->Amount : 0;
	};
	auto HasItemWithQuantity = [&InventoryItems](const FName& ItemID, int32 Quantity)
	{
		const FItemIDEntry* Item = InventoryItems.FindByPredicate(
			[&](const FItemIDEntry& I)
			{
				return I.ItemID == ItemID;
			}
		);
		return Item && Item->Amount >= Quantity;
	};

	// Check each requirement for the subject
	for (const FRecipeItemRequirement& Req : RecipeRow.RequiredItems)
	{
		FRecipeItemRequirementCheck ReqCheck;

		const FItemData* ItemRow = Req.Item.DataTable->FindRow<FItemData>(
		   Req.Item.RowName,
		   *Req.Item.RowName.ToString()
		);
		if (!ItemRow)
		{
			UE_LOG(LogTemp, Warning, TEXT("Recipe item %s not found in DataTable!"), *Req.Item.RowName.ToString());
			continue;
		}
		
		ReqCheck.RequiredItemID = ItemRow->ID;
		ReqCheck.DisplayName = ItemRow->ItemMetaData.ItemTextData.DisplayName;
		ReqCheck.AmountNeed = Req.Quantity * Amount;
		ReqCheck.AmountHave = GetItemAmountByID(ItemRow->ID);

		// main subject
		bool bSatisfied = HasItemWithQuantity(ItemRow->ID, ReqCheck.AmountNeed);

		// all alternatives, but:
		// - alternatives must all be checked
		// - but they must NOT change bSatisfied if bSatisfied is already true
		bool bFoundAlternativeSatisfy = false;

		for (const FAlternativeItem& Alt : Req.Alternatives)
		{
			const FItemData* AltItemRow = Req.Item.DataTable->FindRow<FItemData>(
					   Req.Item.RowName,
					   *Req.Item.RowName.ToString()
					);
			if (!AltItemRow)
			{
				UE_LOG(LogTemp, Warning, TEXT("Recipe item %s not found in DataTable!"), *Req.Item.RowName.ToString());
				continue;
			}
			
			FAlternativeItemRequirementCheck AltCheck;
			AltCheck.RequiredItemID = AltItemRow->ID;
			AltCheck.AmountNeed = Alt.Quantity * Amount;
			AltCheck.AmountHave = GetItemAmountByID(AltItemRow->ID);

			AltCheck.bIsSatisfied = HasItemWithQuantity(AltItemRow->ID, AltCheck.AmountNeed);
			if (AltCheck.bIsSatisfied)
			{
				bFoundAlternativeSatisfy = true;
			}

			ReqCheck.AlternativeRequirementCheck.Add(AltCheck);
		}

		// Итоговое условие
		if (!bSatisfied)
		{
			// если основной не удовлетворён — смотрим альтернативы
			bSatisfied = bFoundAlternativeSatisfy;
		}

		ReqCheck.bIsSatisfied = bSatisfied;

		Result.Requirements.Add(ReqCheck);
	}

	return Result;
}

void UCraftingComponent::EnqueueRecipe(FItemRecipeRow ItemRecipeRow, int32 Count)
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
		Server_EnqueueRecipe(ItemRecipeRow, Count);
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
	
	/*if (CurrentRecipeID != NAME_None)
	{
		// вернуть ресурсы (если нужно)
		RefundResourcesForRecipe(CurrentRecipeID, 1);

		// уведомления
		Multicast_OnCraftCanceled(CurrentRecipeID);
		OnCraftCanceled.Broadcast(CurrentRecipeID);

		// сброс текущего
		CurrentRecipeID = NAME_None;
		CurrentProgress = 0.f;
		CurrentWorkAmount = 1.f;

		OnRep_CurrentRecipe();
		OnRep_Progress();

		// пробуем следующий
		TryStartNext();
	}*/
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

	// если уже крафтим — ничего не делаем
	if (!CurrentCraftingRecipe.ItemRecipeRow.ID.IsNone())
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
	if (OnCraftQueueChanged.IsBound())
		OnCraftQueueChanged.Broadcast(RecipeQueue);
}

void UCraftingComponent::OnRep_CurrentRecipe()
{
	//OnCurrentRecipeChanged.Broadcast(CurrentCraftingRecipe);
}

void UCraftingComponent::CurrentRecipeProgressChanged()
{
	if (OnCraftProgressChanged.IsBound())
	{
		OnCraftProgressChanged.Broadcast(CurrentCraftingRecipe.CurrentProgress);
	}
}

void UCraftingComponent::Server_CancelCurrentCraft_Implementation()
{
	CancelCurrentCraft();
}

void UCraftingComponent::Server_EnqueueRecipe_Implementation(FItemRecipeRow ItemRecipeRow, int32 Count)
{
	if (ConsumePolicy == ECraftingResourceConsumePolicy::OnQueueAdd)
	{
		auto ResultCanCraft = CanCraft(ItemRecipeRow, Count);
		if (!ResultCanCraft.bCanCraft)
			return;
	}

	RecipeQueue.Add(FQueuedRecipe(ItemRecipeRow, Count));
	OnRep_Queue();

	// Если сейчас ничего не делает — запустить
	if (CurrentCraftingRecipe.ItemRecipeRow.ID.IsNone())
	{
		TryStartNext();
	}
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

