// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/CraftingQuantitySelector.h"

#include "UI/Core/EditableLabelBaseText.h"
#include "UI/Core/Buttons/UIButton.h"


UCraftingQuantitySelector::UCraftingQuantitySelector()
{
	SetQuantity(MinQuantity);
}

void UCraftingQuantitySelector::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (Btn_SetMin)
		Btn_SetMin->OnButtonClicked.AddDynamic(this, &UCraftingQuantitySelector::SetToMin);
	if (Btn_SetMax)
		Btn_SetMax->OnButtonClicked.AddDynamic(this, &UCraftingQuantitySelector::SetToMax);

	Btn_Decrease->OnButtonClicked.AddDynamic(this, &UCraftingQuantitySelector::Decrease);
	Btn_Increase->OnButtonClicked.AddDynamic(this, &UCraftingQuantitySelector::Increase);
}

void UCraftingQuantitySelector::NativeConstruct()
{
	Super::NativeConstruct();

	if (CurrentQuantityText)
	{
		CurrentQuantityText->OnEditableTextChanged.AddDynamic(this, &UCraftingQuantitySelector::OnTextCommitted);
	}
}

int32 UCraftingQuantitySelector::GetCurrentQuantity() const
{
	return CurrentQuantity;
}

void UCraftingQuantitySelector::SetToMin(UUIButton* ButtonPressed)
{
	SetQuantity(MinQuantity);
}

void UCraftingQuantitySelector::SetToMax(UUIButton* ButtonPressed)
{
	/*const int32 Max = GetMaxQuantity();
	SetQuantity(Max);*/
}

void UCraftingQuantitySelector::Increase(UUIButton* ButtonPressed)
{
	SetQuantity(CurrentQuantity + 1);
}

void UCraftingQuantitySelector::Decrease(UUIButton* ButtonPressed)
{
	const int32 ClampedValue = FMath::Clamp(CurrentQuantity - 1, MinQuantity, CurrentQuantity);
	SetQuantity(ClampedValue);
}

void UCraftingQuantitySelector::SetQuantity(int32 NewValue)
{
	/*const int32 Max = GetMaxQuantity();
	const int32 ClampedValue = FMath::Clamp(NewValue, MinQuantity, Max);

	if (ClampedValue == CurrentQuantity)
		return;

	CurrentQuantity = ClampedValue;*/
	CurrentQuantity = NewValue;

	UpdateText();
	OnQuantityChanged.Broadcast(CurrentQuantity);
}

void UCraftingQuantitySelector::OnTextCommitted(const FText& NewText)
{
	const int32 Value = FCString::Atoi(*NewText.ToString());
	SetQuantity(Value);
}

void UCraftingQuantitySelector::UpdateText()
{
	if (!CurrentQuantityText)
		return;

	CurrentQuantityText->UpdateText(FText::AsNumber(CurrentQuantity));
}
