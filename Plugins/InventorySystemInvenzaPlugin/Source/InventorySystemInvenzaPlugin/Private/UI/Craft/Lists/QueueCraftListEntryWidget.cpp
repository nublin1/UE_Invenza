// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/Lists/QueueCraftListEntryWidget.h"

#include "Data/CraftSystem/Entries/ProductionQueueListEntryObject.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/Image/ImageBaseWidget.h"
#include "UI/Core/Progress/CurrentMaxDisplay.h"
#include "UI/Craft/CraftingQuantitySelector.h"


UQueueCraftListEntryWidget::UQueueCraftListEntryWidget(): QueueListEntryRef(nullptr)
{
}

void UQueueCraftListEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_QueueUp)
	{
		Btn_QueueUp->OnButtonClicked.AddDynamic(this,
			&UQueueCraftListEntryWidget::OnBtnUpClicked);
	}
	if (Btn_QueueDown)
	{
		Btn_QueueDown->OnButtonClicked.AddDynamic(this,
			&UQueueCraftListEntryWidget::OnBtnDownClicked);
	}

	if (Btn_QueueDelete)
	{
		Btn_QueueDelete->OnButtonClicked.AddDynamic(this, &UQueueCraftListEntryWidget::OnBtnDeleteClicked);
	}
}

void UQueueCraftListEntryWidget::NativeOnListItemObjectSet(UObject* DetailItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(DetailItemObject);

	if (auto ProductionDetail = Cast<UProductionQueueListEntryObject>(DetailItemObject))
	{
		QueueListEntryRef = ProductionDetail;

		QueueListEntryRef->OnDataChanged.AddDynamic(this, &UQueueCraftListEntryWidget::HandleDataChanged);
		
		if (UTexture2D* Icon = ProductionDetail->GetQueuedRecipeData().ItemRecipeRow.RecipeIcon.Get())
		{
			if (QueueIcon)
				QueueIcon->UpdateImage(Icon);
		}

		QueueItemName->UpdateText(ProductionDetail->GetQueuedRecipeData().ItemRecipeRow.DisplayName);

		if (CraftingQuantitySelectorMini)
		{
			CraftingQuantitySelectorMini->SetQuantity(ProductionDetail->GetQueuedRecipeData().Count);
		}
		
		if (RemainingCount)
		{
			RemainingCount->CurrentValue->UpdateText(FText::AsNumber(ProductionDetail->GetQueuedRecipeData().Count));
		}

		if (CraftProgress)
		{
			CraftProgress->CurrentValue->UpdateText(FText::AsNumber(ProductionDetail->GetQueuedRecipeData().CurrentProgress));
			CraftProgress->MaxValue->UpdateText(FText::AsNumber(ProductionDetail->GetQueuedRecipeData().ItemRecipeRow.CraftVolume));
		}
	}
}

void UQueueCraftListEntryWidget::HandleDataChanged()
{
	if (CraftingQuantitySelectorMini)
	{
		CraftingQuantitySelectorMini->SetQuantity(QueueListEntryRef->GetQueuedRecipeData().Count);
	}
	
	if (RemainingCount)
	{
		RemainingCount->CurrentValue->UpdateText(FText::AsNumber(QueueListEntryRef->GetQueuedRecipeData().Count));
	}
	
	if (CraftProgress)
	{
		CraftProgress->CurrentValue->UpdateText(FText::AsNumber(QueueListEntryRef->GetQueuedRecipeData().CurrentProgress));
		CraftProgress->MaxValue->UpdateText(FText::AsNumber(QueueListEntryRef->GetQueuedRecipeData().ItemRecipeRow.CraftVolume));
	}
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
		OnDeleteRequested.Broadcast(QueueListEntryRef);
}
