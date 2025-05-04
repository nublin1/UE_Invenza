// Nublin Studio 2025 All Rights Reserved.


#include "UI/ModalWidgets/ModalTradeWidget.h"

#include "Components/Button.h"
#include "Components/Slider.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/ModalWidgets/BinaryPromptButtons.h"

UModalTradeWidget::UModalTradeWidget()
{
}

void UModalTradeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	auto PromptButtons = Cast<UBinaryPromptButtons>(ModalWindowBase->GetSlot_Interaction()->GetChildAt(0));
	if (PromptButtons)
	{
		PromptButtons->ConfirmButton->OnButtonClicked.AddDynamic(this, &UModalTradeWidget::Confirm);
		PromptButtons->CancelButton->OnButtonClicked.AddDynamic(this, &UModalTradeWidget::Cancel);
	}
}

void UModalTradeWidget::Confirm(UUIButton* Button)
{
	float v = QuantitySlider->GetValue();
	int32 qty = FMath::RoundToInt(v);

	if (ConfirmCallback)
	{
		ConfirmCallback(qty);
	}
}

void UModalTradeWidget::Cancel(UUIButton* Button)
{
	if (CancelCallback)
	{
		CancelCallback();
	}
}
