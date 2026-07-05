// Nublin Studio 2026 All Rights Reserved.


#include "UI/Core/Modal/ModalDialogBase.h"

#include "Components/NamedSlot.h"
#include "Interface/UI/ModalButtonsPanelInterface.h"
#include "UI/Core/Buttons/UIButton.h"

void UModalDialogBase::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Down_Slot)
		Configure();
}

void UModalDialogBase::ForceClose(FModalResult Result)
{
}

void UModalDialogBase::Configure()
{
	UWidget* Content = Down_Slot->GetContent();
	
	if (!Content)
	{
		return;
	}
	
	if (Content->GetClass()->ImplementsInterface(UModalButtonsPanelInterface::StaticClass()))
	{
		TArray<UUIButton*> Buttons = IModalButtonsPanelInterface::Execute_GetButtons(Content);
		if (Buttons.Num() == 0)
			return;

		for (auto Btn : Buttons)
		{
			Btn->OnButtonClicked.AddDynamic(this, &UModalDialogBase::OnButtonClicked);
		}
	}
}

void UModalDialogBase::OnButtonClicked(UUIButton* UIButton)
{
	
}
