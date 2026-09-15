// Nublin Studio 2026 All Rights Reserved.


#include "UI/World/WorldCraftStateWidget.h"

#include "ActorComponents/Crafting/CraftingComponent.h"

void UWorldCraftStateWidget::SetCraftComponentPtr(UCraftingComponent* NewCraftingComponent)
{
	if (CraftingComponentLink)
	{
		CraftingComponentLink->OnCurrentCraftDataChanged.RemoveAll(this);
	}
	
	CraftingComponentLink = NewCraftingComponent;
	
	if (CraftingComponentLink)
	{
		CraftingComponentLink->OnCurrentCraftDataChanged.AddDynamic(this, &UWorldCraftStateWidget::HandleCurrentCraftDataChanged);
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
		return;
	
	QueueCraftListEntryWidget->UpdateData(Recipe);
	
	if (Recipe.ItemRecipeRow.ID.IsNone())
	{
		QueueCraftListEntryWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		QueueCraftListEntryWidget->SetVisibility(ESlateVisibility::Visible);
	}
}
