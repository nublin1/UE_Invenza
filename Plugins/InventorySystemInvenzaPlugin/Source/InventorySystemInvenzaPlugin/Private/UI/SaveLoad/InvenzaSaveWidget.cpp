//  Nublin Studio 2026 All Rights Reserved.

#include "UI/SaveLoad/InvenzaSaveWidget.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "Subsystems/UIInputModeSubsystem.h"
#include "SaveLoad/InvenzaSaveSettings.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/PanelWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/LabelBaseText.h"
#include "Data/ItemData.h"

void UInvenzaSaveWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (auto* Local = !bEmbeddedMenuPage ? GetOwningLocalPlayer() : nullptr)
	{
		Local->GetSubsystem<UUIInputModeSubsystem>()->RequestUIInput(this);
	}
	if (auto* Instance = GetGameInstance())
	{
		Instance->GetSubsystem<UInvenzaSaveSubsystem>()->OnStateChanged.AddUniqueDynamic(this, &ThisClass::Refresh);
	}
	for (auto* Button : {ContinueButton.Get(), QuickSaveButton.Get(), ManualSave1Button.Get(), ManualSave2Button.Get(),
						 ManualSave3Button.Get(), LoadButton.Get(), RecoverButton.Get(), ResetButton.Get(),
						 ConfirmResetButton.Get(), CancelResetButton.Get(), CloseButton.Get()})
	{
		if (Button)
		{
			Button->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::Clicked);
		}
	}
	if (Trust)
	{
		Trust->OnCheckStateChanged.AddUniqueDynamic(this, &ThisClass::TrustChanged);
	}
	if (Confirmation)
	{
		Confirmation->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (SaveControls)
	{
		SaveControls->SetVisibility(View == EInvenzaSaveView::Load ? ESlateVisibility::Collapsed
																   : ESlateVisibility::SelfHitTestInvisible);
	}
	if (LoadControls)
	{
		LoadControls->SetVisibility(View == EInvenzaSaveView::Save ? ESlateVisibility::Collapsed
																   : ESlateVisibility::SelfHitTestInvisible);
	}
	if (SessionControls)
	{
		SessionControls->SetVisibility(View == EInvenzaSaveView::Load ? ESlateVisibility::Collapsed
																	  : ESlateVisibility::SelfHitTestInvisible);
	}
	Refresh();
	if (!bEmbeddedMenuPage)
	{
		SetKeyboardFocus();
	}
}

void UInvenzaSaveWidget::NativeDestruct()
{
	if (auto* Local = !bEmbeddedMenuPage ? GetOwningLocalPlayer() : nullptr)
	{
		Local->GetSubsystem<UUIInputModeSubsystem>()->ReleaseUIInput(this);
	}
	if (auto* Instance = GetGameInstance())
	{
		Instance->GetSubsystem<UInvenzaSaveSubsystem>()->OnStateChanged.RemoveDynamic(this, &ThisClass::Refresh);
	}
	for (auto* Button : {ContinueButton.Get(), QuickSaveButton.Get(), ManualSave1Button.Get(), ManualSave2Button.Get(),
						 ManualSave3Button.Get(), LoadButton.Get(), RecoverButton.Get(), ResetButton.Get(),
						 ConfirmResetButton.Get(), CancelResetButton.Get(), CloseButton.Get()})
	{
		if (Button)
		{
			Button->OnButtonClicked.RemoveDynamic(this, &ThisClass::Clicked);
		}
	}
	if (Trust)
	{
		Trust->OnCheckStateChanged.RemoveDynamic(this, &ThisClass::TrustChanged);
	}
	Super::NativeDestruct();
}

void UInvenzaSaveWidget::Refresh()
{
	auto* Instance = GetGameInstance();
	if (!Instance || !StatusLabel || !Slots || !Trust)
	{
		return;
	}
	TGuardValue<bool> Guard(bRefreshing, true);
	auto* Save = Instance->GetSubsystem<UInvenzaSaveSubsystem>();
	const auto* Settings = GetDefault<UInvenzaSaveSettings>();
	const bool bHost = Save->CanManageSession();
	FText StatusText = bHost ? Save->Status : Settings->GetSaveText(TEXT("HostOnly"));
	const bool bMenuLoad = !bHost && GetWorld()->GetNetMode() != NM_Client && !LoadWorld.IsNull();
	if (bMenuLoad)
	{
		StatusText = Save->Status;
	}
	const auto RecoveryItems = Save->GetRecoveryItems();
	if (bHost && !RecoveryItems.IsEmpty())
	{
		TArray<FText> Lines;
		Lines.Add(StatusText);
		Lines.Add(Settings->GetSaveText(TEXT("RecoveryHeading")));
		for (const auto& Item : RecoveryItems)
		{
			const auto* Definition = Item.Definition.DataTable
										 ? Item.Definition.DataTable->FindRow<FItemData>(
											   Item.Definition.RowName, TEXT("Recovery display"), false)
										 : nullptr;
			const FText Name = Definition ? Definition->ItemMetaData.ItemTextData.DisplayName
										  : FText::AsCultureInvariant(Item.DefinitionRow.ToString());
			Lines.Add(FText::Format(Settings->GetSaveText(TEXT("RecoveryItem")), Name, Item.Quantity));
		}
		StatusText = FText::Join(FText::AsCultureInvariant(TEXT("\n")), Lines);
	}
	StatusLabel->UpdateText(StatusText);
	Trust->SetIsEnabled(bHost && !Save->bBusy);
	Trust->SetIsChecked(Save->GetClientTrust() == EInventoryClientTrust::TrustedClient);
	const FString PreviousKey = SlotKeysByLabel.FindRef(Slots->GetSelectedOption());
	SlotKeysByLabel.Empty();
	Slots->ClearOptions();
	for (const auto& Info : (bMenuLoad ? Save->GetSaveSlotsForWorld(LoadWorld.ToSoftObjectPath().GetLongPackageName())
									   : Save->GetSaveSlots()))
	{
		if (!Info.bAvailable)
		{
			continue;
		}
		FText SlotName;
		if (Info.Slot.StartsWith(TEXT("Auto")))
		{
			SlotName = FText::Format(Settings->GetSaveText(TEXT("AutoSlot")), FCString::Atoi(*Info.Slot.Mid(4)) + 1);
		}
		else
		{
			SlotName = Settings->GetSaveText(FName(*Info.Slot));
		}
		const FString Display =
			FText::Format(Settings->GetSaveText(TEXT("SlotEntry")), SlotName,
						  FText::AsDateTime(Info.SavedAt, EDateTimeStyle::Short, EDateTimeStyle::Short))
				.ToString();
		SlotKeysByLabel.Add(Display, Info.Slot);
		Slots->AddOption(Display);
		if (Info.Slot == PreviousKey)
		{
			Slots->SetSelectedOption(Display);
		}
	}
	if (Slots->GetSelectedOption().IsEmpty() && Slots->GetOptionCount() > 0)
	{
		Slots->SetSelectedIndex(0);
	}
	for (auto* Button : {ContinueButton.Get(), QuickSaveButton.Get(), ManualSave1Button.Get(), ManualSave2Button.Get(),
						 ManualSave3Button.Get(), RecoverButton.Get(), ResetButton.Get(), ConfirmResetButton.Get()})
	{
		if (Button)
		{
			Button->SetIsEnabled(bHost && !Save->bBusy);
		}
	}
	if (ContinueButton)
	{
		ContinueButton->SetIsEnabled((bHost || bMenuLoad) && !Save->bBusy && Slots->GetOptionCount() > 0);
	}
	if (LoadButton)
	{
		LoadButton->SetIsEnabled((bHost || bMenuLoad) && !Save->bBusy && Slots->GetOptionCount() > 0);
	}
}

void UInvenzaSaveWidget::Clicked(UUIButton* Button)
{
	if (!Button)
	{
		return;
	}
	if (Button == CloseButton)
	{
		RequestClose();
		return;
	}
	if (Button == ResetButton)
	{
		if (Confirmation)
		{
			Confirmation->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}
	if (Button == CancelResetButton)
	{
		if (Confirmation)
		{
			Confirmation->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}
	auto* Save = GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>();
	if (Button == ConfirmResetButton)
	{
		Save->ResetPlaythrough();
	}
	else if (Button == RecoverButton)
	{
		Save->RetryRecovery();
	}
	else if (Button == ContinueButton)
	{
		if (Save->CanManageSession())
		{
			Save->LoadSession();
		}
		else
		{
			Save->LoadWorldSave(LoadWorld, TEXT("Continue"));
		}
	}
	else if (Button == QuickSaveButton)
	{
		Save->SaveSession(TEXT("Quick"));
	}
	else if (Button == ManualSave1Button)
	{
		Save->SaveSession(TEXT("Manual1"));
	}
	else if (Button == ManualSave2Button)
	{
		Save->SaveSession(TEXT("Manual2"));
	}
	else if (Button == ManualSave3Button)
	{
		Save->SaveSession(TEXT("Manual3"));
	}
	else if (Button == LoadButton && Slots)
	{
		if (const auto* Key = SlotKeysByLabel.Find(Slots->GetSelectedOption()))
		{
			if (Save->CanManageSession())
			{
				Save->LoadSession(*Key);
			}
			else
			{
				Save->LoadWorldSave(LoadWorld, *Key);
			}
		}
	}
}

void UInvenzaSaveWidget::TrustChanged(bool bChecked)
{
	if (!bRefreshing)
	{
		GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->SetClientTrust(
			bChecked ? EInventoryClientTrust::TrustedClient : EInventoryClientTrust::ServerValidated);
	}
}

FReply UInvenzaSaveWidget::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
	if (Event.GetKey() == EKeys::Escape)
	{
		RequestClose();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(Geometry, Event);
}

void UInvenzaSaveWidget::RequestClose()
{
	if (OnCloseRequested.IsBound())
	{
		OnCloseRequested.Broadcast();
	}
	else
	{
		RemoveFromParent();
	}
}
