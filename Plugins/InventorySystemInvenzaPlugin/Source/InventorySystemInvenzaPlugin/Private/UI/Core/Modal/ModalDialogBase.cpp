// Nublin Studio 2026 All Rights Reserved.


#include "UI/Core/Modal/ModalDialogBase.h"

#include "Components/NamedSlot.h"
#include "Interface/UI/ModalButtonsPanelInterface.h"
#include "UI/Core/Buttons/UIButton.h"

void UModalDialogBase::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Btns_Slot)
		Configure();
}

void UModalDialogBase::Configure()
{
	UWidget* Content = Btns_Slot->GetContent();
	
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
