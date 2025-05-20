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

	SetItemUseKeyText(InUseKeyTextByDefault);

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

void USlotbasedInventorySlot::SetItemUseKeyText(FString InUseKeyText)
{
	Super::SetItemUseKeyText(InUseKeyText);
	if (ItemUseKey)
	{
		ItemUseKey->SetText(FText::FromString(InUseKeyText));
		ItemUseKey->SetVisibility(ESlateVisibility::Visible);
	}
}

