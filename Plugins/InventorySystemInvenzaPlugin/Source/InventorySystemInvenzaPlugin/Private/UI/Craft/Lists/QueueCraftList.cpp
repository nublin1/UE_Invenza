// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/Lists/QueueCraftList.h"

#include "Components/ListView.h"
#include "Data/CraftSystem/Entries/ProductionQueueListEntryObject.h"
#include "Data/CraftSystem/Entries/QueueCraftListEntry.h"


UQueueCraftList::UQueueCraftList()
{
}

void UQueueCraftList::NativeConstruct()
{
	Super::NativeConstruct();

	QueueList->OnEntryWidgetGenerated().AddUObject(
	this, &UQueueCraftList::OnEntryGenerated);
}

void UQueueCraftList::SetNewProductionQueueList(TArray<UProductionQueueListEntryObject*> InArray)
{
	ProductionQueueList = InArray;

	UpdateProductionQueueList();
}

void UQueueCraftList::UpdateProductionQueueList()
{
	QueueList->ClearListItems();
	
	for (int i = 0; i< ProductionQueueList.Num(); i++)
	{
		QueueList->AddItem(ProductionQueueList[i]);
	}
}

void UQueueCraftList::MoveItem(UObject* Item, bool bMoveUp)
{
	int32 Index = ProductionQueueList.IndexOfByKey(Item);

	if (bMoveUp)
	{
		if (Index != INDEX_NONE && Index > 0)
		{
			ProductionQueueList.Swap(Index, Index - 1);
			QueueList->RequestRefresh();
		}
	}
	else
	{
		if (Index != INDEX_NONE && Index < ProductionQueueList.Num() - 1)
		{
			ProductionQueueList.Swap(Index, Index + 1);
			QueueList->RequestRefresh();
		}
	}
}

void UQueueCraftList::OnEntryGenerated(UUserWidget& UserWidget)
{
	if (UQueueCraftListEntry* Entry =
			Cast<UQueueCraftListEntry>(&UserWidget))
	{
		Entry->OnMoveRequested.AddUniqueDynamic(
			this, &UQueueCraftList::MoveItem);
	}
}