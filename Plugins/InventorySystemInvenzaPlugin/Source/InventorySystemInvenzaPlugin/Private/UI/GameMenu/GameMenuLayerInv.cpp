// Nublin Studio 2026 All Rights Reserved.


#include "UI/GameMenu/GameMenuLayerInv.h"

#include "ActorComponents/UIInventoryManager.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/NamedSlot.h"
#include "Components/PanelWidget.h"
#include "Data/Inventory/InventoryBase.h"
#include "DragDrop/InvContainerDragDropOperation.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/MovableTitleBar/MovableTitleBar.h"
#include "UI/Craft/CraftControlPanel.h"
#include "UI/Craft/CraftDashboard.h"
#include "UI/Craft/CraftMenuChoose.h"
#include "UI/Inventory/Container/DualInventoryWidget.h"

UGameMenuLayerInv::UGameMenuLayerInv()
{
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
	if (!PawnInventories) return Result;

	for (int32 i = 0; i < PawnInventories->GetChildrenCount(); i++)
	{
		if (auto* InvContainer = Cast<UInventoryContainerWidget>(PawnInventories->GetChildAt(i)))
		{
			if (auto InventoryWidget = Cast<UUInventoryBaseWidget>(InvContainer->ContainerSlot->GetContent()))
				Result.Add(InventoryWidget);
		}
	}
	return Result;
}

TArray<UInventoryContainerWidget*> UGameMenuLayerInv::GetAllPawnInvContainers() const
{
	TArray<UInventoryContainerWidget*> Result;
	if (!PawnInventories) return Result;
	for (int32 i = 0; i < PawnInventories->GetChildrenCount(); i++)
	{
		if (auto* InvContainer = Cast<UInventoryContainerWidget>(PawnInventories->GetChildAt(i)))
			Result.Add(InvContainer);
	}

	return Result;
}

UPanelSlot* UGameMenuLayerInv::AddPawnInvContainerWidget(UInventoryContainerWidget* InvContainerWidgetToAdd) const
{
	if (!PawnInventories) return nullptr;

	UPanelSlot* InvContainerSlot = PawnInventories->AddChild(InvContainerWidgetToAdd);

	auto InvWidget = InvContainerWidgetToAdd->GetInventoryWidgetFromContainerSlot();
	auto InvSettings = InvWidget->GetInventoryRef()->GetInventorySettings();
	InvWidget->ReDrawAllItems();
	EInventoryType Type = InvSettings.InventoryType;

	if (InventoryDefaultPositions.Contains(Type))
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(InvContainerSlot))
		{
			CanvasSlot->SetAutoSize(true);
			CanvasSlot->SetSize(FVector2D(0,0));
			FVector2D NormalizedPos = InventoryDefaultPositions[Type];

			FVector2D ViewportSize;
			GEngine->GameViewport->GetViewportSize(ViewportSize);

			FVector2D RealPos = FVector2D(
				NormalizedPos.X * ViewportSize.X,
				NormalizedPos.Y * ViewportSize.Y
			);

			CanvasSlot->SetPosition(RealPos);
		}
	}

	if (InvSettings.bIsAlwaysVisible)
		InvContainerWidgetToAdd->SetVisibility(ESlateVisibility::Visible);
	else
		InvContainerWidgetToAdd->SetVisibility(ESlateVisibility::Collapsed);
	
	return InvContainerSlot;
}

void UGameMenuLayerInv::RemovePawnInvContainer(UInventoryContainerWidget* InvContainerToRemove) const
{
	if (!PawnInventories) return;

	for (int32 i = 0; i < PawnInventories->GetChildrenCount(); i++)
	{
		if (PawnInventories->GetChildAt(i) == InvContainerToRemove)
		{
			PawnInventories->RemoveChildAt(i);
			break;
		}
	}
}

void UGameMenuLayerInv::OpenDualInventoryView(UInventoryContainerWidget* ExternalContainerWidget,
	UInventoryContainerWidget* PlayerInventoryToShow)
{
	if (!DualInventoryWidget || !ExternalContainerWidget ||! PlayerInventoryToShow) return;
	
	BorrowedPlayerContainer = PlayerInventoryToShow;
	DualInventoryWidget->SetRightInventoryContainer(PlayerInventoryToShow);
	
	CurrentExternalContainer = ExternalContainerWidget;
	DualInventoryWidget->SetLeftInventoryContainer(ExternalContainerWidget);
	
	DualInventoryWidget->ApplyCornerAlignment();
	
	DualInventoryWidget->SetVisibility(ESlateVisibility::Visible);
	if (WorldDropZone) WorldDropZone->SetVisibility(ESlateVisibility::Visible);

	bInventoryOpen = true;
	UpdateInputMode();
}

void UGameMenuLayerInv::CloseDualInventoryView()
{
	if (!DualInventoryWidget) return;
	
	if (BorrowedPlayerContainer)
	{
		AddPawnInvContainerWidget(BorrowedPlayerContainer); 
		BorrowedPlayerContainer = nullptr;
	}
	
	if (CurrentExternalContainer)
	{
		CurrentExternalContainer->RemoveFromParent();
		CurrentExternalContainer = nullptr;
	}

	DualInventoryWidget->ClearAll();
	DualInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);

	bInventoryOpen = false;
	UpdateInputMode();
}

void UGameMenuLayerInv::OpenNestedInventory(UObject* Key, UInventoryContainerWidget* ContainerWidget,
	FVector2D ScreenPosition)
{
	if (!DualInventoryWidget || !ContainerWidget) return;
	DualInventoryWidget->AddFloatingInventory(Key, ContainerWidget, ScreenPosition);
}

void UGameMenuLayerInv::CloseNestedInventory(UObject* Key)
{
	if (!DualInventoryWidget) return;
	DualInventoryWidget->RemoveFloatingInventory(Key);
}

void UGameMenuLayerInv::ToggleInventoryLayout()
{
	if (!GetWorld())
		return;
	
	bInventoryOpen = !bInventoryOpen;
	
	const auto Containers = GetAllPawnInvContainers();
	if (Containers.IsEmpty())
		return;
	
	const ESlateVisibility NewVisibility = bInventoryOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	bool bAnyToggleableVisible = bInventoryOpen;

	if (WorldDropZone)
	{
		WorldDropZone->SetVisibility(NewVisibility);
	}
	
	for (auto* Container : Containers)
	{
		if (!Container)
			continue;
		
		auto InvWidget = Container->GetInventoryWidgetFromContainerSlot();
		if (InvWidget && InvWidget->GetInventoryRef() && InvWidget->GetInventoryRef()->GetInventorySettings().bIsAlwaysVisible)
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
		return nullptr;

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
		return nullptr;

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
	if (!WidgetToAdd || !PawnCraftWidgetsPanel) return nullptr;

	UPanelSlot* CraftSlot = PawnCraftWidgetsPanel->AddChild(WidgetToAdd);
	WidgetToAdd->SetVisibility(ESlateVisibility::Collapsed);

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(CraftSlot))
	{
		CanvasSlot->SetAutoSize(true);
		CanvasSlot->SetSize(FVector2D(0,0));
	}

	return CraftSlot;
}

UPanelSlot* UGameMenuLayerInv::AddPawnCraftChooseWidget(UInvenzaBaseWidget* WidgetToAdd)
{
	return AddPawnCraftDashboardWidget(WidgetToAdd);
}

void UGameMenuLayerInv::ToggleCraftMenuLayout()
{
	if (!GetWorld())
		return;

	bCraftMenuOpen = !bCraftMenuOpen;
	
	const ESlateVisibility NewVisibility = bCraftMenuOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	if (WorldDropZone)
	{
		WorldDropZone->SetVisibility(NewVisibility);
	}

	auto CraftMenu = GetCraftMenuDashboard();
	if (CraftMenu)
	{
		CraftMenu->SetVisibility(NewVisibility);
	}
	
	UpdateInputMode();
}

void UGameMenuLayerInv::BindCraftWidgets()
{
	auto Dashboard = Cast<UCraftDashboard>(GetCraftMenuDashboard());
	if (Dashboard && Dashboard->CraftControlPanel && Dashboard->CraftControlPanel->Btn_AddTask)
	{
		Dashboard->CraftControlPanel->Btn_AddTask->OnButtonClicked.AddDynamic(
			this,
			&UGameMenuLayerInv::HandleCraftMenuSwap
		);
	}

	auto Choose = Cast<UCraftMenuChoose>(GetCraftChoose());
	if (Choose)
	{
		Choose->MovableTitleBar->Button_Close->OnButtonClicked.AddDynamic(
			this,
			&UGameMenuLayerInv::HandleCraftMenuSwap);
	}
}

void UGameMenuLayerInv::HandleCraftMenuSwap(UUIButton* UIButton)
{
	const ECraftMenuState NewState =
		CraftMenuState == ECraftMenuState::Dashboard
		? ECraftMenuState::Choose
		: ECraftMenuState::Dashboard;

	SetCraftMenuState(NewState);
}

void UGameMenuLayerInv::SetCraftMenuState(ECraftMenuState NewState)
{
	CraftMenuState = NewState;

	auto Dashboard = GetCraftMenuDashboard();
	auto Choose = GetCraftChoose();

	if (Dashboard)
	{
		Dashboard->SetVisibility(
			NewState == ECraftMenuState::Dashboard
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed
		);
	}

	if (Choose)
	{
		Choose->SetVisibility(
			NewState == ECraftMenuState::Choose
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed
		);
	}
}

void UGameMenuLayerInv::UpdateInputMode()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
		return;

	if (bInventoryOpen || bCraftMenuOpen)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);

		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
	else
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
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
