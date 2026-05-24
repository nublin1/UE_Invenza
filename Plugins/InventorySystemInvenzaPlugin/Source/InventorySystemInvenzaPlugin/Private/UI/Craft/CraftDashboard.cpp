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
	CraftComponentPtr->OnCraftProgressChanged.AddDynamic(this, &UCraftDashboard::UpdateCurrentCraftProgress);
	CraftComponentPtr->OnCraftQueueChanged.AddDynamic(this, &UCraftDashboard::UpdateQueueCraftList);
}

void UCraftDashboard::AddTaskBtnPressed(UUIButton* Btn)
{
	if (!CraftMenuRef)
		return;

	CraftMenuRef->SetVisibility(ESlateVisibility::Visible);
}

void UCraftDashboard::SetNewCurrentCraftRecipe(FQueuedRecipe NewQueuedRecipe)
{
	if (CraftProgressSimple)
	{
		CraftProgressSimple->SetNewCraft(NewQueuedRecipe);
	}
}

void UCraftDashboard::UpdateCurrentCraftProgress(float NewValue)
{
	if (CraftProgressSimple)
	{
		CraftProgressSimple->UpdateProgress(NewValue);
	}
}

void UCraftDashboard::UpdateQueueCraftList(TArray<FQueuedRecipe>& NewRecipeQueue)
{
	if (!QueueCraftList || NewRecipeQueue.IsEmpty())
		return;

	QueueCraftList->QueueList->ClearListItems();

	TArray<UProductionQueueListEntryObject*> NewProductionQueueList;

	for (int i = 0; i < NewRecipeQueue.Num(); i++)
	{
		UProductionQueueListEntryObject* ProductionQueueList = NewObject<UProductionQueueListEntryObject>();

		ProductionQueueList->RecipeRow = NewRecipeQueue[i].ItemRecipeRow;
		ProductionQueueList->AmountInQueue = NewRecipeQueue[i].Count;
		ProductionQueueList->CurrentProgress = NewRecipeQueue[i].CurrentProgress;

		NewProductionQueueList.Add(ProductionQueueList);
	}

	if (!NewProductionQueueList.IsEmpty())
	{
		QueueCraftList->SetNewProductionQueueList(NewProductionQueueList);
	}
}
