// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/CraftDashboard.h"

#include "Components/ListView.h"
#include "Components/NamedSlot.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBoxSlot.h"
#include "ActorComponents/Crafting/CraftingComponent.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Core/List/SimpleUserObjectListEntry.h"
#include "UI/Craft/CraftControlPanel.h"
#include "UI/Craft/CraftMenuChoose.h"
#include "UI/Craft/Lists/QueueCraftList.h"

UCraftDashboard::UCraftDashboard()
{
}

void UCraftDashboard::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UCraftDashboard::NativeConstruct()
{
	Super::NativeConstruct();

	if (CraftControlPanel)
	{
		for (auto Btn : CraftControlPanel->Execute_GetButtons(CraftControlPanel))
		{
			if (!IsValid(Btn)) continue;
			if (Btn->GetBtnTag() == AddTaskBtnTag)
				Btn->OnButtonClicked.AddUniqueDynamic(this, &UCraftDashboard::AddTaskBtnPressed);
			
			if (Btn->GetBtnTag() == PauseBtnTag)
				Btn->OnButtonClicked.AddUniqueDynamic(this, &UCraftDashboard::PauseBtnPressed);
		}
	}

	if (QueueCraftList)
	{
		QueueCraftList->OnQueueOrderChangeRequested.AddUniqueDynamic(this, &UCraftDashboard::HandleQueueOrderChangeRequested);
		QueueCraftList->OnQueueItemDeleteRequested.AddUniqueDynamic(this, &UCraftDashboard::HandleQueueItemDeleteRequested);
	}
	InitializeCraftComponentBindings();
	RefreshCraftState();
}

void UCraftDashboard::NativeDestruct()
{
	UnbindCraftComponent();
	if (CraftControlPanel)
	{
		for (UUIButton* Button : CraftControlPanel->Execute_GetButtons(CraftControlPanel))
		{
			if (IsValid(Button)) Button->OnButtonClicked.RemoveAll(this);
		}
	}
	if (QueueCraftList)
	{
		QueueCraftList->OnQueueOrderChangeRequested.RemoveAll(this);
		QueueCraftList->OnQueueItemDeleteRequested.RemoveAll(this);
	}
	Super::NativeDestruct();
}

void UCraftDashboard::InitializeCraftComponentBindings()
{
	if (!CraftComponentPtr)
		return;
	
	CraftComponentPtr->OnCurrentCraftDataChanged.AddUniqueDynamic(this, &UCraftDashboard::UpdateCurrentCraftProgress);
	CraftComponentPtr->OnCraftQueueChanged.AddUniqueDynamic(this, &UCraftDashboard::UpdateQueueCraftList);
	CraftComponentPtr->OnBlocksUpdated.AddUniqueDynamic(this, &UCraftDashboard::HandleBlocksUpdated);
}

void UCraftDashboard::UnbindCraftComponent()
{
	if (IsValid(CraftComponentPtr))
	{
		CraftComponentPtr->OnCurrentCraftDataChanged.RemoveAll(this);
		CraftComponentPtr->OnCraftQueueChanged.RemoveAll(this);
		CraftComponentPtr->OnBlocksUpdated.RemoveAll(this);
	}
}

void UCraftDashboard::SetCraftComponentPtr(UCraftingComponent* NewCraftingComponent)
{
	UnbindCraftComponent();
	CraftComponentPtr = NewCraftingComponent;
	InitializeCraftComponentBindings();
	RefreshCraftState();
}

void UCraftDashboard::RefreshCraftState()
{
	if (IsValid(CraftComponentPtr))
	{
		UpdateQueueCraftList(CraftComponentPtr->GetQueueItems());
		UpdateCurrentCraftProgress(CraftComponentPtr->GetCurrentCraftingRecipe());
		HandleBlocksUpdated(CraftComponentPtr->GetBlocksReasons());
	}
	else
	{
		UpdateQueueCraftList(TArray<FQueuedRecipe>());
		UpdateCurrentCraftProgress(FQueuedRecipe());
		HandleBlocksUpdated({});
	}
}

void UCraftDashboard::SetInventoryWidgets(UInventoryContainerWidget* InputWidget, UInventoryContainerWidget* FuelWidget,
	UInventoryContainerWidget* OutputWidget)
{
	if (InputSlot)
	{
		InputSlot->ClearChildren();
		if (InputWidget) InputSlot->AddChild(InputWidget);
	}

	if (FuelSlot)
	{
		FuelSlot->ClearChildren();
		if (FuelWidget) FuelSlot->AddChild(FuelWidget);
	}

	if (OutputSlot)
	{
		OutputSlot->ClearChildren();
		if (OutputWidget) OutputSlot->AddChild(OutputWidget);
	}
}

void UCraftDashboard::SetInteractorWidget(UInventoryContainerWidget* InteractorWidget)
{
	if (InteractorSlot)
	{
		InteractorSlot->ClearChildren();
		if (InteractorWidget)
		{
			InteractorSlot->AddChild(InteractorWidget);
			InteractorWidget->ReDrawRequest();
		}
	}
}

void UCraftDashboard::AddTaskBtnPressed(UUIButton* Btn)
{
	if (!CraftComponentPtr)
		return;
}

void UCraftDashboard::PauseBtnPressed(UUIButton* Btn)
{
	if (!IsValid(CraftComponentPtr) || !IsValid(Btn))
		return;
	
	if (!Btn->GetBtnTag().IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PauseBtnPressed] Btn has an invalid Tag."));
		return;
	}
	
	const auto* MySettings = UInvenzaInventorySettingsSubsystem::GetSettingsStatic(this);
	if (!MySettings)
		return;
	
	const FGameplayTag BtnTag = Btn->GetBtnTag();
	const TArray<FBlockReasonData>& ActiveBlocks = CraftComponentPtr->GetBlocksReasons();
	const bool bAlreadyBlocked = ActiveBlocks.ContainsByPredicate(
		[&BtnTag](const FBlockReasonData& Data)
		{
			return Data.Tag == BtnTag;
		}
	);
	
	const FBlockReasonData* BlockReason = MySettings->FindBlockReason(BtnTag);
	if (BlockReason)
	{
		CraftComponentPtr->SetBlockStateRequest(*BlockReason, !bAlreadyBlocked);
	}
}

void UCraftDashboard::UpdateCurrentCraftProgress(const FQueuedRecipe& Recipe)
{
	if (QueueCraftList)
	{
		QueueCraftList->UpdateDataInRecipe(Recipe);
	}
}

void UCraftDashboard::UpdateQueueCraftList(const TArray<FQueuedRecipe>& NewRecipeQueue)
{
	if (EmptyQueueLabel)
	{
		EmptyQueueLabel->SetVisibility(NewRecipeQueue.IsEmpty()
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (!QueueCraftList)
		return;
	
	QueueCraftList->SetNewProductionQueueList(NewRecipeQueue);
}

void UCraftDashboard::HandleBlocksUpdated(TArray<FBlockReasonData> Blocks)
{
	const bool bManuallyPaused = PauseBtnTag.IsValid() && Blocks.ContainsByPredicate(
		[this](const FBlockReasonData& Block) { return Block.Tag == PauseBtnTag; });

	// Use the existing button API/label; its Blueprint and internal structure stay intact.
	if (CraftControlPanel && CraftControlPanel->Btn_Pause)
	{
		UUIButton* Button = CraftControlPanel->Btn_Pause;
		Button->SetIsEnabled(IsValid(CraftComponentPtr));
		if (Button->MainLabel)
		{
			Button->MainLabel->SetText(bManuallyPaused ? ResumeButtonText : PauseButtonText);
		}
	}

	if (BlockReasonsPanel) BlockReasonsPanel->ClearChildren();
	const UInvenzaInventorySettingsAsset* Settings = UInvenzaInventorySettingsSubsystem::GetSettingsStatic(this);
	TSet<FGameplayTag> DisplayedTags;
	int32 RowCount = 0;
	if (BlockReasonsPanel && BlockReasonWidgetClass)
	{
		for (const FBlockReasonData& Block : Blocks)
		{
			if (!Block.Tag.IsValid() || DisplayedTags.Contains(Block.Tag)) continue;
			DisplayedTags.Add(Block.Tag);
			FText Message = Block.Message;
			if (Message.IsEmpty() && Settings)
			{
				if (const FBlockReasonData* Configured = Settings->FindBlockReason(Block.Tag))
				{
					Message = Configured->Message;
				}
			}
			if (Message.IsEmpty())
			{
				if (Block.Tag == PauseBtnTag)
					Message = NSLOCTEXT("InvenzaCraft", "ManualPause", "Paused by user");
				else if (Settings && Block.Tag == Settings->Block_NoResources)
					Message = NSLOCTEXT("InvenzaCraft", "NoResources", "Not enough resources");
				else if (Settings && Block.Tag == Settings->Block_NoFuel)
					Message = NSLOCTEXT("InvenzaCraft", "NoFuel", "No fuel");
				else if (Settings && Block.Tag == Settings->Block_NoOperator)
					Message = NSLOCTEXT("InvenzaCraft", "NoOperator", "Operator required");
				else
					Message = NSLOCTEXT("InvenzaCraft", "Blocked", "Production is blocked");
			}

			USimpleUserObjectListEntry* Row = CreateWidget<USimpleUserObjectListEntry>(this, BlockReasonWidgetClass);
			if (!Row) continue;
			Row->UpdateText(Message);
			if (Row->ListEntry_Text && Row->ListEntry_Text->MainTextBlock)
			{
				Row->ListEntry_Text->MainTextBlock->SetAutoWrapText(true);
			}
			Row->SetToolTipText(Message);
			if (UVerticalBoxSlot* RowSlot = Cast<UVerticalBoxSlot>(BlockReasonsPanel->AddChild(Row)))
			{
				RowSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
			}
			++RowCount;
		}
	}
	if (BlockReasonsSection)
	{
		BlockReasonsSection->SetVisibility(RowCount > 0
			? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UCraftDashboard::HandleQueueOrderChangeRequested(const FName RecipeID, const int32 QueueIndex, const bool bMoveUp)
{
	if (CraftComponentPtr)
	{
		CraftComponentPtr->RequestMoveQueueItem(RecipeID, QueueIndex, bMoveUp);
	}
}

void UCraftDashboard::HandleQueueItemDeleteRequested(int32 QueueIndex)
{
	if (CraftComponentPtr)
	{
		CraftComponentPtr->CancelRecipeRequest(QueueIndex);
	}
}
