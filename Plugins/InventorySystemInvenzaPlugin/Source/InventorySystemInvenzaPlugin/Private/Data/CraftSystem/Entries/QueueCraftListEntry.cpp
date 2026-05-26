// Nublin Studio 2026 All Rights Reserved.

#include "Data/CraftSystem/Entries/QueueCraftListEntry.h"

#include "Data/CraftSystem/Entries/ProductionQueueListEntryObject.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/Image/ImageBaseWidget.h"
#include "UI/Core/Progress/CurrentMaxDisplay.h"
#include "UI/Craft/CraftingQuantitySelector.h"


UQueueCraftListEntry::UQueueCraftListEntry(): QueueListEntryRef(nullptr)
{
}

void UQueueCraftListEntry::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_QueueUp)
	{
		Btn_QueueUp->OnButtonClicked.AddDynamic(this,
			&UQueueCraftListEntry::OnBtnUpClicked);
	}
	if (Btn_QueueDown)
	{
		Btn_QueueDown->OnButtonClicked.AddDynamic(this,
			&UQueueCraftListEntry::OnBtnDownClicked);
	}
}

void UQueueCraftListEntry::NativeOnListItemObjectSet(UObject* DetailItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(DetailItemObject);

	if (auto ProductionDetail = Cast<UProductionQueueListEntryObject>(DetailItemObject))
	{
		QueueListEntryRef = ProductionDetail;

		UpdateQueueImage(ProductionDetail->RecipeRow.RecipeIcon);

		QueueItemName->UpdateText(ProductionDetail->RecipeRow.DisplayName);

		if (CraftingQuantitySelectorMini)
		{
			CraftingQuantitySelectorMini->SetQuantity(ProductionDetail->AmountInQueue);
		}

		if (CurrentMaxDisplay)
		{
			CurrentMaxDisplay->CurrentValue->UpdateText(FText::AsNumber(ProductionDetail->CurrentProgress));
			CurrentMaxDisplay->MaxValue->UpdateText(FText::AsNumber(ProductionDetail->RecipeRow.CraftTime));
		}
	}
}

void UQueueCraftListEntry::UpdateQueueImage(const TSoftObjectPtr<UTexture2D>& NewQueueIcon)
{
	if (!QueueIcon || NewQueueIcon.IsNull())
		return;

	UTexture2D* LoadedTexture = NewQueueIcon.LoadSynchronous();
	if (!LoadedTexture)
		return;

	FSlateBrush NewBrush;
	NewBrush.SetResourceObject(LoadedTexture);
	NewBrush.ImageSize = FVector2D(LoadedTexture->GetSizeX(), LoadedTexture->GetSizeY());
	QueueIcon->UpdateBrush(NewBrush);
}

void UQueueCraftListEntry::OnBtnUpClicked(UUIButton* Btn)
{
	if (QueueListEntryRef)
	{
		OnMoveRequested.Broadcast(QueueListEntryRef, true);
	}
}

void UQueueCraftListEntry::OnBtnDownClicked(UUIButton* Btn)
{
	if (QueueListEntryRef)
	{
		OnMoveRequested.Broadcast(QueueListEntryRef, false);
	}
}
