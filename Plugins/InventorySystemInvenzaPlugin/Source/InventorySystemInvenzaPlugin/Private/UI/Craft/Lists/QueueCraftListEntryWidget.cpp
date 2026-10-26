// Nublin Studio 2026 All Rights Reserved.

#include "UI/Craft/Lists/QueueCraftListEntryWidget.h"

#include "Data/CraftSystem/Entries/ProductionQueueListEntryObject.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/Image/ImageBaseWidget.h"
#include "UI/Core/Progress/CurrentMaxDisplay.h"
#include "UI/Core/Progress/CraftProgressDisplay.h"
#include "UI/Craft/CraftingQuantitySelector.h"

UQueueCraftListEntryWidget::UQueueCraftListEntryWidget() : QueueListEntryRef(nullptr)
{
	// Existing queue presentations retain their numeric display until opted in.
	ProgressSettings.bUseProgressBar = false;
	ProgressSettings.bShowPercentage = false;
}

void UQueueCraftListEntryWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	RefreshCraftProgress();
}

void UQueueCraftListEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_QueueUp)
	{
		Btn_QueueUp->OnButtonClicked.AddUniqueDynamic(this, &UQueueCraftListEntryWidget::OnBtnUpClicked);
	}
	if (Btn_QueueDown)
	{
		Btn_QueueDown->OnButtonClicked.AddUniqueDynamic(this, &UQueueCraftListEntryWidget::OnBtnDownClicked);
	}
	if (Btn_QueueDelete)
	{
		Btn_QueueDelete->OnButtonClicked.AddUniqueDynamic(this, &UQueueCraftListEntryWidget::OnBtnDeleteClicked);
	}
	if (IsValid(QueueListEntryRef))
	{
		QueueListEntryRef->OnDataChanged.AddUniqueDynamic(this, &UQueueCraftListEntryWidget::HandleDataChanged);
		UpdateQueueData(QueueListEntryRef);
	}
}

void UQueueCraftListEntryWidget::NativeDestruct()
{
	if (Btn_QueueUp)
	{
		Btn_QueueUp->OnButtonClicked.RemoveDynamic(this, &UQueueCraftListEntryWidget::OnBtnUpClicked);
	}
	if (Btn_QueueDown)
	{
		Btn_QueueDown->OnButtonClicked.RemoveDynamic(this, &UQueueCraftListEntryWidget::OnBtnDownClicked);
	}
	if (Btn_QueueDelete)
	{
		Btn_QueueDelete->OnButtonClicked.RemoveDynamic(this, &UQueueCraftListEntryWidget::OnBtnDeleteClicked);
	}
	if (IsValid(QueueListEntryRef))
	{
		QueueListEntryRef->OnDataChanged.RemoveDynamic(this, &UQueueCraftListEntryWidget::HandleDataChanged);
	}
	Super::NativeDestruct();
}

void UQueueCraftListEntryWidget::UnbindQueueData()
{
	if (IsValid(QueueListEntryRef))
	{
		QueueListEntryRef->OnDataChanged.RemoveDynamic(this, &UQueueCraftListEntryWidget::HandleDataChanged);
	}
	QueueListEntryRef = nullptr;
}

void UQueueCraftListEntryWidget::NativeOnEntryReleased()
{
	UnbindQueueData();
	IUserObjectListEntry::NativeOnEntryReleased();
}

void UQueueCraftListEntryWidget::NativeOnListItemObjectSet(UObject* DetailItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(DetailItemObject);
	UnbindQueueData();

	if (auto* ProductionDetail = Cast<UProductionQueueListEntryObject>(DetailItemObject))
	{
		QueueListEntryRef = ProductionDetail;
		QueueListEntryRef->OnDataChanged.AddUniqueDynamic(this, &UQueueCraftListEntryWidget::HandleDataChanged);
		UpdateQueueData(ProductionDetail);
	}
}

void UQueueCraftListEntryWidget::UpdateData(const FQueuedRecipe& NewData)
{
	DisplayedRecipe = NewData;
	if (UTexture2D* Icon = NewData.ItemRecipeRow.RecipeIcon.Get())
	{
		if (QueueIcon)
		{
			QueueIcon->UpdateImage(Icon);
		}
	}

	if (QueueItemName)
	{
		QueueItemName->UpdateText(NewData.ItemRecipeRow.DisplayName);
	}
	if (CraftingQuantitySelectorMini)
	{
		CraftingQuantitySelectorMini->SetQuantity(NewData.Count);
	}
	if (RemainingCount && RemainingCount->CurrentValue)
	{
		RemainingCount->CurrentValue->UpdateText(FText::AsNumber(NewData.Count));
	}
	RefreshCraftProgress();
}

void UQueueCraftListEntryWidget::SetProgressDisplaySettings(const FCraftProgressDisplaySettings& Settings)
{
	ProgressSettings = Settings;
	RefreshCraftProgress();
}

void UQueueCraftListEntryWidget::RefreshCraftProgress()
{
	if (UCraftProgressDisplay* Display = Cast<UCraftProgressDisplay>(CraftProgress))
	{
		Display->ConfigureDisplay(ProgressSettings);
		Display->SetProgress(DisplayedRecipe.CurrentProgress, DisplayedRecipe.ItemRecipeRow.CraftVolume);
	}
	else if (CraftProgress)
	{
		if (CraftProgress->CurrentValue)
		{
			CraftProgress->CurrentValue->UpdateText(FText::AsNumber(DisplayedRecipe.CurrentProgress));
		}
		if (CraftProgress->MaxValue)
		{
			CraftProgress->MaxValue->UpdateText(FText::AsNumber(DisplayedRecipe.ItemRecipeRow.CraftVolume));
		}
	}
}

void UQueueCraftListEntryWidget::UpdateQueueData(UProductionQueueListEntryObject* ProductionDetail)
{
	if (!ProductionDetail)
	{
		return;
	}
	UpdateData(ProductionDetail->GetQueuedRecipeData());
}

void UQueueCraftListEntryWidget::HandleDataChanged()
{
	UpdateQueueData(QueueListEntryRef);
}

void UQueueCraftListEntryWidget::OnBtnUpClicked(UUIButton* Btn)
{
	if (QueueListEntryRef)
	{
		OnMoveRequested.Broadcast(QueueListEntryRef, true);
	}
}

void UQueueCraftListEntryWidget::OnBtnDownClicked(UUIButton* Btn)
{
	if (QueueListEntryRef)
	{
		OnMoveRequested.Broadcast(QueueListEntryRef, false);
	}
}

void UQueueCraftListEntryWidget::OnBtnDeleteClicked(UUIButton* Btn)
{
	if (QueueListEntryRef)
	{
		OnDeleteRequested.Broadcast(QueueListEntryRef);
	}
}
