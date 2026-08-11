// Nublin Studio 2025 All Rights Reserved.

#include "UI/Item/InventoryItemWidget.h"

#include "Data/Items/ItemBase.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "UI/Core/CoreCellWidget.h"
#include "UI/Core/Image/ImageBaseWidget.h"
#include "Utility/InterfaceUtils.h"

UInventoryItemWidget::UInventoryItemWidget()
{
}

void UInventoryItemWidget::UpdateItemVisual( UObject* Item,EItemOrientationType Orientation, FVector2D TotalSize,
	FVector2D Position, bool bIgnoreSize)
{
	if (!Item || !UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("UpdateItemVisual")))
		return;

	FIntPoint ItemSize = bIgnoreSize
		? FIntPoint(1, 1)
		: IObjectDataProvider::Execute_GetItemSize(Item, Orientation);

	float RotationAngle = 0.f;
	if (ItemSize.X != ItemSize.Y)
	{
		RotationAngle =
			(IObjectDataProvider::Execute_GetInitialItemOrientation(Item) == Orientation)
			? 0.f
			: -90.f;
	}

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(this->Slot))
	{
		CanvasSlot->SetAutoSize(false);
		CanvasSlot->SetSize(FVector2D(TotalSize.X, TotalSize.Y));
		CanvasSlot->SetPosition(Position);
	}

	UpdateVisualSize(TotalSize);
	UpdateVisual(Item, RotationAngle);

	/*UE_LOG(LogTemp, Log, TEXT("Rotate: Size=(%d,%d) TotalSize=(%.1f,%.1f) Angle=%.1f Orientation=%s"),
		ItemSize.X, ItemSize.Y, TotalSize.X, TotalSize.Y, RotationAngle,
		Orientation == EItemOrientationType::Horizontal ? TEXT("Horizontal") : TEXT("Vertical"));*/
}

void UInventoryItemWidget::UpdateVisual(UObject* Item,float AngleDegrees)
{
	if (!Item ||
		!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("UpdateVisual")) ||
		!CoreCellWidget ||
		!CoreCellWidget->Content_Image)
		return;

	UTexture2D* Icon =
		IObjectDataProvider::Execute_GetItemRef(Item).ItemAssetData.Icon;

	if (!Icon)
		return;

	if (!IconMaterial)
	{
		CoreCellWidget->SetContentImage(Icon);
		return;
	}

	UImageBaseWidget* ImageWidget = CoreCellWidget->Content_Image;
	ImageWidget->SetNewMaterial(IconMaterial);

	ImageWidget->SetMaterialTextureParam(FName("IconTexture"), Icon);

	const float NormalizedAngle = FMath::Fmod(AngleDegrees, 360.f) / 360.f;
	ImageWidget->SetMaterialScalarParam(FName("RotationAngle"), NormalizedAngle);
}

void UInventoryItemWidget::ClearVisual()
{
	if (!CoreCellWidget || !CoreCellWidget->Content_Image) return;
	
	CoreCellWidget->Content_Image->UpdateImage(nullptr);
}

void UInventoryItemWidget::UpdateVisualSize(FVector2D ItemTotalSize) const
{
	CoreCellWidget->SizeBox->SetWidthOverride(ItemTotalSize.X);
	CoreCellWidget->SizeBox->SetHeightOverride(ItemTotalSize.Y);
	
	if (!SizeBoxText)
		return;
	
	SizeBoxText->SetWidthOverride(ItemTotalSize.X);
	SizeBoxText->SetHeightOverride(ItemTotalSize.Y);

	CoreCellWidget->Content_ImageSizeBox->SetWidthOverride(ItemTotalSize.X);
	CoreCellWidget->Content_ImageSizeBox->SetHeightOverride(ItemTotalSize.Y);

	CoreCellWidget->Content_ImageSizeBox->InvalidateLayoutAndVolatility();
}

void UInventoryItemWidget::UpdateItemName(FText Name)
{
	if (!ItemName)
		return;

	ItemName->SetText(Name);
}

void UInventoryItemWidget::UpdateQuantityText(int Quantity)
{
	if (!ItemQuantity)
		return;
	
	if (Quantity > 1)
	{
		ItemQuantity->SetText(FText::AsNumber(Quantity));
		ItemQuantity->SetVisibility(ESlateVisibility::Visible);
	}
	else
		ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);
}

void UInventoryItemWidget::ChangeBorderColor(FLinearColor NewColor) const
{
	CoreCellWidget->Left_Border->SetBrushColor(NewColor);
	CoreCellWidget->Right_Border->SetBrushColor(NewColor);
	CoreCellWidget->Top_Border->SetBrushColor(NewColor);
	CoreCellWidget->BottomBorder->SetBrushColor(NewColor);
}

void UInventoryItemWidget::ChangeOpacity(float NewValue)
{
	if (CoreCellWidget && CoreCellWidget->Content_Image)
	{
		if (IconMaterial)
		{
			CoreCellWidget->Content_Image->SetMaterialScalarParam(FName("ItemOpacity"), NewValue);
		}
		else
		{
			CoreCellWidget->Content_Image->SetRenderOpacity(NewValue);
		}
	}
}

FReply UInventoryItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return Reply.Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return FReply::Unhandled();
}

void UInventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
}
