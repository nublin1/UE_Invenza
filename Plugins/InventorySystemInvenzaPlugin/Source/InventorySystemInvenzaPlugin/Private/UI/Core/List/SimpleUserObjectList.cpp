// Nublin Studio 2026 All Rights Reserved.


#include "UI/Core/List/SimpleUserObjectList.h"

#include "Components/EditableText.h"
#include "UI/Core/ItemFiltersPanel/FiltersPanel.h"

void USimpleUserObjectList::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ItemFiltersPanel && ItemFiltersPanel->GetSearchText())
	{
		ItemFiltersPanel->GetSearchText()->OnTextChanged.AddDynamic(this, &USimpleUserObjectList::SearchTextChanged);
	}
}
