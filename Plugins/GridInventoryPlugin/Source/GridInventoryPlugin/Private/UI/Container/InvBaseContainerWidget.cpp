//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Container/InvBaseContainerWidget.h"

#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Components/Button.h"
#include "Components/NamedSlot.h"
#include "Components/TextBlock.h"
#include "UI/Core/CoreHUDWidget.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/MovableTitleBar/MovableTitleBar.h"
#include "UI/Core/OperationsPanel/OperationPanelWidget.h"
#include "UI/Core/Weight/InvWeightWidget.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"


UInvBaseContainerWidget::UInvBaseContainerWidget()
{
}

void UInvBaseContainerWidget::ChangeInventoryInContainerSlot(TSubclassOf<UBaseUserWidget> NewInventory)
{
	if (!NewInventory) return;

	auto NewInvWidget = CreateWidget<UUInventoryWidgetBase>(GetWorld(), NewInventory);
	if (!NewInvWidget) return;
	
	NewInvWidget->SetUISettings(GetInventoryFromContainerSlot()->GetUISettings());
	NewInvWidget->InitializeInventory();

	ContainerSlot->ClearChildren();
	ContainerSlot->AddChild(NewInvWidget);
	
}

void UInvBaseContainerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UUInventoryWidgetBase* Inventory = GetInventoryFromContainerSlot();
	if (TitleBar)
	{
		TitleBar->SetParentWidget(this);
		TitleBar->TitleName->SetText(Title);
		TitleBar->Button_Close->OnClicked.AddDynamic(this, &UInvBaseContainerWidget::CloseButtonClicked);
		if (!bIsShowCloseButton)
			TitleBar->Button_Close->SetVisibility(ESlateVisibility::Collapsed);
		if (!bIsShowTotalMoney)
			TitleBar->Money->SetVisibility(ESlateVisibility::Collapsed);
	}	

	if (InvWeight)
	{
		if (!Inventory)
			return;

		if (Inventory->GetInventorySettings().InventoryWeightCapacity <0)
			InvWeight->SetVisibility(ESlateVisibility::Collapsed);
		else
		{
			Inventory->OnWightUpdatedDelegate.AddDynamic(this, &UInvBaseContainerWidget::UpdateWeightInfo);
		}
	}

	if (OperationsSlot && OperationsSlot->GetChildrenCount() > 0)
	{
		if (auto OperationsWidget = Cast<UOperationPanelWidget>(OperationsSlot->GetChildAt(0)))
		{
			if (OperationsWidget->Button_TakeAll && OperationsWidget->Button_TakeAll->MainButton)
				OperationsWidget->Button_TakeAll->MainButton->OnClicked.AddDynamic(this, &UInvBaseContainerWidget::TakeAll);
			if (OperationsWidget->Button_PlaceAll && OperationsWidget->Button_PlaceAll->MainButton)
				OperationsWidget->Button_PlaceAll->MainButton->OnClicked.AddDynamic(this, &UInvBaseContainerWidget::PlaceAll);
			if (OperationsWidget->Button_Sort && OperationsWidget->Button_Sort->MainButton)
				OperationsWidget->Button_Sort->MainButton->OnClicked.AddDynamic(this, &UInvBaseContainerWidget::SortItems);
			
		}
	}

	Inventory->OnMoneyUpdatedDelegate.AddDynamic(this, &UInvBaseContainerWidget::UpdateMoneyInfo);
}

void UInvBaseContainerWidget::CloseButtonClicked()
{
	if (OnClose.IsBound())
		OnClose.Broadcast(this);
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

void UInvBaseContainerWidget::UpdateWeightInfo(float InventoryTotalWeight, float InventoryWeightCapacity)
{
	if (bIsShowWeight)
	{
		FString RoundedString = FString::Printf(TEXT("%0.1f"), InventoryTotalWeight);
		FString Text = {" " + RoundedString + "/" + FString::SanitizeFloat(InventoryWeightCapacity)};
		InvWeight->WeightInfo->SetText(FText::FromString(Text));
	}
	else
	{
		InvWeight->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInvBaseContainerWidget::UpdateMoneyInfo(int32 TotalMoney)
{
	if (!TitleBar)
		return;

	if (bIsShowTotalMoney)
	{
		FString MoneyText = {"$ " + FString::FromInt(TotalMoney)};
		TitleBar->Money->SetText(FText::FromString(MoneyText));
	}
	else
	{
		TitleBar->Money->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInvBaseContainerWidget::TakeAll()
{
	UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager || !InventoryManager->GetCoreHUDWidget())
		return;
    
	auto TargetInv = InventoryManager->GetMainInventory();
	if (!TargetInv)
		return;
    
	TransferAllItems(this, TargetInv);
}

void UInvBaseContainerWidget::PlaceAll()
{
	UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager || !InventoryManager->GetCoreHUDWidget())
		return;

	auto SourceInv = InventoryManager->GetCoreHUDWidget()->GetMainInvWidget();
	if (!SourceInv)
		return;
    
	TransferAllItems(SourceInv, this);
}

void UInvBaseContainerWidget::TransferAllItems(UInvBaseContainerWidget* SourceContainer, UInvBaseContainerWidget* TargetContainer)
{
	if (!SourceContainer || !TargetContainer) return;

	auto SourceInv = SourceContainer->GetInventoryFromContainerSlot();
	auto TargetInv = TargetContainer->GetInventoryFromContainerSlot();
	
	UItemCollection* SourceCollection = SourceInv->GetInventoryData().ItemCollectionLink;
	if (!SourceCollection)
	{
		return;
	}
    
	TArray<UItemBase*> AllItems = SourceCollection->GetAllItemsByContainer(SourceContainer);
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

	Inv->GetInventoryData().ItemCollectionLink->SortInContainer(this);
	
	/*
	TArray<UItemBase*> AllItems = Inv->GetInventoryData().ItemCollectionLink->GetAllItemsByContainer(this);
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
	}*/
}
