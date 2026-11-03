// Nublin Studio 2026 All Rights Reserved.

#include "UI/Core/Modal/InventoryContextActionsWidget.h"

#include "Components/PanelWidget.h"
#include "Blueprint/WidgetTree.h"
#include "UI/Core/Buttons/UIButton.h"

void UInventoryContextActionsWidget::ConfigureButtons_Implementation(const TArray<EObjectInteractionType>& Actions,
	const TArray<FModalAction>& Display)
{
	Buttons.Reset();
	if (!Panel_Container)
	{
		return;
	}
	Panel_Container->ClearChildren();
	for (int32 Index = 0; Index < Actions.Num(); ++Index)
	{
		const FModalAction Data = Display.IsValidIndex(Index) ? Display[Index] : FModalAction();
		const TSubclassOf<UUserWidget> ButtonClass = Data.ButtonWidgetClass ?
			TSubclassOf<UUserWidget>(Data.ButtonWidgetClass.Get()) : DefaultButtonClass;
		UUserWidget* Wrapper = ButtonClass ? CreateWidget<UUserWidget>(this, ButtonClass) : nullptr;
		if (!Wrapper)
		{
			Buttons.Reset();
			Panel_Container->ClearChildren();
			return;
		}
		Panel_Container->AddChild(Wrapper);
		if (Wrapper->Implements<UModalInterface>())
		{
			IModalInterface::Execute_ConfigureModalBtn(Wrapper, Actions[Index], Data);
		}
		UUIButton* Button = Cast<UUIButton>(Wrapper);
		if (!Button && Wrapper->Implements<UUIInterface>())
		{
			const TArray<UUIButton*> WrappedButtons = IUIInterface::Execute_GetButtons(Wrapper);
			Button = WrappedButtons.IsEmpty() ? nullptr : WrappedButtons[0];
		}
		if (!Button && Wrapper->WidgetTree)
		{
			TArray<UWidget*> Widgets;
			Wrapper->WidgetTree->GetAllWidgets(Widgets);
			for (UWidget* Widget : Widgets)
			{
				if (UUIButton* Candidate = Cast<UUIButton>(Widget))
				{
					Button = Candidate;
					break;
				}
			}
		}
		if (!Button)
		{
			Buttons.Reset();
			Panel_Container->ClearChildren();
			return;
		}
		Button->SetToolTipText(Data.DisplayText);
		Buttons.Add(Button);
	}
}

TArray<UUIButton*> UInventoryContextActionsWidget::GetButtons_Implementation() const
{
	TArray<UUIButton*> Result;
	for (UUIButton* Button : Buttons)
	{
		Result.Add(Button);
	}
	return Result;
}
