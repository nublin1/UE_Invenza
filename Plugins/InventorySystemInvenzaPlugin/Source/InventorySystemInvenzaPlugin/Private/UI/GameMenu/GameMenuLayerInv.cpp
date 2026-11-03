// Nublin Studio 2026 All Rights Reserved.

#include "UI/GameMenu/GameMenuLayerInv.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "Utility/InvenzayUtility.h"

#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/NamedSlot.h"
#include "Components/PanelWidget.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Data/Inventory/InventoryBase.h"
#include "DragDrop/InvContainerDragDropOperation.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Subsystems/UIInputModeSubsystem.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/MovableTitleBar/MovableTitleBar.h"
#include "UI/Craft/CraftControlPanel.h"
#include "UI/Craft/CraftDashboard.h"
#include "UI/Craft/CraftMenuChoose.h"
#include "UI/Inventory/Container/DualInventoryWidget.h"

UGameMenuLayerInv::UGameMenuLayerInv()
{
}

void UGameMenuLayerInv::OnRemovedFromLayer_Implementation()
{
	APawn* Pawn = GetOwningPlayerPawn();
	if (GetWorld() && !GetWorld()->bIsTearingDown && IsValid(Pawn) && !Pawn->IsActorBeingDestroyed())
	{
		if (auto* Manager = Pawn->FindComponentByClass<UIInventoryManager>())
		{
			Manager->RequestCloseSecondaryInventory();
			Manager->CloseStorageAccess();
			Manager->CloseCharacterInventory();
		}
		CloseCraftMenu();
	}
	if (IsValid(Pawn))
	{
		if (auto* Collection = Pawn->FindComponentByClass<UInventoryCollectionComponent>())
		{
			for (auto* Container : GetAllPawnInvContainers())
			{
				auto* InventoryWidget = Container->GetInventoryWidgetFromContainerSlot();
				auto* Inventory = InventoryWidget ? InventoryWidget->GetInventoryRef() : nullptr;
				if (Inventory && Collection->GetContainerWidget(Inventory) == Container)
				{
					Collection->UnregisterContainerWidget(Inventory);
					Container->ReleaseInventory();
				}
			}
		}
	}
	bInventoryOpen = false;
	bCraftMenuOpen = false;
	UpdateInputMode();
	Super::OnRemovedFromLayer_Implementation();
}

bool UGameMenuLayerInv::AttachStorageWindow(UUserWidget* Window)
{
	if (!IsValid(Window))
	{
		return false;
	}
	if (auto* Overlay = Cast<UOverlay>(GetRootWidget()))
	{
		auto* WindowSlot = Overlay->AddChildToOverlay(Window);
		WindowSlot->SetHorizontalAlignment(HAlign_Fill);
		WindowSlot->SetVerticalAlignment(VAlign_Fill);
		NotifyDiagnosticsChanged();
		return true;
	}
	return false;
}

void UGameMenuLayerInv::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	APawn* OwnerPawn = GetOwningPlayerPawn();
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("OwnerPawn is null in NativeOnInitialized"));
		return;
	}

	UIInventoryManager* InvManager = OwnerPawn->FindComponentByClass<UIInventoryManager>();
	if (!InvManager)
	{
		UE_LOG(LogTemp, Error, TEXT("UIInventoryManager not found on OwnerPawn"));
		return;
	}

	if (!GetClass()->ImplementsInterface(UInvUIProvider::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("UGameMenuLayerInv does not implement IInvUIProvider"));
		return;
	}

	TScriptInterface<IInvUIProvider> ProviderInterface;
	ProviderInterface.SetObject(this);
	ProviderInterface.SetInterface(Cast<IInvUIProvider>(this));

	InvManager->SetUIProvider(ProviderInterface);
}

TArray<UUInventoryBaseWidget*> UGameMenuLayerInv::GetAllPawnInventories() const
{
	TArray<UUInventoryBaseWidget*> Result;
	if (!PawnInventories)
	{
		return Result;
	}

	for (int32 i = 0; i < PawnInventories->GetChildrenCount(); i++)
	{
		if (auto* InvContainer = Cast<UInventoryContainerWidget>(PawnInventories->GetChildAt(i)))
		{
			if (auto InventoryWidget = Cast<UUInventoryBaseWidget>(InvContainer->ContainerSlot->GetContent()))
			{
				Result.Add(InventoryWidget);
			}
		}
	}
	return Result;
}

TArray<UInventoryContainerWidget*> UGameMenuLayerInv::GetAllPawnInvContainers() const
{
	TArray<UInventoryContainerWidget*> Result;
	if (!PawnInventories)
	{
		return Result;
	}
	for (int32 i = 0; i < PawnInventories->GetChildrenCount(); i++)
	{
		if (auto* InvContainer = Cast<UInventoryContainerWidget>(PawnInventories->GetChildAt(i)))
		{
			Result.Add(InvContainer);
		}
	}

	return Result;
}

UPanelSlot* UGameMenuLayerInv::AddPawnInvContainerWidget(UInventoryContainerWidget* InvContainerWidgetToAdd) const
{
	if (!GetWorld() || GetWorld()->bIsTearingDown || !IsValid(PawnInventories) || !IsValid(InvContainerWidgetToAdd))
	{
		return nullptr;
	}
	if (const APawn* Pawn = GetOwningPlayerPawn(); Pawn && Pawn->IsActorBeingDestroyed())
	{
		return nullptr;
	}
	auto* InvWidget = InvContainerWidgetToAdd->GetInventoryWidgetFromContainerSlot();
	if (!IsValid(InvWidget) || !IsValid(InvWidget->GetInventoryRef()))
	{
		return nullptr;
	}

	UPanelSlot* InvContainerSlot = PawnInventories->AddChild(InvContainerWidgetToAdd);

	auto InvSettings = InvWidget->GetInventoryRef()->GetInventorySettings();
	InvWidget->ReDrawAllItems();
	EInventoryType Type = InvSettings.InventoryType;

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(InvContainerSlot))
	{
		CanvasSlot->SetAutoSize(true);
		CanvasSlot->SetSize(FVector2D::ZeroVector);
		if (InventoryDefaultPositions.Contains(Type))
		{
			FVector2D NormalizedPos = InventoryDefaultPositions[Type];

			FVector2D ViewportSize;
			GEngine->GameViewport->GetViewportSize(ViewportSize);

			FVector2D RealPos = FVector2D(NormalizedPos.X * ViewportSize.X, NormalizedPos.Y * ViewportSize.Y);

			CanvasSlot->SetPosition(RealPos);
		}
	}

	if (InvSettings.bIsAlwaysVisible || bInventoryOpen)
	{
		InvContainerWidgetToAdd->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		InvContainerWidgetToAdd->SetVisibility(ESlateVisibility::Collapsed);
	}

	NotifyDiagnosticsChanged();
	return InvContainerSlot;
}

void UGameMenuLayerInv::RemovePawnInvContainer(UInventoryContainerWidget* InvContainerToRemove) const
{
	if (!PawnInventories)
	{
		return;
	}

	for (int32 i = 0; i < PawnInventories->GetChildrenCount(); i++)
	{
		if (PawnInventories->GetChildAt(i) == InvContainerToRemove)
		{
			PawnInventories->RemoveChildAt(i);
			break;
		}
	}
	NotifyDiagnosticsChanged();
}

void UGameMenuLayerInv::OpenDualInventoryView(UInventoryContainerWidget* ExternalContainerWidget,
											  UInventoryContainerWidget* PlayerInventoryToShow)
{
	TArray<UInventoryContainerWidget*> Containers;
	if (PlayerInventoryToShow)
	{
		Containers.Add(PlayerInventoryToShow);
	}
	OpenInventoryInteractionView(ExternalContainerWidget, Containers);
}

void UGameMenuLayerInv::ReturnInteractionPanels()
{
	auto* Pawn = GetOwningPlayerPawn();
	auto* Manager = Pawn ? Pawn->FindComponentByClass<UIInventoryManager>() : nullptr;
	const auto Accessible = Manager ? Manager->GetInteractionInventories() : TArray<UInventoryBase*>();
	for (const auto& Placement : BorrowedPlayerPlacements)
	{
		auto* Widget = Placement.Widget.Get();
		if (!IsValid(Widget))
		{
			continue;
		}
		Widget->OnClose.RemoveDynamic(this, &ThisClass::InteractionPanelClosed);
		Widget->RemoveFromParent();
		auto* InvWidget = Widget->GetInventoryWidgetFromContainerSlot();
		if (!InvWidget || !Accessible.Contains(InvWidget->GetInventoryRef()))
		{
			continue;
		}
		if (!UInvenzayUtility::RestoreInventoryPanel(Placement))
		{
			AddPawnInvContainerWidget(Widget);
			Widget->SetVisibility(Placement.Visibility);
		}
	}
	BorrowedPlayerPlacements.Reset();
}

void UGameMenuLayerInv::OpenInventoryInteractionView(UInventoryContainerWidget* ExternalContainerWidget,
	const TArray<UInventoryContainerWidget*>& PlayerInventories)
{
	if (!DualInventoryWidget || !ExternalContainerWidget)
	{
		return;
	}
	if (!CurrentExternalContainer)
	{
		bInventoryOpenBeforeInteraction = bInventoryOpen;
	}
	ReturnInteractionPanels();
	if (CurrentExternalContainer)
	{
		CurrentExternalContainer->OnClose.RemoveDynamic(this, &ThisClass::InteractionPanelClosed);
	}
	DualInventoryWidget->ClearAll();
	for (auto* Container : PlayerInventories)
	{
		if (IsValid(Container) && Container != ExternalContainerWidget &&
			!BorrowedPlayerPlacements.ContainsByPredicate([Container](const auto& Entry)
			{
				return Entry.Widget == Container;
			}))
		{
			Container->OnClose.AddUniqueDynamic(this, &ThisClass::InteractionPanelClosed);
			BorrowedPlayerPlacements.Add(UInvenzayUtility::CaptureInventoryPanel(Container));
			Container->SetVisibility(ESlateVisibility::Visible);
		}
	}
	CurrentExternalContainer = ExternalContainerWidget;
	ExternalContainerWidget->OnClose.AddUniqueDynamic(this, &ThisClass::InteractionPanelClosed);
	DualInventoryWidget->SetLeftInventoryContainer(ExternalContainerWidget);
	DualInventoryWidget->SetPlayerInventories(PlayerInventories);
	ExternalContainerWidget->SetVisibility(ESlateVisibility::Visible);
	DualInventoryWidget->ApplyCornerAlignment();
	DualInventoryWidget->SetVisibility(ESlateVisibility::Visible);
	if (WorldDropZone)
	{
		WorldDropZone->SetVisibility(ESlateVisibility::Visible);
	}
	bInventoryOpen = true;
	UpdateInputMode();
}

void UGameMenuLayerInv::CloseDualInventoryView()
{
	if (!DualInventoryWidget || !CurrentExternalContainer)
	{
		return;
	}
	auto* External = CurrentExternalContainer.Get();
	CurrentExternalContainer = nullptr;
	if (APawn* Pawn = GetOwningPlayerPawn())
	{
		if (UIInventoryManager* Manager = Pawn->FindComponentByClass<UIInventoryManager>())
		{
			Manager->CloseTradeSession();
		}
	}
	bInventoryOpen = bInventoryOpenBeforeInteraction;
	ReturnInteractionPanels();
	External->OnClose.RemoveDynamic(this, &ThisClass::InteractionPanelClosed);
	DualInventoryWidget->ClearAll();
	DualInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	if (WorldDropZone)
	{
		WorldDropZone->SetVisibility(bInventoryOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	UpdateInputMode();
}

void UGameMenuLayerInv::OpenNestedInventory(UObject* Key, UInventoryContainerWidget* ContainerWidget,
											FVector2D ScreenPosition)
{
	if (!DualInventoryWidget || !ContainerWidget)
	{
		return;
	}
	DualInventoryWidget->AddFloatingInventory(Key, ContainerWidget, ScreenPosition);
	NotifyDiagnosticsChanged();
}

void UGameMenuLayerInv::CloseNestedInventory(UObject* Key)
{
	if (!DualInventoryWidget)
	{
		return;
	}
	DualInventoryWidget->RemoveFloatingInventory(Key);
	NotifyDiagnosticsChanged();
}

void UGameMenuLayerInv::ToggleInventoryLayout()
{
	if (!GetWorld())
	{
		return;
	}

	if (CurrentExternalContainer)
	{
		if (auto* Pawn = GetOwningPlayerPawn())
		{
			if (auto* Manager = Pawn->FindComponentByClass<UIInventoryManager>())
			{
				Manager->RequestCloseSecondaryInventory();
			}
		}
		return;
	}
	bInventoryOpen = !bInventoryOpen;

	const auto Containers = GetAllPawnInvContainers();
	if (Containers.IsEmpty())
	{
		return;
	}

	const ESlateVisibility NewVisibility = bInventoryOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	bool bAnyToggleableVisible = bInventoryOpen;

	if (WorldDropZone)
	{
		WorldDropZone->SetVisibility(NewVisibility);
	}

	for (auto* Container : Containers)
	{
		if (!Container)
		{
			continue;
		}

		auto InvWidget = Container->GetInventoryWidgetFromContainerSlot();
		if (InvWidget && InvWidget->GetInventoryRef() &&
			InvWidget->GetInventoryRef()->GetInventorySettings().bIsAlwaysVisible)
		{
			Container->SetVisibility(ESlateVisibility::Visible);
			continue;
		}

		Container->SetVisibility(NewVisibility);
	}

	UpdateInputMode();
}

UInvenzaBaseWidget* UGameMenuLayerInv::GetCraftMenuDashboard()
{
	if (!PawnCraftWidgetsPanel)
	{
		return nullptr;
	}

	for (int32 i = 0; i < PawnCraftWidgetsPanel->GetChildrenCount(); i++)
	{
		if (auto Dashboard = Cast<UCraftDashboard>(PawnCraftWidgetsPanel->GetChildAt(i)))
		{
			return Dashboard;
		}
	}

	return nullptr;
}

UInvenzaBaseWidget* UGameMenuLayerInv::GetCraftChoose()
{
	if (!PawnCraftWidgetsPanel)
	{
		return nullptr;
	}

	for (int32 i = 0; i < PawnCraftWidgetsPanel->GetChildrenCount(); i++)
	{
		if (auto Dashboard = Cast<UCraftMenuChoose>(PawnCraftWidgetsPanel->GetChildAt(i)))
		{
			return Dashboard;
		}
	}

	return nullptr;
}

UPanelSlot* UGameMenuLayerInv::AddPawnCraftDashboardWidget(UInvenzaBaseWidget* WidgetToAdd)
{
	if (!WidgetToAdd || !PawnCraftWidgetsPanel)
	{
		return nullptr;
	}

	UPanelSlot* CraftSlot = PawnCraftWidgetsPanel->AddChild(WidgetToAdd);
	WidgetToAdd->SetVisibility(ESlateVisibility::Collapsed);

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(CraftSlot))
	{
		CanvasSlot->SetAutoSize(true);
		CanvasSlot->SetSize(FVector2D(0, 0));
	}

	NotifyDiagnosticsChanged();
	return CraftSlot;
}

UPanelSlot* UGameMenuLayerInv::AddPawnCraftChooseWidget(UInvenzaBaseWidget* WidgetToAdd)
{
	return AddPawnCraftDashboardWidget(WidgetToAdd);
}

void UGameMenuLayerInv::ToggleCraftMenuLayout()
{
	if (!GetWorld())
	{
		return;
	}

	bCraftMenuOpen = !bCraftMenuOpen;
	if (auto* Dashboard = Cast<UCraftDashboard>(GetCraftMenuDashboard()))
	{
		if (bCraftMenuOpen)
		{
			Dashboard->AttachInteractor();
		}
		else if (auto* PlayerWidget = Dashboard->DetachInteractor())
		{
			AddPawnInvContainerWidget(PlayerWidget);
		}
	}

	const ESlateVisibility NewVisibility = bCraftMenuOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	if (WorldDropZone)
	{
		WorldDropZone->SetVisibility(NewVisibility);
	}

	if (bCraftMenuOpen)
	{
		SetCraftMenuState(CraftMenuState);
	}
	else
	{
		if (auto Dashboard = GetCraftMenuDashboard())
		{
			Dashboard->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (auto Choose = GetCraftChoose())
		{
			Choose->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	UpdateInputMode();
}

void UGameMenuLayerInv::OpenCraftDashboard()
{
	SetCraftMenuState(ECraftMenuState::Dashboard);
	if (!bCraftMenuOpen)
	{
		ToggleCraftMenuLayout();
	}
}

void UGameMenuLayerInv::CloseCraftMenu()
{
	if (bCraftMenuOpen)
	{
		ToggleCraftMenuLayout();
	}

	CraftMenuState = ECraftMenuState::Dashboard;
}

void UGameMenuLayerInv::BindCraftWidgets()
{
	auto Dashboard = Cast<UCraftDashboard>(GetCraftMenuDashboard());
	if (Dashboard && Dashboard->CraftControlPanel && Dashboard->CraftControlPanel->Btn_AddTask)
	{
		Dashboard->CraftControlPanel->Btn_AddTask->OnButtonClicked.AddDynamic(this,
																			  &UGameMenuLayerInv::HandleCraftMenuSwap);
	}

	auto Choose = Cast<UCraftMenuChoose>(GetCraftChoose());
	if (Choose)
	{
		Choose->MovableTitleBar->Button_Close->OnButtonClicked.AddDynamic(this,
																		  &UGameMenuLayerInv::HandleCraftMenuSwap);
	}
}

void UGameMenuLayerInv::HandleCraftMenuSwap(UUIButton* UIButton)
{
	const ECraftMenuState NewState =
		CraftMenuState == ECraftMenuState::Dashboard ? ECraftMenuState::Choose : ECraftMenuState::Dashboard;

	SetCraftMenuState(NewState);
}

void UGameMenuLayerInv::SetCraftMenuState(ECraftMenuState NewState)
{
	CraftMenuState = NewState;

	auto Dashboard = GetCraftMenuDashboard();
	auto Choose = GetCraftChoose();

	if (Dashboard)
	{
		Dashboard->SetVisibility(NewState == ECraftMenuState::Dashboard ? ESlateVisibility::Visible
																		: ESlateVisibility::Collapsed);
	}

	if (Choose)
	{
		Choose->SetVisibility(NewState == ECraftMenuState::Choose ? ESlateVisibility::Visible
																  : ESlateVisibility::Collapsed);
	}
}

void UGameMenuLayerInv::UpdateInputMode()
{
	NotifyDiagnosticsChanged();
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		UUIInputModeSubsystem* Input = LocalPlayer->GetSubsystem<UUIInputModeSubsystem>();
		if (!Input)
		{
			return;
		}
		if (bInventoryOpen || bCraftMenuOpen)
		{
			Input->RequestUIInput(this);
		}
		else
		{
			Input->ReleaseUIInput(this);
		}
	}
}

void UGameMenuLayerInv::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateInputMode();
}

bool UGameMenuLayerInv::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
									 UDragDropOperation* InOperation)
{
	if (UInvContainerDragDropOperation* DragOp = Cast<UInvContainerDragDropOperation>(InOperation))
	{
		return true;
	}

	return false;
}

void UGameMenuLayerInv::InteractionPanelClosed(UInvenzaBaseWidget* Widget)
{
	if (auto* Pawn = GetOwningPlayerPawn())
	{
		if (auto* Manager = Pawn->FindComponentByClass<UIInventoryManager>())
		{
			Manager->RequestCloseSecondaryInventory();
		}
	}
}

void UGameMenuLayerInv::RefreshInteractionInventories(const TArray<UInventoryContainerWidget*>& PlayerInventories)
{
	if (!CurrentExternalContainer)
	{
		return;
	}
	bool bSame = PlayerInventories.Num() == BorrowedPlayerPlacements.Num();
	for (int32 Index = 0; bSame && Index < PlayerInventories.Num(); ++Index)
	{
		bSame = PlayerInventories[Index] == BorrowedPlayerPlacements[Index].Widget;
	}
	if (!bSame)
	{
		OpenInventoryInteractionView(CurrentExternalContainer, PlayerInventories);
	}
}
