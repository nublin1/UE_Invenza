//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/SlotbasedInventorySlot.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "TimerManager.h"
#include "UI/Core/CoreCellWidget.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Core/Image/ImageBaseWidget.h"

USlotbasedInventorySlot::USlotbasedInventorySlot()
{
}

void USlotbasedInventorySlot::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (SlotData)
	{
		SetSlotNameText(SlotData->InventorySlotInfo.SlotName.ToString());
	}
	ResetVisual();
}

void USlotbasedInventorySlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void USlotbasedInventorySlot::UpdateVisualWithTexture(UTexture2D* NewTexture)
{
	Super::UpdateVisualWithTexture(NewTexture);

	if (!CoreCellWidget || !CoreCellWidget->Content_Image) return;

	if (!NewTexture)
	{
		ClearVisual();
		return;
	}
	
	CoreCellWidget->Content_Image->UpdateImage(NewTexture);
}

void USlotbasedInventorySlot::ResetVisual()
{
	Super::ResetVisual();

	if (!CoreCellWidget || !CoreCellWidget->Content_Image) return;
	if (DefaultCellImage)
	{
		CoreCellWidget->Content_Image->UpdateImage(DefaultCellImage);
	}
	else 
	{
		ClearVisual();
	}
}

void USlotbasedInventorySlot::ClearVisual()
{
	Super::ClearVisual();
	if (!CoreCellWidget || !CoreCellWidget->Content_Image) return;
	
	CoreCellWidget->Content_Image->UpdateImage(nullptr);
}

FVector2D USlotbasedInventorySlot::GetSlotSize()
{
	return CoreCellWidget->GetCurrentSlotSize();
}

void USlotbasedInventorySlot::SetSlotNameText(FString SlotNameText)
{
	Super::SetSlotNameText(SlotNameText);
	if (SlotName)
	{
		if (SlotNameText.IsEmpty() || SlotNameText.Equals("None"))
		{
			SlotName->UpdateText(FText::GetEmpty());
			return;
		}
		
		SlotName->UpdateText(FText::FromString(SlotNameText));
		SlotName->SetVisibility(ESlateVisibility::Visible);
	}
}

