//  Nublin Studio 2026 All Rights Reserved.

#include "UI/Inventory/StorageAccessWidget.h"
#include "UI/Inventory/InventoryWorkspaceWidget.h"
#include "ActorComponents/UIInventoryManager.h"
#include "UI/Core/Buttons/UIButton.h"
#include "Subsystems/UIInputModeSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "InputCoreTypes.h"

UStorageAccessWidget::UStorageAccessWidget()
{
	WorkspaceSettings.PlayerColumnMinWidth = 430.f;
	WorkspaceSettings.bFitPlayerColumnToContent = true;
	WorkspaceSettings.bAllowInventoryCollapse = true;
	WorkspaceSettings.bHideInventoryCloseButtons = true;
}

void UStorageAccessWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (Workspace)
	{
		Workspace->ConfigureWorkspace(WorkspaceSettings);
	}
}

void UStorageAccessWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (Workspace)
	{
		Workspace->ConfigureWorkspace(WorkspaceSettings);
	}

	SetVisibility(ESlateVisibility::Visible);

	CloseButton->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::CloseClicked);
	RefreshButton->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::RefreshClicked);

	if (auto* Player = GetOwningLocalPlayer())
	{
		if (auto* InputMode = Player->GetSubsystem<UUIInputModeSubsystem>())
		{
			InputMode->RequestUIInput(this);
		}
	}

	SetIsFocusable(true);
	SetKeyboardFocus();
}

void UStorageAccessWidget::NativeDestruct()
{
	CloseButton->OnButtonClicked.RemoveDynamic(this, &ThisClass::CloseClicked);
	RefreshButton->OnButtonClicked.RemoveDynamic(this, &ThisClass::RefreshClicked);

	if (auto* Player = GetOwningLocalPlayer())
	{
		if (auto* InputMode = Player->GetSubsystem<UUIInputModeSubsystem>())
		{
			InputMode->ReleaseUIInput(this);
		}
	}

	Super::NativeDestruct();
}

void UStorageAccessWidget::CloseClicked(UUIButton*)
{
	if (Manager)
	{
		Manager->CloseStorageAccess();
	}
}

void UStorageAccessWidget::RefreshClicked(UUIButton*)
{
	if (Manager)
	{
		Manager->RefreshStorageAccess();
	}
}

FReply UStorageAccessWidget::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
	if (Event.GetKey() == EKeys::Escape)
	{
		CloseClicked(nullptr);
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(Geometry, Event);
}
