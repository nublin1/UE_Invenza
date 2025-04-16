//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Container/InvBaseContainerWidget.h"

#include "ActorComponents/ItemCollection.h"
#include "Components/Button.h"
#include "Components/NamedSlot.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Core/CoreHUDWidget.h"
#include "UI/Core/MovableTitleBar/MovableTitleBar.h"
#include "UI/Core/OperationsPanel/OperationPanelWidget.h"
#include "UI/Core/Weight/InvWeightWidget.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"
#include "World/AUIManagerActor.h"

UInvBaseContainerWidget::UInvBaseContainerWidget()
{
}

UUInventoryWidgetBase* UInvBaseContainerWidget::GetInventoryFromContainerSlot()
{
	if (!ContainerSlot || ContainerSlot->GetChildrenCount() == 0)
	{
		return nullptr;
	}

	if (UUInventoryWidgetBase* BaseInventoryWidget = Cast<UUInventoryWidgetBase>(ContainerSlot->GetChildAt(0)))
	{
		return BaseInventoryWidget;
	}

	return nullptr;
}

void UInvBaseContainerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UUInventoryWidgetBase* Inventory = GetInventoryFromContainerSlot();

	if (TitleBar)
	{
		TitleBar->SetParentWidget(this);
	}	

	if (InvWeight)
	{
		if (!Inventory)
			return;

		if (Inventory->GetWeightCapacity() <0)
			InvWeight->SetVisibility(ESlateVisibility::Collapsed);
		else
		{
			Inventory->OnWightUpdatedDelegate.AddDynamic(this, &UInvBaseContainerWidget::UpdateWeightInfo);
		}
	}

	Inventory->OnMoneyUpdatedDelegate.AddDynamic(this, &UInvBaseContainerWidget::UpdateMoneyInfo);
	
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
	FString RoundedString = FString::Printf(TEXT("%0.2f"), InventoryTotalWeight);
	FString Text = {" " + RoundedString + "/" + FString::SanitizeFloat(InventoryWeightCapacity)};
	InvWeight->WeightInfo->SetText(FText::FromString(Text));
}

void UInvBaseContainerWidget::UpdateMoneyInfo(int32 TotalMoney)
{
	if (!TitleBar)
		return;

	FString MoneyText = {"$ " + FString::FromInt(TotalMoney)};
	TitleBar->Money->SetText(FText::FromString(MoneyText));
		
}

void UInvBaseContainerWidget::TakeAll()
{
	UUInventoryWidgetBase* SourceInv = GetInventoryFromContainerSlot(); 
	if (!SourceInv)
		return;

	AUIManagerActor* ManagerActor = Cast<AUIManagerActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AUIManagerActor::StaticClass()));
	if (!ManagerActor || !ManagerActor->GetCoreHUDWidget())
		return;
    
	UUInventoryWidgetBase* TargetInv = ManagerActor->GetMainInventory()->GetInventoryFromContainerSlot();
	if (!TargetInv)
		return;
    
	TransferAllItems(SourceInv, TargetInv);
}

void UInvBaseContainerWidget::PlaceAll()
{
	UUInventoryWidgetBase* TargetInv = GetInventoryFromContainerSlot(); 
	if (!TargetInv)
		return;

	AUIManagerActor* ManagerActor = Cast<AUIManagerActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AUIManagerActor::StaticClass()));
	if (!ManagerActor || !ManagerActor->GetCoreHUDWidget())
		return;

	UUInventoryWidgetBase* SourceInv = ManagerActor->GetCoreHUDWidget()->GetMainInvWidget()->GetInventoryFromContainerSlot();
	if (!SourceInv)
		return;
    
	TransferAllItems(SourceInv, TargetInv);
}

void UInvBaseContainerWidget::TransferAllItems(UUInventoryWidgetBase* SourceInv, UUInventoryWidgetBase* TargetInv)
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
