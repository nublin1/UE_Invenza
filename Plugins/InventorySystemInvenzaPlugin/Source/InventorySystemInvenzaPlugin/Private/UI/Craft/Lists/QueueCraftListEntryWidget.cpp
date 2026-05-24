// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/Lists/QueueCraftListEntryWidget.h"

#include "Data/CraftSystem/Entries/ProductionQueueListEntryObject.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/Image/ImageBaseWidget.h"
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
}

void UQueueCraftListEntryWidget::NativeOnListItemObjectSet(UObject* DetailItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(DetailItemObject);

	if (auto ProductionDetail = Cast<UProductionQueueListEntryObject>(DetailItemObject))
	{
		QueueListEntryRef = ProductionDetail;
		
		if (UTexture2D* Icon = ProductionDetail->RecipeRow.RecipeIcon.Get())
		{
			QueueIcon->UpdateImage(Icon);
		}

		QueueItemName->UpdateText(ProductionDetail->RecipeRow.DisplayName);

		if (CraftingQuantitySelectorMini)
		{
			CraftingQuantitySelectorMini->SetQuantity(ProductionDetail->AmountInQueue);
		}

		if (CurrentProgressAmount)
		{
			CurrentProgressAmount->UpdateText(FText::AsNumber(ProductionDetail->CurrentProgress));
		}
		if (MaxProgressAmount)
		{
			MaxProgressAmount->UpdateText(FText::AsNumber(ProductionDetail->RecipeRow.CraftTime));
		}
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