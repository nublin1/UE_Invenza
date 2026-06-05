// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/CraftDashboard.h"

#include "Components/ListView.h"
#include "Data/CraftSystem/Entries/ProductionQueueListEntryObject.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/Progress/GenericProgress.h"
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

	Btn_AddTask->OnButtonClicked.AddDynamic(this, &UCraftDashboard::AddTaskBtnPressed);

	if (QueueCraftList)
	{
		QueueCraftList->OnQueueOrderChangeRequested.AddDynamic(this, &UCraftDashboard::HandleQueueOrderChangeRequested);
		QueueCraftList->OnQueueItemDeleteRequested.AddDynamic(this, &UCraftDashboard::HandleQueueItemDeleteRequested);
	}
}

void UCraftDashboard::BindCraftMenu(UCraftMenuChoose* NewCraftMenuChooseRef)
{
	CraftMenuRef = NewCraftMenuChooseRef;
	if (!CraftMenuRef)
		return;

	CraftComponentPtr = CraftMenuRef->GetCraftComponentPtr();
	
	InitializeCraftComponentBindings();
}

void UCraftDashboard::InitializeCraftComponentBindings()
{
	if (!CraftComponentPtr)
		return;

	CraftComponentPtr->OnNewCraftStarted.AddDynamic(this, &UCraftDashboard::SetNewCurrentCraftRecipe);
	CraftComponentPtr->OnCraftDataChanged.AddDynamic(this, &UCraftDashboard::UpdateCurrentCraftProgress);
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
	if (!CraftMenuRef)
		return;

	CraftMenuRef->SetVisibility(ESlateVisibility::Visible);
}

void UCraftDashboard::SetNewCurrentCraftRecipe(FQueuedRecipe NewQueuedRecipe)
{
}

void UCraftDashboard::UpdateCurrentCraftProgress(FQueuedRecipe& Recipe)
{
	if (QueueCraftList)
	{
		QueueCraftList->UpdateDataInRecipe(Recipe);
	}
}

void UCraftDashboard::UpdateQueueCraftList(TArray<FQueuedRecipe>& NewRecipeQueue)
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
