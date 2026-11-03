// Nublin Studio 2026 All Rights Reserved.

#include "UI/Core/Modal/InventoryTakeAmountHeaderWidget.h"

#include "ActorComponents/InventoryCollectionComponent.h"
#include "Components/SpinBox.h"
#include "Data/Inventory/InventoryBase.h"
#include "Interface/Interaction/ObjectDataProvider.h"
#include "UI/Core/LabelBaseText.h"

void UInventoryTakeAmountHeaderWidget::ConfigureHeader_Implementation(FModalHeaderData HeaderData)
{
	UnbindSource();
	SourceInventory = HeaderData.SourceInventory;
	SourceItem = HeaderData.ContextItem;
	if (TitleLabel)
	{
		TitleLabel->UpdateText(HeaderData.Title);
	}
	if (UInventoryBase* Inventory = SourceInventory.Get())
	{
		SourceCollection = Inventory->GetItemCollectionLinked();
		if (UInventoryCollectionComponent* Collection = SourceCollection.Get())
		{
			Collection->OnInventoryItemsChanged.AddUniqueDynamic(this, &ThisClass::HandleCollectionChanged);
		}
		Inventory->OnInventoryRedrawRequested.AddUniqueDynamic(this, &ThisClass::RefreshSource);
	}
	if (IObjectDataProvider* Provider = Cast<IObjectDataProvider>(SourceItem.Get()))
	{
		Provider->GetOnItemDataReplicatedDelegate().AddUniqueDynamic(this, &ThisClass::HandleSourceItemChanged);
		Provider->GetOnAmountChangedDelegate().AddUniqueDynamic(this, &ThisClass::HandleQuantityChanged);
	}
	if (AmountInput)
	{
		AmountInput->SetDelta(1.f);
		AmountInput->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleInputChanged);
	}
	RefreshSource();
}

void UInventoryTakeAmountHeaderWidget::NativeDestruct()
{
	UnbindSource();
	if (AmountInput)
	{
		AmountInput->OnValueChanged.RemoveAll(this);
	}
	Super::NativeDestruct();
}

void UInventoryTakeAmountHeaderWidget::UnbindSource()
{
	if (UInventoryCollectionComponent* Collection = SourceCollection.Get())
	{
		Collection->OnInventoryItemsChanged.RemoveAll(this);
	}
	if (UInventoryBase* Inventory = SourceInventory.Get())
	{
		Inventory->OnInventoryRedrawRequested.RemoveAll(this);
	}
	if (IObjectDataProvider* Provider = Cast<IObjectDataProvider>(SourceItem.Get()))
	{
		Provider->GetOnItemDataReplicatedDelegate().RemoveAll(this);
		Provider->GetOnAmountChangedDelegate().RemoveAll(this);
	}
	SourceCollection.Reset();
	SourceInventory.Reset();
	SourceItem.Reset();
}

void UInventoryTakeAmountHeaderWidget::RefreshSource()
{
	UObject* Item = SourceItem.Get();
	UInventoryBase* Inventory = SourceInventory.Get();
	UInventoryCollectionComponent* Collection = SourceCollection.Get();
	const bool bPresent = IsValid(Inventory) && IsValid(Collection) && IsValid(Item) &&
		Item->Implements<UObjectDataProvider>() && Collection->ItemHasInventory(Item, Inventory->GetInventoryContainerID());
	TotalQuantity = bPresent ? IObjectDataProvider::Execute_GetQuantity(Item) : 0;
	AvailableQuantity = bPresent ? FMath::Clamp(IObjectDataProvider::Execute_GetFreeAmount(Item), 0, TotalQuantity) : 0;
	UpdateLabels();
}

void UInventoryTakeAmountHeaderWidget::UpdateLabels()
{
	SelectedQuantity = AvailableQuantity > 0 ? FMath::Clamp(SelectedQuantity, 1, AvailableQuantity) : 0;
	if (AmountInput)
	{
		TGuardValue<bool> InputGuard(bUpdatingInput, true);
		AmountInput->SetMinValue(AvailableQuantity > 0 ? 1.f : 0.f);
		AmountInput->SetMinSliderValue(AvailableQuantity > 0 ? 1.f : 0.f);
		AmountInput->SetMaxValue(static_cast<float>(AvailableQuantity));
		AmountInput->SetMaxSliderValue(static_cast<float>(AvailableQuantity));
		AmountInput->SetValue(static_cast<float>(SelectedQuantity));
	}
	if (TakeLabel)
	{
		TakeLabel->UpdateText(FText::Format(TakeFormat, FText::AsNumber(SelectedQuantity)));
	}
	if (RemainingLabel)
	{
		RemainingLabel->UpdateText(FText::Format(RemainingFormat, FText::AsNumber(FMath::Max(0, TotalQuantity - SelectedQuantity))));
	}
	if (FeedbackLabel && AvailableQuantity <= 0)
	{
		FeedbackLabel->UpdateText(NoAvailableAmountText);
	}
}

void UInventoryTakeAmountHeaderWidget::HandleInputChanged(float Value)
{
	if (!bUpdatingInput && FMath::IsFinite(Value))
	{
		SelectedQuantity = static_cast<int32>(FMath::Clamp(FMath::RoundToDouble(static_cast<double>(Value)), 0.0, static_cast<double>(AvailableQuantity)));
		UpdateLabels();
	}
}

FModalHeaderResult UInventoryTakeAmountHeaderWidget::GetHeaderResult_Implementation()
{
	FModalHeaderResult Result;
	Result.SelectedAmount = static_cast<float>(SelectedQuantity);
	Result.SelectedQuantity = SelectedQuantity;
	Result.ExpectedSourceQuantity = TotalQuantity;
	Result.ExpectedAvailableAmount = AvailableQuantity;
	return Result;
}

void UInventoryTakeAmountHeaderWidget::SetServerFeedback(const FText& Message, int32 SourceQuantity, int32 AvailableAmount)
{
	TotalQuantity = FMath::Max(0, SourceQuantity);
	AvailableQuantity = FMath::Clamp(AvailableAmount, 0, TotalQuantity);
	UpdateLabels();
	if (FeedbackLabel)
	{
		FeedbackLabel->UpdateText(Message);
	}
}

void UInventoryTakeAmountHeaderWidget::SetSubmitting()
{
	if (FeedbackLabel)
	{
		FeedbackLabel->UpdateText(SubmittingText);
	}
}

void UInventoryTakeAmountHeaderWidget::HandleSourceItemChanged(UObject* Item)
{
	RefreshSource();
}

void UInventoryTakeAmountHeaderWidget::HandleQuantityChanged(int32 Amount, UObject* Item)
{
	RefreshSource();
}

void UInventoryTakeAmountHeaderWidget::HandleCollectionChanged(const FString& InventoryID)
{
	if (SourceInventory.IsValid() && SourceInventory->GetInventoryContainerID() == InventoryID)
	{
		RefreshSource();
	}
}
