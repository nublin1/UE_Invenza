// Nublin Studio 2026 All Rights Reserved.


#include "UI/GameMenu/GameMenuLayerInv.h"

#include "Components/NamedSlot.h"
#include "Components/PanelWidget.h"

UGameMenuLayerInv::UGameMenuLayerInv()
{
}

void UGameMenuLayerInv::NativePreConstruct()
{
	
}

void UGameMenuLayerInv::NativeConstruct()
{
	
}

TArray<UUInventoryWidgetBase*> UGameMenuLayerInv::GetAllPawnInventories() const
{
	TArray<UUInventoryWidgetBase*> Result;
	if (!PawnInventories) return Result;

	for (int32 i = 0; i < PawnInventories->GetChildrenCount(); i++)
	{
		if (auto* InvContainer = Cast<UInvBaseContainerWidget>(PawnInventories->GetChildAt(i)))
		{
			if (auto InventoryWidget = Cast<UUInventoryWidgetBase>(InvContainer->ContainerSlot->GetContent()))
				Result.Add(InventoryWidget);
		}
	}
	return Result;
}

