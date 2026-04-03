//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/Container/InvBaseContainerWidget.h"

#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Components/Button.h"
#include "Components/NamedSlot.h"
#include "Components/TextBlock.h"
#include "Data/Inventory/InventoryBase.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/MovableTitleBar/MovableTitleBar.h"
#include "UI/Core/OperationsPanel/OperationPanelWidget.h"
#include "UI/Core/Weight/InvWeightWidget.h"
#include "GameFramework/Pawn.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Inventory/SlotbasedInventoryWidget.h"


UInvBaseContainerWidget::UInvBaseContainerWidget()
{
}

void UInvBaseContainerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//UE_LOG(LogTemp, Log, TEXT("FName %s"), *GetFName().ToString());

	if (TitleBar)
	{
		TitleBar->SetParentWidget(this);
		TitleBar->TitleName->SetText(Title);
		TitleBar->Button_Close->OnClicked.AddDynamic(this, &UInvBaseContainerWidget::CloseButtonClicked);
		if (!bIsShowCloseButton)
			TitleBar->Button_Close->SetVisibility(ESlateVisibility::Collapsed);
		if (!bIsShowTotalMoney && InvMoney)
			InvMoney->SetVisibility(ESlateVisibility::Collapsed);
		if (!bIsShowWeight)
			InvWeight->SetVisibility(ESlateVisibility::Collapsed);
		
	}
}

void UInvBaseContainerWidget::InitializeInventoryBindings()
{
	UUInventoryWidgetBase* InventoryWidget = GetInventoryWidgetFromContainerSlot();
	if (!InventoryWidget)
		return;

	auto Inventory = InventoryWidget->GetInventoryRef();
	if (!Inventory)
		return;

	InventoryRef = Inventory;

	if (InvWeight)
	{
		if (InventoryRef->GetInventorySettings().InventoryMaxWeightCapacity < 0)
			InvWeight->SetVisibility(ESlateVisibility::Collapsed);
		else
		{
			InventoryRef->OnWeightUpdatedDelegate.AddDynamic(this, &UInvBaseContainerWidget::UpdateWeightInfo);
			InventoryRef->UpdateWeightInfo();
		}
	}

	if (OperationsSlot && OperationsSlot->GetChildrenCount() > 0)
	{
		if (auto OperationsWidget = Cast<UOperationPanelWidget>(OperationsSlot->GetChildAt(0)))
		{
			if (OperationsWidget->Button_TakeAll && OperationsWidget->Button_TakeAll->MainButton)
				OperationsWidget->Button_TakeAll->MainButton->OnClicked.AddDynamic(
					this, &UInvBaseContainerWidget::TakeAll);
			if (OperationsWidget->Button_PlaceAll && OperationsWidget->Button_PlaceAll->MainButton)
				OperationsWidget->Button_PlaceAll->MainButton->OnClicked.AddDynamic(
					this, &UInvBaseContainerWidget::PlaceAll);
			if (OperationsWidget->Button_Sort && OperationsWidget->Button_Sort->MainButton)
				OperationsWidget->Button_Sort->MainButton->OnClicked.AddDynamic(
					this, &UInvBaseContainerWidget::SortItems);
		}
	}

	Inventory->OnMoneyUpdatedDelegate.AddDynamic(this, &UInvBaseContainerWidget::UpdateMoneyInfo);
	Inventory->UpdateMoneyInfo();
}

void UInvBaseContainerWidget::ChangeInventoryInContainerSlot(TSubclassOf<UInvenzaBaseWidget> NewInventory)
{
	if (!NewInventory) return;

	auto NewInvWidget = CreateWidget<UUInventoryWidgetBase>(GetWorld(), NewInventory);
	if (!NewInvWidget) return;

	NewInvWidget->SetUISettings(GetInventoryWidgetFromContainerSlot()->GetUISettings());
	NewInvWidget->InitializeInventoryWidget();

	ContainerSlot->ClearChildren();
	ContainerSlot->AddChild(NewInvWidget);
}

void UInvBaseContainerWidget::CloseButtonClicked()
{
	if (OnClose.IsBound())
		OnClose.Broadcast(this);
}

UUInventoryWidgetBase* UInvBaseContainerWidget::GetInventoryWidgetFromContainerSlot()
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

void UInvBaseContainerWidget::UpdateWeightInfo(float InventoryTotalWeight)
{
	if (bIsShowWeight)
	{
		FString RoundedString = FString::Printf(TEXT("%0.1f"), InventoryTotalWeight);
		FString Text = {
			" " + RoundedString + "/" + FString::SanitizeFloat(
				InventoryRef->GetInventorySettings().InventoryMaxWeightCapacity)
		};
		InvWeight->WeightInfo->SetText(FText::FromString(Text));
	}
	else
	{
		InvWeight->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInvBaseContainerWidget::UpdateMoneyInfo(int32 TotalMoney)
{
	if (!InvMoney)
		return;

	if (bIsShowTotalMoney)
	{
		FString MoneyText = {"$ " + FString::FromInt(TotalMoney)};
		InvMoney->UpdateText(FText::FromString(MoneyText));
	}
	else
	{
		InvMoney->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInvBaseContainerWidget::TakeAll()
{
	/*UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager || !InventoryManager->GetCoreHUDWidget())
		return;
    
	auto TargetInv = InventoryManager->GetMainInventory();
	if (!TargetInv)
		return;
    
	TransferAllItems(this, TargetInv);*/
}

void UInvBaseContainerWidget::PlaceAll()
{
	/*UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager || !InventoryManager->GetCoreHUDWidget())
		return;

	auto SourceInv = InventoryManager->GetCoreHUDWidget()->GetMainInvWidget();
	if (!SourceInv)
		return;
    
	TransferAllItems(SourceInv, this);*/
}

void UInvBaseContainerWidget::TransferAllItems(UInvBaseContainerWidget* SourceContainer,
                                               UInvBaseContainerWidget* TargetContainer)
{
	/*if (!SourceContainer || !TargetContainer) return;

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
	}*/
}

void UInvBaseContainerWidget::SortItems()
{
	if (!InventoryRef)
		return;

	InventoryRef->SortItemsInContainerByName();
}
