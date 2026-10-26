// Nublin Studio 2026 All Rights Reserved.

#include "UI/Inventory/InventoryWorkspaceRowWidget.h"
#include "Components/Image.h"
#include "Components/NamedSlot.h"
#include "Components/Button.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"

void UInventoryWorkspaceRowWidget::ConfigureRow(const FText& Title, UInventoryContainerWidget* Inventory,
												bool bAllowCollapse, bool bExpanded)
{
	if (!TitleLabel || !ToggleButton || !InventoryContent || !Arrow)
	{
		return;
	}
	TitleLabel->UpdateText(Title);
	if (ToggleButton->MainButton)
	{
		ToggleButton->MainButton->SetVisibility(bAllowCollapse ? ESlateVisibility::Visible
															   : ESlateVisibility::SelfHitTestInvisible);
	}
	Arrow->SetVisibility(bAllowCollapse ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	InventoryContent->AddChild(Inventory);
	SetExpanded(bExpanded);
}

void UInventoryWorkspaceRowWidget::SetExpanded(bool bExpanded)
{
	if (Arrow)
	{
		Arrow->SetBrush(bExpanded ? ExpandedBrush : CollapsedBrush);
	}
}
