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

		if (CurrentMaxDisplay)
		{
			CurrentMaxDisplay->CurrentValue->UpdateText(FText::AsNumber(ProductionDetail->GetQueuedRecipeData().CurrentProgress));
			CurrentMaxDisplay->MaxValue->UpdateText(FText::AsNumber(ProductionDetail->GetQueuedRecipeData().ItemRecipeRow.CraftTime));
		}
	}
}

void UQueueCraftListEntryWidget::HandleDataChanged()
{
	if (CraftingQuantitySelectorMini)
	{
		CraftingQuantitySelectorMini->SetQuantity(QueueListEntryRef->GetQueuedRecipeData().Count);
	}
	
	if (CurrentMaxDisplay)
	{
		CurrentMaxDisplay->CurrentValue->UpdateText(FText::AsNumber(QueueListEntryRef->GetQueuedRecipeData().CurrentProgress));
		CurrentMaxDisplay->MaxValue->UpdateText(FText::AsNumber(QueueListEntryRef->GetQueuedRecipeData().ItemRecipeRow.CraftTime));
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