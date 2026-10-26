//  Nublin Studio 2025 All Rights Reserved.

#include "UI/Inventory/Container/InventoryContainerWidget.h"

#include "ActorComponents/InventoryCollectionComponent.h"
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
		UpdateTitle(Title);
	}
	UpdateCloseButtonVisibility();

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
	{
		InvMoney->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (!bIsShowWeight && InvWeight)
	{
		InvWeight->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInventoryContainerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//UE_LOG(LogTemp, Log, TEXT("FName %s"), *GetFName().ToString());

	if (TitleBar)
	{
		TitleBar->SetParentWidget(this);
	}
	UpdateCloseButtonVisibility();
}

void UInventoryContainerWidget::InitializeInventoryContainer()
{
	UUInventoryBaseWidget* InventoryWidget = GetInventoryWidgetFromContainerSlot();
	if (!InventoryWidget)
	{
		return;
	}

	UInventoryBase* Inventory = InventoryWidget->GetInventoryRef();
	if (!Inventory)
	{
		return;
	}

	if (IsValid(InventoryRef) && InventoryRef != Inventory)
	{
		InventoryRef->OnWeightUpdatedDelegate.RemoveAll(this);
		InventoryRef->OnMoneyUpdatedDelegate.RemoveAll(this);
	}
	InventoryRef = Inventory;
	InventoryWidgetRef = InventoryWidget;

	InitializeTitleBar();
	InitializeInventoryInfo();
	InitializeOperations();
}

void UInventoryContainerWidget::ChangeInventoryInContainerSlot(TSubclassOf<UInvenzaBaseWidget> NewInventory)
{
	if (!NewInventory)
	{
		return;
	}

	auto NewInvWidget = CreateWidget<UUInventoryBaseWidget>(GetWorld(), NewInventory);
	if (!NewInvWidget)
	{
		return;
	}

	NewInvWidget->SetUISettings(GetInventoryWidgetFromContainerSlot()->GetUISettings());
	NewInvWidget->InitializeInventoryWidget();

	ContainerSlot->ClearChildren();
	ContainerSlot->AddChild(NewInvWidget);

	InventoryWidgetRef = NewInvWidget;
}

void UInventoryContainerWidget::CloseButtonClicked(UUIButton* Btn)
{
	if (!bIsShowCloseButton || (IsValid(InventoryRef) && !InventoryRef->GetInventorySettings().bIsClosable))
	{
		return;
	}
	OnClose.Broadcast(this);
}

void UInventoryContainerWidget::SetShowCloseButton(bool bShow)
{
	bIsShowCloseButton = bShow;
	UpdateCloseButtonVisibility();
}

void UInventoryContainerWidget::UpdateCloseButtonVisibility()
{
	if (!TitleBar || !TitleBar->Button_Close)
	{
		return;
	}
	auto* Button = TitleBar->Button_Close.Get();
	const bool bShow =
		bIsShowCloseButton && (!IsValid(InventoryRef) || InventoryRef->GetInventorySettings().bIsClosable);
	Button->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	Button->OnButtonClicked.RemoveDynamic(this, &UInventoryContainerWidget::CloseButtonClicked);
	if (bShow)
	{
		Button->OnButtonClicked.AddUniqueDynamic(this, &UInventoryContainerWidget::CloseButtonClicked);
	}
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

void UInventoryContainerWidget::ReleaseInventory()
{
	if (IsValid(InventoryRef))
	{
		InventoryRef->OnWeightUpdatedDelegate.RemoveAll(this);
		InventoryRef->OnMoneyUpdatedDelegate.RemoveAll(this);
	}
	if (auto* InventoryWidget = GetInventoryWidgetFromContainerSlot())
	{
		InventoryWidget->SetInventoryBaseRef(nullptr);
	}
	InventoryRef = nullptr;
	InventoryWidgetRef = nullptr;
}

void UInventoryContainerWidget::UpdateWeightInfo(float InventoryTotalWeight)
{
	if (bIsShowWeight)
	{
		FString RoundedString = FString::Printf(TEXT("%0.1f"), InventoryTotalWeight);
		FString Text = {" " + RoundedString + "/" +
						FString::SanitizeFloat(InventoryRef->GetInventorySettings().InventoryMaxWeightCapacity)};
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
	{
		return;
	}

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
	{
		return;
	}

	UInventoryBase* SourceInventory = GetInventoryWidgetFromContainerSlot()->GetInventoryRef();
	if (!SourceInventory)
	{
		return;
	}

	UInventoryBase* TargetInventory = InventoryManager->GetPawnMainInventory();
	if (!TargetInventory)
	{
		return;
	}

	InventoryManager->Execute_TransferItemArray(InventoryManager, SourceInventory, TargetInventory);
}

void UInventoryContainerWidget::PlaceAll()
{
	UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager)
	{
		return;
	}

	UInventoryBase* TargetInventory = GetInventoryWidgetFromContainerSlot()->GetInventoryRef();
	if (!TargetInventory)
	{
		return;
	}

	UInventoryBase* SourceInventory = InventoryManager->GetPawnMainInventory();
	if (!SourceInventory)
	{
		return;
	}

	InventoryManager->Execute_TransferItemArray(InventoryManager, SourceInventory, TargetInventory);
}

void UInventoryContainerWidget::SortItems()
{
	if (!InventoryRef)
	{
		return;
	}

	if (auto* Pawn = GetOwningPlayerPawn())
	{
		if (auto* Collection = Pawn->FindComponentByClass<UInventoryCollectionComponent>())
		{
			Collection->RequestSortInventory(InventoryRef->GetInventoryContainerID(), EInventorySortCriteria::ByName);
		}
	}
}

void UInventoryContainerWidget::PlaceMatching()
{
	if (APawn* Pawn = GetOwningPlayerPawn())
	{
		if (UIInventoryManager* Manager = Pawn->FindComponentByClass<UIInventoryManager>())
		{
			Manager->TransferMatchingItems(Manager->GetPawnMainInventory(), InventoryRef);
		}
	}
}

void UInventoryContainerWidget::InitializeTitleBar()
{
	if (!IsValid(InventoryRef))
	{
		return;
	}
	const auto& InvSettings = InventoryRef->GetInventorySettings();

	UpdateTitle(InvSettings.ContainerTitle);

	UpdateCloseButtonVisibility();
}

void UInventoryContainerWidget::InitializeInventoryInfo()
{
	if (!InventoryRef)
	{
		return;
	}

	const auto& InvSettings = InventoryRef->GetInventorySettings();

	if (InvWeight)
	{
		if (!bIsShowWeight || !InvSettings.bShowTotalWeight || InvSettings.InventoryMaxWeightCapacity < 0)
		{
			InvWeight->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			InvWeight->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

			InventoryRef->OnWeightUpdatedDelegate.AddUniqueDynamic(this, &UInventoryContainerWidget::UpdateWeightInfo);

			InventoryRef->UpdateWeightInfo();
		}
	}

	if (InvMoney)
	{
		if (!bIsShowTotalMoney || !InvSettings.bShowTotalMoney)
		{
			InvMoney->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			InvMoney->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

			InventoryRef->OnMoneyUpdatedDelegate.AddUniqueDynamic(this, &UInventoryContainerWidget::UpdateMoneyInfo);

			InventoryRef->UpdateMoneyInfo();
		}
	}
}

void UInventoryContainerWidget::InitializeOperations()
{
	if (!OperationsSlot || OperationsSlot->GetChildrenCount() == 0)
	{
		return;
	}

	UOperationPanelWidget* OperationsWidget = Cast<UOperationPanelWidget>(OperationsSlot->GetChildAt(0));
	if (!OperationsWidget)
	{
		return;
	}

	if (OperationsWidget->Button_PlaceMatching)
	{
		APawn* Pawn = GetOwningPlayerPawn();
		UIInventoryManager* Manager = Pawn ? Pawn->FindComponentByClass<UIInventoryManager>() : nullptr;
		const bool bAvailable =
			Manager && Manager->CanTransferMatchingItems(Manager->GetPawnMainInventory(), InventoryRef);
		OperationsWidget->Button_PlaceMatching->SetVisibility(bAvailable ? ESlateVisibility::Visible
																		 : ESlateVisibility::Collapsed);
		if (OperationsWidget->Button_PlaceMatching->MainButton)
		{
			OperationsWidget->Button_PlaceMatching->MainButton->OnClicked.AddUniqueDynamic(
				this, &UInventoryContainerWidget::PlaceMatching);
		}
	}

	if (OperationsWidget->Button_TakeAll && OperationsWidget->Button_TakeAll->MainButton)
	{
		OperationsWidget->Button_TakeAll->MainButton->OnClicked.AddUniqueDynamic(this,
																				 &UInventoryContainerWidget::TakeAll);
	}

	if (OperationsWidget->Button_PlaceAll && OperationsWidget->Button_PlaceAll->MainButton)
	{
		OperationsWidget->Button_PlaceAll->MainButton->OnClicked.AddUniqueDynamic(this,
																				  &UInventoryContainerWidget::PlaceAll);
	}

	if (OperationsWidget->Button_Sort && OperationsWidget->Button_Sort->MainButton)
	{
		OperationsWidget->Button_Sort->MainButton->OnClicked.AddUniqueDynamic(this,
																			  &UInventoryContainerWidget::SortItems);
	}
}

void UInventoryContainerWidget::UpdateTitle(FText NewText)
{
	Title = NewText;

	if (TitleBar)
	{
		TitleBar->TitleName->UpdateText(Title);

		if (!bIsShowCloseButton && TitleBar->HeaderCanvasPanel)
		{
			TitleBar->Button_Close->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
