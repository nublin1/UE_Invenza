//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/SlotbasedInventorySlot.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "UI/Core/CoreCellWidget.h"

USlotbasedInventorySlot::USlotbasedInventorySlot()
{
}
void USlotbasedInventorySlot::NativeConstruct()
{
	Super::NativeConstruct();

	SetSlotNameText(SlotData.SlotName.ToString());

	if (DefaultCellImage)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(DefaultCellImage);
		CoreCellWidget->Content_Image->SetBrush(Brush);
	}
}

void USlotbasedInventorySlot::UpdateVisualWithTexture(UTexture2D* NewTexture)
{
	Super::UpdateVisualWithTexture(NewTexture);

	if (!NewTexture)
	{
		CoreCellWidget->Content_Image->SetBrush(FSlateBrush());
		return;
	}

	FSlateBrush Brush;
	Brush.SetResourceObject(NewTexture);
	CoreCellWidget->Content_Image->SetBrush(Brush);
}

void USlotbasedInventorySlot::ResetVisual()
{
	Super::ResetVisual();

	FSlateBrush Brush;
	Brush.SetResourceObject(DefaultCellImage);
	CoreCellWidget->Content_Image->SetBrush(Brush);
}

void USlotbasedInventorySlot::ClearVisual()
{
	Super::ClearVisual();
	FSlateBrush Brush = CoreCellWidget->Content_Image->GetBrush();
	Brush.SetResourceObject(nullptr);
	CoreCellWidget->Content_Image->SetBrush(Brush);
}

void USlotbasedInventorySlot::SetSlotNameText(FString SlotNameText)
{
	Super::SetSlotNameText(SlotNameText);
	if (SlotName)
	{
		if (SlotNameText.IsEmpty() || SlotNameText.Equals("None"))
		{
			SlotName->SetText(FText::GetEmpty());
			return;
		}
		
		SlotName->SetText(FText::FromString(SlotNameText));
		SlotName->SetVisibility(ESlateVisibility::Visible);
	}
}

