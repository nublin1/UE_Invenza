//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/UIInventoryManager.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "TimerManager.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/Interactable/CharacterInventoryAccessComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interface/Inventory/InvUIProvider.h"
#include "UI/Inventory/CharacterInventoryExchangeWidget.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"
#include "UI/Inventory/UInventoryBaseWidget.h"
#include "UI/Inventory/EquipmentInventoryWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

bool UIInventoryManager::IsCharacterInventoryAccessible(UInventoryBase* Inventory) const
{
	if (!IsValid(CharacterInventoryAccess) || CharacterInventoryRevision == ClosedCharacterInventoryRevision ||
		!CharacterInventoryAccess->ContainsInventory(Inventory))
	{
		return false;
	}
	return !GetOwner()->HasAuthority() ||
		   (TrackedInteractionTarget == CharacterInventoryAccess &&
			CharacterInventoryAccess->IsAccessedBy(GetOwner()) &&
			UInteractionComponent::IsWithinInteractionDistance(GetOwner(), CharacterInventoryAccess));
}

void UIInventoryManager::OnRep_CharacterInventoryAccess()
{
	if (!GetWorld() || bEndingPlay || GetWorld()->bIsTearingDown || !OwnerPawnRef ||
		!OwnerPawnRef->IsLocallyControlled())
	{
		return;
	}
	if (!CharacterInventoryAccess || CharacterInventoryRevision == ClosedCharacterInventoryRevision)
	{
		ClearCharacterInventoryUI();
		return;
	}
	if (!GetWorld()->GetTimerManager().IsTimerActive(CharacterInventoryUITimer))
	{
		CharacterInventoryUITimer =
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::RefreshCharacterInventoryUI);
	}
}

void UIInventoryManager::RefreshCharacterInventoryUI()
{
	if (!IsValid(CharacterInventoryAccess) || !GetWorld() || bEndingPlay || GetWorld()->bIsTearingDown ||
		!ItemCollectionRef || !UIInvProvider)
	{
		return;
	}
	auto* Actor = CharacterInventoryAccess->GetOwner();
	auto* Collection = IsValid(Actor) ? Actor->FindComponentByClass<UInventoryCollectionComponent>() : nullptr;
	if (!Collection)
	{
		return;
	}
	if (ObservedCharacterCollection != Collection)
	{
		ClearCharacterInventoryUI();
		ObservedCharacterCollection = Collection;
		Collection->OnActorInventoriesChanged.AddUniqueDynamic(this, &ThisClass::OnRep_CharacterInventoryAccess);
	}
	if (CharacterInventoryWindow)
	{
		return;
	}
	auto* Bag = CharacterInventoryAccess->GetMainLootContainer().Get();
	auto* Gear = CharacterInventoryAccess->GetEquipmentInventory();
	auto* PlayerGear = GlobalSettings ? ItemCollectionRef->GetInventoryByTag(GlobalSettings->EquipmentInvTag) : nullptr;
	TArray<UInventoryBase*> Inventories{Gear, Bag, PlayerGear, MainPawnInventoryRef};
	for (auto* Inventory : Inventories)
	{
		if (!IsValid(Inventory))
		{
			return;
		}
		Inventory->OnConfigurationChanged.AddUniqueDynamic(this, &ThisClass::OnRep_CharacterInventoryAccess);
		if (!Inventory->GetItemCollectionLinked())
		{
			return;
		}
	}
	for (auto* Inventory : Inventories)
	{
		auto* Widget = ItemCollectionRef->GetContainerWidget(Inventory);
		if (!Widget)
		{
			Widget = CreateInventoryWidget(Inventory);
		}
		if (!Widget)
		{
			ClearCharacterInventoryUI();
			// Configuration and subobject references can arrive in separate replication updates.
			ObservedCharacterCollection = Collection;
			Collection->OnActorInventoriesChanged.AddUniqueDynamic(this, &ThisClass::OnRep_CharacterInventoryAccess);
			for (auto* Pending : Inventories)
			{
				Pending->OnConfigurationChanged.AddUniqueDynamic(this, &ThisClass::OnRep_CharacterInventoryAccess);
			}
			return;
		}
		CharacterInventoryWidgets.Add(Widget);
		if (Inventory->GetItemCollectionLinked() == ItemCollectionRef)
		{
			if (auto* OriginalSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
			{
				auto& Placement = CharacterInventoryOriginalPlacements.AddDefaulted_GetRef();
				Placement.Widget = Widget;
				Placement.Parent = Cast<UCanvasPanel>(Widget->GetParent());
				Placement.Layout = OriginalSlot->GetLayout();
				Placement.bAutoSize = OriginalSlot->GetAutoSize();
				Placement.ZOrder = OriginalSlot->GetZOrder();
				Placement.Visibility = Widget->GetVisibility();
			}
		}
	}
	if (auto* EquipmentPanel = Cast<UEquipmentInventoryWidget>(CharacterInventoryWidgets[0]))
	{
		EquipmentPanel->SetPreviewActor(Actor);
	}
	const auto WindowClass =
		GetDefault<UInvenzaInventoryDeveloperSettings>()->CharacterInventoryWindowClass.LoadSynchronous();
	if (!WindowClass)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterInventoryWindowClass is not configured."));
		ClearCharacterInventoryUI();
		return;
	}
	CharacterInventoryWindow = CreateWidget<UCharacterInventoryExchangeWidget>(
		Cast<APlayerController>(OwnerPawnRef->GetController()), WindowClass);
	CharacterInventoryWindow->SetManager(this);
	CharacterInventoryWindow->SetInventories(CharacterInventoryAccess->CharacterDisplayName,
											 {CharacterInventoryWidgets[0], CharacterInventoryWidgets[1]},
											 {CharacterInventoryWidgets[2], CharacterInventoryWidgets[3]});
	if (!UIInvProvider->AttachStorageWindow(CharacterInventoryWindow))
	{
		CharacterInventoryWindow->AddToViewport(20);
	}
	for (UInventoryContainerWidget* Widget : CharacterInventoryWidgets)
	{
		Widget->ReDrawRequest();
	}
}

void UIInventoryManager::ClearCharacterInventoryUI()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(CharacterInventoryUITimer);
	}
	if (ObservedCharacterCollection)
	{
		ObservedCharacterCollection->OnActorInventoriesChanged.RemoveDynamic(
			this, &ThisClass::OnRep_CharacterInventoryAccess);
		for (auto* Inventory : ObservedCharacterCollection->GetActorInventories())
		{
			if (IsValid(Inventory))
			{
				Inventory->OnConfigurationChanged.RemoveDynamic(this, &ThisClass::OnRep_CharacterInventoryAccess);
			}
		}
	}
	ObservedCharacterCollection = nullptr;
	if (CharacterInventoryWindow)
	{
		auto* Window = CharacterInventoryWindow.Get();
		CharacterInventoryWindow = nullptr;
		Window->SetManager(nullptr);
		Window->ClearInventories();
		Window->RemoveFromParent();
	}
	for (UInventoryContainerWidget* Widget : CharacterInventoryWidgets)
	{
		if (!IsValid(Widget))
		{
			continue;
		}
		auto* InventoryWidget = Widget->GetInventoryWidgetFromContainerSlot();
		auto* Inventory = InventoryWidget ? InventoryWidget->GetInventoryRef() : nullptr;
		if (!Inventory)
		{
			continue;
		}
		Inventory->OnConfigurationChanged.RemoveDynamic(this, &ThisClass::OnRep_CharacterInventoryAccess);
		const bool bOwnInventory = Inventory->GetItemCollectionLinked() == ItemCollectionRef;
		const bool bRestore = !bEndingPlay && GetWorld() && !GetWorld()->bIsTearingDown && IsValid(GetOwner()) &&
							  !GetOwner()->IsActorBeingDestroyed();
		if (bOwnInventory && bRestore && UIInvProvider)
		{
			const auto* Placement = CharacterInventoryOriginalPlacements.FindByPredicate(
				[Widget](const auto& Entry)
				{
					return Entry.Widget == Widget;
				});
			if (Placement && IsValid(Placement->Parent))
			{
				auto* RestoredSlot = Placement->Parent->AddChildToCanvas(Widget);
				RestoredSlot->SetLayout(Placement->Layout);
				RestoredSlot->SetAutoSize(Placement->bAutoSize);
				RestoredSlot->SetZOrder(Placement->ZOrder);
				Widget->SetVisibility(Placement->Visibility);
				Widget->ReDrawRequest();
			}
			else
			{
				UIInvProvider->AddPawnInvContainerWidget(Widget);
			}
		}
		else
		{
			Widget->ReleaseInventory();
			if (!bOwnInventory && ItemCollectionRef)
			{
				ItemCollectionRef->UnregisterContainerWidget(Inventory);
			}
		}
	}
	CharacterInventoryWidgets.Reset();
	CharacterInventoryOriginalPlacements.Reset();
}

void UIInventoryManager::CloseCharacterInventory()
{
	auto* Expected = CharacterInventoryAccess.Get();
	const uint32 Revision = CharacterInventoryRevision;
	ClosedCharacterInventoryRevision = Revision;
	if (!Expected)
	{
		return;
	}
	if (InteractionComponent && InteractionComponent->GetActiveInteractable() == Expected)
	{
		InteractionComponent->StopInteract();
	}
	ClearCharacterInventoryUI();
	if (GetOwner()->HasAuthority())
	{
		Server_CloseCharacterInventory_Implementation(Expected, Revision);
	}
	else
	{
		Server_CloseCharacterInventory(Expected, Revision);
	}
}

void UIInventoryManager::Server_CloseCharacterInventory_Implementation(
	UCharacterInventoryAccessComponent* ExpectedAccess, uint32 Revision)
{
	if (ExpectedAccess && CharacterInventoryAccess == ExpectedAccess && Revision == CharacterInventoryRevision)
	{
		HandleClearInteraction(ExpectedAccess, EInteractionType::Primary);
	}
}
