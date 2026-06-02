// Nublin Studio 2026 All Rights Reserved.


#include "Data/CraftSystem/Entries/ProductionQueueListEntryObject.h"

void UProductionQueueListEntryObject::UpdateData(int32 NewCount, float NewProgress)
{
	QueuedRecipeData.Count = NewCount;
	QueuedRecipeData.CurrentProgress = NewProgress;

	OnDataChanged.Broadcast();
}