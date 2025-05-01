// Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/EquipmentSlotWidget.h"

#include "Components/Image.h"
#include "UI/Core/CoreCellWidget.h"

void UEquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CellImage)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(CellImage);
		CoreCellWidget->Content_Image->SetBrush(Brush);
	}
}
