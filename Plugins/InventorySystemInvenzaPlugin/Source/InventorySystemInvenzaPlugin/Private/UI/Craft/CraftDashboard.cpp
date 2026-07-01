// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/CraftDashboard.h"

#include "Components/ListView.h"
#include "Data/CraftSystem/Entries/ProductionQueueListEntryObject.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Craft/CraftControlPanel.h"
#include "UI/Craft/CraftMenuChoose.h"
#include "UI/Craft/Lists/QueueCraftList.h"
#include "Utility/InventoryUtility.h"

UCraftDashboard::UCraftDashboard()
{
}

void UCraftDashboard::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UCraftDashboard::NativeConstruct()
{
	Super::NativeConstruct();

	if (CraftControlPanel)
	{
		for (auto Btn : CraftControlPanel->Execute_GetButtons(CraftControlPanel))
		{
			if (Btn->GetBtnTag() == AddTaskBtnTag)
				Btn->OnButtonClicked.AddDynamic(this, &UCraftDashboard::AddTaskBtnPressed);
			
			if (Btn->GetBtnTag() == PauseBtnTag)
				Btn->OnButtonClicked.AddDynamic(this, &UCraftDashboard::PauseBtnPressed);
		}
	}

	if (QueueCraftList)
	{
		QueueCraftList->OnQueueOrderChangeRequested.AddDynamic(this, &UCraftDashboard::HandleQueueOrderChangeRequested);
		QueueCraftList->OnQueueItemDeleteRequested.AddDynamic(this, &UCraftDashboard::HandleQueueItemDeleteRequested);
	}
}

void UCraftDashboard::InitializeCraftComponentBindings()
{
	if (!CraftComponentPtr)
		return;
	
	CraftComponentPtr->OnCurrentCraftDataChanged.AddDynamic(this, &UCraftDashboard::UpdateCurrentCraftProgress);
	CraftComponentPtr->OnCraftQueueChanged.AddDynamic(this, &UCraftDashboard::UpdateQueueCraftList);
}

void UCraftDashboard::SetCraftComponentPtr(UCraftingComponent* NewCraftingComponent)
{
	if (!NewCraftingComponent)
		return;
	
	if (CraftComponentPtr)
	{
		CraftComponentPtr->OnAvailableRecipesChanged.RemoveAll(this);
	}

	CraftComponentPtr = NewCraftingComponent;

	InitializeCraftComponentBindings();
}

void UCraftDashboard::AddTaskBtnPressed(UUIButton* Btn)
{
	if (!CraftComponentPtr)
		return;
}

void UCraftDashboard::PauseBtnPressed(UUIButton* Btn)
{
	if (!CraftComponentPtr)
		return;
	
	if (!Btn->GetBtnTag().IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PauseBtnPressed] Btn has an invalid Tag."));
		return;
	}
	
	const auto* MySettings = UInvenzaInventorySettingsSubsystem::GetSettingsStatic(this);
	if (!MySettings)
		return;
	
	const FGameplayTag BtnTag = Btn->GetBtnTag();
	const TArray<FBlockReasonData>& ActiveBlocks = CraftComponentPtr->GetBlocksReasons();
	const bool bAlreadyBlocked = ActiveBlocks.ContainsByPredicate(
		[&BtnTag](const FBlockReasonData& Data)
		{
			return Data.Tag == BtnTag;
		}
	);
	
	const FBlockReasonData* BlockReason = MySettings->FindBlockReason(BtnTag);
	if (BlockReason)
	{
		CraftComponentPtr->SetBlockStateRequest(*BlockReason, !bAlreadyBlocked);
	}
}

void UCraftDashboard::UpdateCurrentCraftProgress(const FQueuedRecipe& Recipe)
{
	if (QueueCraftList)
	{
		QueueCraftList->UpdateDataInRecipe(Recipe);
	}
}

void UCraftDashboard::UpdateQueueCraftList(const TArray<FQueuedRecipe>& NewRecipeQueue)
{
	if (!QueueCraftList)
		return;
	
	QueueCraftList->SetNewProductionQueueList(NewRecipeQueue);
}

void UCraftDashboard::HandleQueueOrderChangeRequested(const FName RecipeID, const int32 QueueIndex, const bool bMoveUp)
{
	if (CraftComponentPtr)
	{
		CraftComponentPtr->RequestMoveQueueItem(RecipeID, QueueIndex, bMoveUp);
	}
}

void UCraftDashboard::HandleQueueItemDeleteRequested(int32 QueueIndex)
{
	if (CraftComponentPtr)
	{
		CraftComponentPtr->CancelRecipeRequest(QueueIndex);
	}
}
