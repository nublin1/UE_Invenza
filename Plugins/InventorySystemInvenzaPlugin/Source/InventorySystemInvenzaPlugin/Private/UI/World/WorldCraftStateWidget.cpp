// Nublin Studio 2026 All Rights Reserved.

#include "UI/World/WorldCraftStateWidget.h"

#include "ActorComponents/Crafting/CraftingComponent.h"
#include "Components/Border.h"

UWorldCraftStateWidget::UWorldCraftStateWidget() = default;

void UWorldCraftStateWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (WorldSurface)
	{
		WorldSurface->SetBrushColor(BackgroundColor);
		WorldSurface->SetPadding(ContentPadding);
	}
	if (WorldFrame)
	{
		WorldFrame->SetBrushColor(FrameColor);
	}
	SetProgressDisplaySettings(ProgressSettings);
}

void UWorldCraftStateWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindCraftComponent();
}

void UWorldCraftStateWidget::NativeDestruct()
{
	if (IsValid(CraftingComponentLink))
	{
		CraftingComponentLink->OnCurrentCraftDataChanged.RemoveDynamic(
			this, &UWorldCraftStateWidget::HandleCurrentCraftDataChanged);
	}
	Super::NativeDestruct();
}

void UWorldCraftStateWidget::SetProgressDisplaySettings(const FCraftProgressDisplaySettings& Settings)
{
	ProgressSettings = Settings;
	if (QueueCraftListEntryWidget)
	{
		QueueCraftListEntryWidget->SetProgressDisplaySettings(ProgressSettings);
	}
}

void UWorldCraftStateWidget::SetCraftComponentPtr(UCraftingComponent* NewCraftingComponent)
{
	if (IsValid(CraftingComponentLink))
	{
		CraftingComponentLink->OnCurrentCraftDataChanged.RemoveDynamic(
			this, &UWorldCraftStateWidget::HandleCurrentCraftDataChanged);
	}

	CraftingComponentLink = NewCraftingComponent;
	BindCraftComponent();
	SetProgressDisplaySettings(ProgressSettings);
}

void UWorldCraftStateWidget::BindCraftComponent()
{

	if (IsValid(CraftingComponentLink))
	{
		CraftingComponentLink->OnCurrentCraftDataChanged.AddUniqueDynamic(
			this, &UWorldCraftStateWidget::HandleCurrentCraftDataChanged);
		RefreshFromRecipe(CraftingComponentLink->GetCurrentCraftingRecipe());
	}
	else
	{
		RefreshFromRecipe(FQueuedRecipe());
	}
}

void UWorldCraftStateWidget::HandleCurrentCraftDataChanged(const FQueuedRecipe& Recipe)
{
	RefreshFromRecipe(Recipe);
}

void UWorldCraftStateWidget::RefreshFromRecipe(const FQueuedRecipe& Recipe)
{
	if (!QueueCraftListEntryWidget)
	{
		return;
	}

	QueueCraftListEntryWidget->UpdateData(Recipe);
	if (WorldFrame)
	{
		WorldFrame->SetVisibility(Recipe.ItemRecipeRow.ID.IsNone() ? ESlateVisibility::Collapsed
																   : ESlateVisibility::HitTestInvisible);
	}

	if (Recipe.ItemRecipeRow.ID.IsNone())
	{
		QueueCraftListEntryWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		QueueCraftListEntryWidget->SetVisibility(ESlateVisibility::Visible);
	}
}
