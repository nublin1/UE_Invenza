//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/Container/InventoryContainerWidget.h"

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


UInventoryContainerWidget::UInventoryContainerWidget()
{
}

void UInventoryContainerWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (TitleBar)
	{
		TitleBar->TitleName->UpdateText(Title);
		
		if (!bIsShowCloseButton && TitleBar->HeaderCanvasPanel)
			TitleBar->Button_Close->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (OperationsSlot)
	{
		if (!OperationsSlot->GetContent())
		{
			OperationsSlot->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			OperationsSlot->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
	
	if (!bIsShowTotalMoney && InvMoney)
		InvMoney->SetVisibility(ESlateVisibility::Collapsed);
	if (!bIsShowWeight && InvWeight)
		InvWeight->SetVisibility(ESlateVisibility::Collapsed);
}

void UInventoryContainerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//UE_LOG(LogTemp, Log, TEXT("FName %s"), *GetFName().ToString());

	if (TitleBar)
	{
		TitleBar->SetParentWidget(this);
		
		if (!bIsShowCloseButton)
			TitleBar->Button_Close->OnButtonClicked.AddDynamic(this, &UInventoryContainerWidget::CloseButtonClicked);
	}
}

void UInventoryContainerWidget::InitializeInventoryBindings()
{
	UUInventoryBaseWidget* InventoryWidget = GetInventoryWidgetFromContainerSlot();
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
			InventoryRef->OnWeightUpdatedDelegate.AddDynamic(this, &UInventoryContainerWidget::UpdateWeightInfo);
			InventoryRef->UpdateWeightInfo();
		}
	}

	if (OperationsSlot && OperationsSlot->GetChildrenCount() > 0)
	{
		if (auto OperationsWidget = Cast<UOperationPanelWidget>(OperationsSlot->GetChildAt(0)))
		{
			if (OperationsWidget->Button_TakeAll && OperationsWidget->Button_TakeAll->MainButton)
				OperationsWidget->Button_TakeAll->MainButton->OnClicked.AddDynamic(
					this, &UInventoryContainerWidget::TakeAll);
			if (OperationsWidget->Button_PlaceAll && OperationsWidget->Button_PlaceAll->MainButton)
				OperationsWidget->Button_PlaceAll->MainButton->OnClicked.AddDynamic(
					this, &UInventoryContainerWidget::PlaceAll);
			if (OperationsWidget->Button_Sort && OperationsWidget->Button_Sort->MainButton)
				OperationsWidget->Button_Sort->MainButton->OnClicked.AddDynamic(
					this, &UInventoryContainerWidget::SortItems);
		}
	}
	
	InventoryWidgetRef = InventoryWidget;

	Inventory->OnMoneyUpdatedDelegate.AddDynamic(this, &UInventoryContainerWidget::UpdateMoneyInfo);
	Inventory->UpdateMoneyInfo();
}

void UInventoryContainerWidget::ChangeInventoryInContainerSlot(TSubclassOf<UInvenzaBaseWidget> NewInventory)
{
	if (!NewInventory) return;

	auto NewInvWidget = CreateWidget<UUInventoryBaseWidget>(GetWorld(), NewInventory);
	if (!NewInvWidget) return;

	NewInvWidget->SetUISettings(GetInventoryWidgetFromContainerSlot()->GetUISettings());
	NewInvWidget->InitializeInventoryWidget();

	ContainerSlot->ClearChildren();
	ContainerSlot->AddChild(NewInvWidget);
	
	InventoryWidgetRef = NewInvWidget;
}

void UInventoryContainerWidget::CloseButtonClicked(UUIButton* Btn)
{
	OnClose.Broadcast(this);
}

UUInventoryBaseWidget* UInventoryContainerWidget::GetInventoryWidgetFromContainerSlot()
{
	if (!ContainerSlot || ContainerSlot->GetChildrenCount() == 0)
	{
		return nullptr;
	}

	if (UUInventoryBaseWidget* BaseInventoryWidget = Cast<UUInventoryBaseWidget>(ContainerSlot->GetChildAt(0)))
	{
		return BaseInventoryWidget;
	}

	return nullptr;
}

void UInventoryContainerWidget::ReDrawRequest()
{
	if (!InventoryRef || !InventoryWidgetRef)
	{
		return;
	}
	
	InventoryWidgetRef->ReDrawAllItems();
}

void UInventoryContainerWidget::UpdateWeightInfo(float InventoryTotalWeight)
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

void UInventoryContainerWidget::UpdateMoneyInfo(int32 TotalMoney)
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

void UInventoryContainerWidget::TakeAll()
{
	UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager)
		return;

	UInventoryBase* SourceInventory = GetInventoryWidgetFromContainerSlot()->GetInventoryRef();
	if (!SourceInventory)
		return;

	UInventoryBase* TargetInventory = InventoryManager->GetPawnMainInventory();
	if (!TargetInventory)
		return;

	InventoryManager->Execute_TransferItemArray(InventoryManager, SourceInventory, TargetInventory);
}

void UInventoryContainerWidget::PlaceAll()
{
	UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager)
		return;

	UInventoryBase* TargetInventory = GetInventoryWidgetFromContainerSlot()->GetInventoryRef();
	if (!TargetInventory)
		return;

	UInventoryBase* SourceInventory = InventoryManager->GetPawnMainInventory();
	if (!SourceInventory)
		return;

	InventoryManager->Execute_TransferItemArray(InventoryManager, SourceInventory, TargetInventory);
}

void UInventoryContainerWidget::SortItems()
{
	if (!InventoryRef)
		return;

	InventoryRef->GetItemCollectionLinked()->RequestSortInventory(InventoryRef->GetInventoryContainerID(), EInventorySortCriteria::ByName);
}
