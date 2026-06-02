// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/Lists/QueueCraftList.h"

#include "Components/ListView.h"
#include "Data/CraftSystem/Entries/ProductionQueueListEntryObject.h"
#include "UI/Craft/Lists/QueueCraftListEntryWidget.h"


UQueueCraftList::UQueueCraftList()
{
}

void UQueueCraftList::NativeConstruct()
{
	Super::NativeConstruct();

	QueueList->OnEntryWidgetGenerated().AddUObject(
	this, &UQueueCraftList::OnEntryGenerated);
}

void UQueueCraftList::SetNewProductionQueueList(const TArray<FQueuedRecipe>& InRecipeQueue)
{
	ProductionQueueList.Empty();
	QueueList->ClearListItems();

	if (InRecipeQueue.IsEmpty())
	{
		return;
	}
	
	for (const FQueuedRecipe& Recipe : InRecipeQueue)
	{
		UProductionQueueListEntryObject* NewEntry = NewObject<UProductionQueueListEntryObject>(this);

		NewEntry->SetQueuedRecipe(Recipe);

		ProductionQueueList.Add(NewEntry);
	}
	
	UpdateProductionQueueList();
}

void UQueueCraftList::UpdateDataInRecipe(FQueuedRecipe& UpdatedRecipe)
{
	TObjectPtr<UProductionQueueListEntryObject>* FoundEntryPtr = ProductionQueueList.FindByPredicate(
		[&UpdatedRecipe](const TObjectPtr<UProductionQueueListEntryObject>& Entry)
		{
			return Entry && Entry->GetQueuedRecipeData().ItemRecipeRow.ID == UpdatedRecipe.ItemRecipeRow.ID;
		}
	);

	if (FoundEntryPtr && *FoundEntryPtr)
	{
		(*FoundEntryPtr)->UpdateData(UpdatedRecipe.Count, UpdatedRecipe.CurrentProgress);
	}
}

void UQueueCraftList::UpdateProductionQueueList()
{
	QueueList->ClearListItems();
	
	for (int i = 0; i< ProductionQueueList.Num(); i++)
	{
		QueueList->AddItem(ProductionQueueList[i]);
	}
}

void UQueueCraftList::HandleMoveItemRequest(UObject* Item, bool bMoveUp)
{
	UProductionQueueListEntryObject* EntryObj = Cast<UProductionQueueListEntryObject>(Item);
	if (!EntryObj || EntryObj->GetQueuedRecipeData().ItemRecipeRow.ID.IsNone()) return;
	
	OnQueueOrderChangeRequested.Broadcast(EntryObj->GetQueuedRecipeData().ItemRecipeRow.ID, bMoveUp);
	
}

void UQueueCraftList::OnEntryGenerated(UUserWidget& UserWidget)
{
	if (UQueueCraftListEntryWidget* Entry =
			Cast<UQueueCraftListEntryWidget>(&UserWidget))
	{
		Entry->OnMoveRequested.AddUniqueDynamic(
			this, &UQueueCraftList::HandleMoveItemRequest);
	}
}