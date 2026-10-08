// Nublin Studio 2026 All Rights Reserved.
#include "UI/Craft/CraftingQuantitySelector.h"
#include "Components/EditableTextBox.h"
#include "UI/Core/EditableLabelBaseText.h"
#include "UI/Core/Buttons/UIButton.h"

UCraftingQuantitySelector::UCraftingQuantitySelector() {}

void UCraftingQuantitySelector::NativePreConstruct()
{
	Super::NativePreConstruct();
	UpdateText();
	UpdateButtons();
}

void UCraftingQuantitySelector::NativeConstruct()
{
	Super::NativeConstruct();
	if (Btn_SetMin) Btn_SetMin->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::SetToMin);
	if (Btn_SetMax) Btn_SetMax->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::SetToMax);
	if (Btn_Decrease) Btn_Decrease->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::Decrease);
	if (Btn_Increase) Btn_Increase->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::Increase);
	if (CurrentQuantityText)
	{
		CurrentQuantityText->OnEditableTextChanged.AddUniqueDynamic(this, &ThisClass::OnTextCommitted);
		if (CurrentQuantityText->MainEditableTextBox)
			CurrentQuantityText->MainEditableTextBox->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::HandleTextCommitted);
	}
	SetQuantity(CurrentQuantity);
}

void UCraftingQuantitySelector::NativeDestruct()
{
	if (Btn_SetMin) Btn_SetMin->OnButtonClicked.RemoveDynamic(this, &ThisClass::SetToMin);
	if (Btn_SetMax) Btn_SetMax->OnButtonClicked.RemoveDynamic(this, &ThisClass::SetToMax);
	if (Btn_Decrease) Btn_Decrease->OnButtonClicked.RemoveDynamic(this, &ThisClass::Decrease);
	if (Btn_Increase) Btn_Increase->OnButtonClicked.RemoveDynamic(this, &ThisClass::Increase);
	if (CurrentQuantityText)
	{
		CurrentQuantityText->OnEditableTextChanged.RemoveDynamic(this, &ThisClass::OnTextCommitted);
		if (CurrentQuantityText->MainEditableTextBox)
			CurrentQuantityText->MainEditableTextBox->OnTextCommitted.RemoveDynamic(this, &ThisClass::HandleTextCommitted);
	}
	Super::NativeDestruct();
}

int32 UCraftingQuantitySelector::GetCurrentQuantity() const { return CurrentQuantity; }
void UCraftingQuantitySelector::SetToMin(UUIButton* ButtonPressed) { SetQuantity(MinQuantity); }
void UCraftingQuantitySelector::SetToMax(UUIButton* ButtonPressed)
{
	if (ResourceMaximum >= FMath::Max(1, MinQuantity)) SetQuantity(ResourceMaximum);
}
void UCraftingQuantitySelector::Increase(UUIButton* ButtonPressed)
{
	CommitPendingQuantity();
	SetQuantity(CurrentQuantity < MAX_int32 ? CurrentQuantity + 1 : MAX_int32);
}
void UCraftingQuantitySelector::Decrease(UUIButton* ButtonPressed)
{
	CommitPendingQuantity();
	SetQuantity(CurrentQuantity - 1);
}
void UCraftingQuantitySelector::SetQuantity(int32 NewValue)
{
	const int32 Value = FMath::Max(FMath::Max(1, MinQuantity), NewValue);
	const bool bChanged = CurrentQuantity != Value;
	CurrentQuantity = Value;
	bHasPendingText = false;
	UpdateText();
	UpdateButtons();
	if (bChanged) OnQuantityChanged.Broadcast(CurrentQuantity);
}
void UCraftingQuantitySelector::SetResourceMaximum(int32 NewMaximum)
{
	ResourceMaximum = FMath::Max(0, NewMaximum);
	UpdateButtons();
}
void UCraftingQuantitySelector::UpdateButtons()
{
	if (Btn_SetMax) Btn_SetMax->SetIsEnabled(ResourceMaximum >= FMath::Max(1, MinQuantity));
	if (Btn_Decrease) Btn_Decrease->SetIsEnabled(CurrentQuantity > FMath::Max(1, MinQuantity));
	if (Btn_Increase) Btn_Increase->SetIsEnabled(CurrentQuantity < MAX_int32);
}
void UCraftingQuantitySelector::OnTextCommitted(const FText& NewText)
{
	// The legacy callback is bound to text CHANGED. Do not rewrite the field mid-edit.
	if (bUpdatingText) return;
	PendingText = NewText;
	bHasPendingText = true;
}
void UCraftingQuantitySelector::HandleTextCommitted(const FText& NewText, ETextCommit::Type CommitMethod)
{
	if (bUpdatingText) return;
	PendingText = NewText;
	bHasPendingText = true;
	CommitPendingQuantity();
}
void UCraftingQuantitySelector::CommitPendingQuantity()
{
	if (!bHasPendingText) return;
	const FString Input = PendingText.ToString().TrimStartAndEnd();
	int64 Value = 0;
	bool bValid = !Input.IsEmpty();
	for (TCHAR Character : Input)
	{
		if (Character < TEXT('0') || Character > TEXT('9')) { bValid = false; break; }
		Value = FMath::Min<int64>(MAX_int32, Value * 10 + Character - TEXT('0'));
	}
	SetQuantity(bValid ? static_cast<int32>(Value) : CurrentQuantity);
}
void UCraftingQuantitySelector::UpdateText()
{
	if (!CurrentQuantityText || bUpdatingText) return;
	TGuardValue<bool> Guard(bUpdatingText, true);
	// Ungrouped digits also round-trip through the numeric editor in every locale.
	CurrentQuantityText->UpdateText(FText::FromString(FString::FromInt(CurrentQuantity)));
}
