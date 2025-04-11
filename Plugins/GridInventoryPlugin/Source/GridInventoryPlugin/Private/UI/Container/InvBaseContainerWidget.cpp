//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Container/InvBaseContainerWidget.h"

#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/UIManagerComponent.h"
#include "Components/Button.h"
#include "Components/NamedSlot.h"
#include "Components/TextBlock.h"
#include "UI/Core/CoreHUDWidget.h"
#include "UI/Core/OperationsPanel/OperationPanelWidget.h"
#include "UI/Core/Weight/InvWeightWidget.h"
#include "UI/Inventory/BaseInventoryWidget.h"

UInvBaseContainerWidget::UInvBaseContainerWidget()
{
}

UBaseInventoryWidget* UInvBaseContainerWidget::GetInventoryFromContainerSlot()
{
	if (!ContainerSlot || ContainerSlot->GetChildrenCount() == 0)
	{
		return nullptr;
	}

	if (UBaseInventoryWidget* BaseInventoryWidget = Cast<UBaseInventoryWidget>(ContainerSlot->GetChildAt(0)))
	{
		return BaseInventoryWidget;
	}

	return nullptr;
}

void UInvBaseContainerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HeaderSlot && HeaderSlot->GetChildrenCount()>0)
	{
		if (auto Widget = Cast<UBaseUserWidget>(HeaderSlot->GetChildAt(0)))
			Widget->SetParentWidget(this);
	}

	if (InvWeight)
	{
		auto Inv = GetInventoryFromContainerSlot();
		if (!Inv)
			return;

		if (Inv->GetWeightCapacity() <0)
			InvWeight->SetVisibility(ESlateVisibility::Collapsed);
		else
		{
			Inv->OnWightUpdatedDelegate.AddDynamic(this, &UInvBaseContainerWidget::UInvBaseContainerWidget::UpdateWeightInfo);
		}
	}
	
	if (OperationsSlot && OperationsSlot->GetChildrenCount() > 0)
	{
		if (auto OperationsWidget = Cast<UOperationPanelWidget>(OperationsSlot->GetChildAt(0)))
		{
			if (OperationsWidget->Button_TakeAll) OperationsWidget->Button_TakeAll->OnClicked.AddDynamic(this, &UInvBaseContainerWidget::TakeAll);
			if (OperationsWidget->Button_PlaceAll) OperationsWidget->Button_PlaceAll->OnClicked.AddDynamic(this, &UInvBaseContainerWidget::PlaceAll);
			if (OperationsWidget->Button_Sort) OperationsWidget->Button_Sort->OnClicked.AddDynamic(this, &UInvBaseContainerWidget::SortItems);
		}
	}
}

void UInvBaseContainerWidget::UpdateWeightInfo(float InventoryTotalWeight, float InventoryWeightCapacity)
{
	FString Text = {" " + FString::SanitizeFloat(InventoryTotalWeight) + "/" + FString::SanitizeFloat(InventoryWeightCapacity)};
	InvWeight->WeightInfo->SetText(FText::FromString(Text));
}

void UInvBaseContainerWidget::TakeAll()
{
	UBaseInventoryWidget* SourceInv = GetInventoryFromContainerSlot(); 
	if (!SourceInv)
		return;

	auto ManagerComponent = GetOwningPlayerPawn()->FindComponentByClass<UUIManagerComponent>();
	if (!ManagerComponent || !ManagerComponent->GetCoreHUDWidget())
		return;
    
	UBaseInventoryWidget* TargetInv = ManagerComponent->GetCoreHUDWidget()->GetMainInvWidget()->GetInventoryFromContainerSlot();
	if (!TargetInv)
		return;
    
	TransferAllItems(SourceInv, TargetInv);
}

void UInvBaseContainerWidget::PlaceAll()
{
	UBaseInventoryWidget* TargetInv = GetInventoryFromContainerSlot(); 
	if (!TargetInv)
		return;

	auto ManagerComponent = GetOwningPlayerPawn()->FindComponentByClass<UUIManagerComponent>();
	if (!ManagerComponent || !ManagerComponent->GetCoreHUDWidget())
		return;

	UBaseInventoryWidget* SourceInv = ManagerComponent->GetCoreHUDWidget()->GetMainInvWidget()->GetInventoryFromContainerSlot();
	if (!SourceInv)
		return;
    
	TransferAllItems(SourceInv, TargetInv);
}

void UInvBaseContainerWidget::TransferAllItems(UBaseInventoryWidget* SourceInv, UBaseInventoryWidget* TargetInv)
{
	if (!SourceInv || !TargetInv) return;
	
	UItemCollection* SourceCollection = SourceInv->GetItemCollection();
	if (!SourceCollection)
	{
		return;
	}
    
	TArray<UItemBase*> AllItems = SourceCollection->GetAllItemsByContainer(SourceInv);
	if (AllItems.IsEmpty())
	{
		return;
	}
	
	for (UItemBase* Item : AllItems)
	{
		FItemMoveData MoveData;
		MoveData.SourceItem = Item;
		MoveData.SourceInventory = SourceInv;
		MoveData.TargetInventory = TargetInv;
		
		FItemAddResult Result = TargetInv->HandleAddItem(MoveData, false);
		if (Result.OperationResult == EItemAddResult::IAR_AllItemAdded || 
			Result.OperationResult == EItemAddResult::IAR_PartialAmountItemAdded)
		{
			SourceInv->HandleRemoveItem(Item, Item->GetQuantity());
		}
	}
}

void UInvBaseContainerWidget::SortItems()
{
	const auto Inv = GetInventoryFromContainerSlot();
	if (!Inv)
		return;

	TArray<UItemBase*> AllItems = Inv->GetItemCollection()->GetAllItemsByContainer(Inv);
	if (AllItems.IsEmpty())
		return;
	
	AllItems.Sort([](UItemBase& A, UItemBase& B)
	{
		return A.GetItemRef().ItemTextData.Name.ToString() < B.GetItemRef().ItemTextData.Name.ToString();
	});
	
	TArray<UItemBase*> DuplicatedItems;
	for (UItemBase* Item : AllItems)
	{
		if (Item)
		{
			if (UItemBase* DuplicatedItem = Item->DuplicateItem())
			{
				DuplicatedItems.Add(DuplicatedItem);
			}
		}
	}

	for (auto Item : AllItems)
	{
		Inv->HandleRemoveItem(Item, Item->GetQuantity());
	}

	for (const auto DupItem : DuplicatedItems)
	{
		FItemMoveData MoveData;
		MoveData.SourceItem = DupItem;
		MoveData.SourceInventory = Inv;
		auto Result = Inv->HandleAddItem(MoveData, false);
	}
}
