//  Nublin Studio 2026 All Rights Reserved.

#include "UI/Controls/ControlsHelpLibrary.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/InteractionComponent.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "UObject/UObjectIterator.h"
#include "InputAction.h"
#include "InputTriggers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

#define LOCTEXT_NAMESPACE "ControlsHelp"
static FText GetControlsHelpUnassignedText()
{
	return GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.Unassigned"));
}
static FText GetControlsHelpKeyText(const FKey& Key)
{
	return Key.IsValid() ? Key.GetDisplayName() : GetControlsHelpUnassignedText();
}

static FText ResolveControlsHelpActionKeys(const UEnhancedInputLocalPlayerSubsystem* Input, const UInputAction* Action,
										   TSet<const UInputAction*>& Visiting)
{
	if (!Input || !Action || Visiting.Contains(Action))
	{
		return GetControlsHelpUnassignedText();
	}
	Visiting.Add(Action);
	TArray<FString> Alternatives;
	for (const FKey& Key : Input->QueryKeysMappedToAction(Action))
	{
		if (!Key.IsValid())
		{
			continue;
		}
		TArray<FString> Parts;
		auto AddChords = [&](const TArray<TObjectPtr<UInputTrigger>>& Triggers)
		{
			for (const auto& Trigger : Triggers)
			{
				if (const auto* Chord = Cast<UInputTriggerChordAction>(Trigger);
					Chord && !Cast<UInputTriggerChordBlocker>(Trigger))
				{
					const FString Keys = ResolveControlsHelpActionKeys(Input, Chord->ChordAction, Visiting).ToString();
					Parts.AddUnique(Keys.Contains(TEXT(" / ")) ? TEXT("(") + Keys + TEXT(")") : Keys);
				}
			}
		};
		AddChords(Action->Triggers);
		// Remappable mappings are returned with the player's current key and triggers.
		bool bFoundPlayerMapping = false;
		for (const auto& Mapping : Input->GetAllPlayerMappableActionKeyMappings())
		{
			if (Mapping.Action == Action && Mapping.Key == Key)
			{
				AddChords(Mapping.Triggers);
				bFoundPlayerMapping = true;
			}
		}
		if (!bFoundPlayerMapping)
		{
			// Non-remappable keys keep their context key. Only inspect active contexts.
			const FEnhancedActionKeyMapping* BestMapping = nullptr;
			int32 BestPriority = MIN_int32;
			for (TObjectIterator<UInputMappingContext> It; It; ++It)
			{
				int32 Priority = 0;
				if (!Input->HasMappingContext(*It, Priority) || Priority < BestPriority)
				{
					continue;
				}
				for (const auto& Mapping : It->GetMappings())
				{
					if (Mapping.Action == Action && Mapping.Key == Key)
					{
						BestMapping = &Mapping;
						BestPriority = Priority;
					}
				}
			}
			if (BestMapping)
			{
				AddChords(BestMapping->Triggers);
			}
		}
		Parts.Add(Key.GetDisplayName().ToString());
		Alternatives.AddUnique(FString::Join(Parts, TEXT(" + ")));
	}
	Visiting.Remove(Action);
	return Alternatives.IsEmpty() ? GetControlsHelpUnassignedText()
								  : FText::FromString(FString::Join(Alternatives, TEXT(" / ")));
}

FText UControlsHelpLibrary::GetActionKeys(APlayerController* Player, const UInputAction* Action)
{
	const auto* Local = Player ? Player->GetLocalPlayer() : nullptr;
	const auto* Subsystem = Local ? Local->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	TSet<const UInputAction*> Visiting;
	return ResolveControlsHelpActionKeys(Subsystem, Action, Visiting);
}

TArray<FControlsHelpRow> UControlsHelpLibrary::BuildRows(APlayerController* Player,
														 const UInvenzaInventorySettingsAsset* Settings)
{
	TArray<FControlsHelpRow> Rows;
	if (!Settings)
	{
		return Rows;
	}
	const FText General = GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.General"));
	const FText InventoryGroup =
		GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.Inventory"));
	const FText InteractionGroup =
		GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.Interaction"));
	TSet<const UInputAction*> Listed;
	auto AddAction = [&](FText Group, FText Label, const UInputAction* Action)
	{
		if (Action && Listed.Contains(Action))
		{
			return;
		}
		Rows.Add({Group, Label, GetActionKeys(Player, Action)});
		if (Action)
		{
			Listed.Add(Action);
		}
	};
	for (const auto& Entry : Settings->AdditionalControls)
	{
		if (!Entry.Label.IsEmpty())
		{
			AddAction(Entry.Group.IsEmpty() ? General : Entry.Group, Entry.Label, Entry.Action);
		}
	}
	if (Settings->ControlsHelpAction)
	{
		AddAction(General, GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.Help")),
				  Settings->ControlsHelpAction);
	}
	const APawn* Pawn = Player ? Player->GetPawn() : nullptr;
	if (!Settings->bIncludeInventoryControls || !Pawn)
	{
		return Rows;
	}
	if (const auto* Manager = Pawn->FindComponentByClass<UIInventoryManager>())
	{
		const auto UI = Manager->GetUISettings();
		if (UI.ToggleInventoryAction)
		{
			AddAction(InventoryGroup,
					  GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.OpenInventory")),
					  UI.ToggleInventoryAction);
		}
		if (UI.ToggleCraftAction)
		{
			AddAction(InventoryGroup,
					  GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.Crafting")),
					  UI.ToggleCraftAction);
		}
		Rows.Add({InventoryGroup,
				  GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.Select")),
				  GetControlsHelpKeyText(UI.ItemSelectKey)});
		Rows.Add({InventoryGroup, GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.Use")),
				  GetControlsHelpKeyText(UI.ItemUseKey)});
		Rows.Add({InventoryGroup, GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.Menu")),
				  GetControlsHelpKeyText(UI.ItemMenuKey)});
		auto AddTransfer = [&](FText Label, const UInputAction* Modifier)
		{
			if (!Modifier)
			{
				return;
			}
			Rows.Add({InventoryGroup, Label,
					  FText::Format(
						  GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.Combination")),
						  GetActionKeys(Player, Modifier), GetControlsHelpKeyText(UI.ItemSelectKey))});
		};
		AddTransfer(GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.Quick")),
					UI.IA_Mod_QuickGrab);
		AddTransfer(GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.AllSame")),
					UI.IA_Mod_GrabAllSame);
		if (UI.IA_RotateDraggedItem)
		{
			AddAction(InventoryGroup,
					  GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.Rotate")),
					  UI.IA_RotateDraggedItem);
		}
	}
	if (const auto* Interaction = Pawn->FindComponentByClass<UInteractionComponent>())
	{
		for (const auto& Binding : Interaction->GetKeyBindings())
		{
			FText Label;
			switch (Binding.Type)
			{
			case EInteractionType::Primary:
				Label = GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.Primary"));
				break;
			case EInteractionType::Secondary:
				Label = GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.Secondary"));
				break;
			default:
				Label =
					GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.OtherInteraction"));
				break;
			}
			AddAction(InteractionGroup, Label, Binding.Action);
		}
	}
	if (auto* Collection = Pawn->FindComponentByClass<UInventoryCollectionComponent>())
	{
		int32 Number = 0;
		for (auto* Inventory : Collection->GetActorInventories())
		{
			if (auto* Grid = Cast<USlotbasedInventory>(Inventory))
			{
				for (const auto* Slot : Grid->GetInventorySlots())
				{
					if (IsValid(Slot) && Slot->InventorySlotInfo.UseAction &&
						!Listed.Contains(Slot->InventorySlotInfo.UseAction.Get()))
					{
						AddAction(
							GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.Hotbar")),
							FText::Format(
								GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ControlsHelp.Slot")),
								++Number),
							Slot->InventorySlotInfo.UseAction.Get());
					}
				}
			}
		}
	}
	return Rows;
}
#undef LOCTEXT_NAMESPACE
