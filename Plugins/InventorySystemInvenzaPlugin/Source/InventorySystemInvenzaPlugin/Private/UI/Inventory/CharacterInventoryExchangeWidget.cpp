//  Nublin Studio 2026 All Rights Reserved.

#include "UI/Inventory/CharacterInventoryExchangeWidget.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Engine/LocalPlayer.h"
#include "Subsystems/UIInputModeSubsystem.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"

UCharacterInventoryExchangeWidget::UCharacterInventoryExchangeWidget()
{
	SetIsFocusable(true);
}

void UCharacterInventoryExchangeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CloseButton->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::CloseClicked);
	if (auto* Player = GetOwningLocalPlayer())
	{
		if (auto* Input = Player->GetSubsystem<UUIInputModeSubsystem>())
		{
			Input->RequestUIInput(this);
		}
	}
	SetKeyboardFocus();
}

void UCharacterInventoryExchangeWidget::NativeDestruct()
{
	CloseButton->OnButtonClicked.RemoveDynamic(this, &ThisClass::CloseClicked);
	if (auto* Player = GetOwningLocalPlayer())
	{
		if (auto* Input = Player->GetSubsystem<UUIInputModeSubsystem>())
		{
			Input->ReleaseUIInput(this);
		}
	}
	Super::NativeDestruct();
}

void UCharacterInventoryExchangeWidget::SetInventories(const FText& CharacterName,
													   const TArray<UInventoryContainerWidget*>& Other,
													   const TArray<UInventoryContainerWidget*>& Player)
{
	ClearInventories();
	CharacterLabel->UpdateText(CharacterName);
	auto Install = [this](UHorizontalBox* Column, const TArray<UInventoryContainerWidget*>& Widgets)
	{
		if (!Column)
		{
			return;
		}
		for (auto* Widget : Widgets)
		{
			if (!Widget)
			{
				continue;
			}
			OriginalCloseOptions.Add(Widget, Widget->GetShowCloseButton());
			Widget->SetShowCloseButton(false);
			Widget->SetVisibility(ESlateVisibility::Visible);
			auto* InventorySlot = Column->AddChildToHorizontalBox(Widget);
			InventorySlot->SetVerticalAlignment(VAlign_Top);
			InventorySlot->SetPadding(Widget == Widgets.Last() ? FMargin() : InventoryPadding);
		}
	};
	Install(OtherInventories, Other);
	Install(PlayerInventories, Player);
}

void UCharacterInventoryExchangeWidget::ClearInventories()
{
	for (const auto& Pair : OriginalCloseOptions)
	{
		if (IsValid(Pair.Key))
		{
			Pair.Key->RemoveFromParent();
			Pair.Key->SetShowCloseButton(Pair.Value);
		}
	}
	OriginalCloseOptions.Reset();
	if (OtherInventories)
	{
		OtherInventories->ClearChildren();
	}
	if (PlayerInventories)
	{
		PlayerInventories->ClearChildren();
	}
}

void UCharacterInventoryExchangeWidget::CloseClicked(UUIButton*)
{
	if (Manager)
	{
		Manager->CloseCharacterInventory();
	}
}

FReply UCharacterInventoryExchangeWidget::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
	if (Event.GetKey() == EKeys::Escape)
	{
		CloseClicked(nullptr);
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(Geometry, Event);
}
