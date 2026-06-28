// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/CraftDashboard.h"

#include "Components/ListView.h"
#include "Data/CraftSystem/Entries/ProductionQueueListEntryObject.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Craft/CraftControlPanel.h"
#include "UI/Craft/CraftMenuChoose.h"
#include "UI/Craft/Lists/QueueCraftList.h"

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
	
	CraftComponentPtr->SetManualPauseRequest(!CraftComponentPtr->GetIsManualPaused());
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
