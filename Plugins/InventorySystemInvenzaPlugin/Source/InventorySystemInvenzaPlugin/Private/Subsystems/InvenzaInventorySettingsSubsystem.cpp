//  Nublin Studio 2026 All Rights Reserved.


#include "Subsystems/InvenzaInventorySettingsSubsystem.h"

#include "Data/CraftSystem/CraftingStructs.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "UI/Drag/DragContainerWidget.h"

void UInvenzaInventorySettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	 auto DevSettings = GetDefault<UInvenzaInventoryDeveloperSettings>();

	if (DevSettings && !DevSettings->InventorySettingsAsset.IsNull())
	{
		Settings = DevSettings->InventorySettingsAsset.LoadSynchronous();
	}
	
	ValidateSettings();
}

const UInvenzaInventorySettingsAsset* UInvenzaInventorySettingsSubsystem::GetSettingsStatic(
	const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetSettingsStatic: Failed to get World from ContextObject."));
		return nullptr;
	}
	
	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetSettingsStatic: GameInstance is null."));
		return nullptr;
	}
	
	auto* Subsystem = GI->GetSubsystem<UInvenzaInventorySettingsSubsystem>();
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetSettingsStatic: Could not find UInvenzaInventorySettingsSubsystem."));
		return nullptr;
	}
	
	const UInvenzaInventorySettingsAsset* Settings = Subsystem->GetSettings();
	if (!Settings)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetSettingsStatic: Settings asset is null inside the subsystem."));
		return nullptr;
	}

	return Settings;
}

void UInvenzaInventorySettingsSubsystem::ValidateSettings() const
{
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("[InventorySettings] Settings Asset is null."));
		return;
	}

	// Inventory

	if (!Settings->PickupClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySettings] PickupClass is not assigned."));
	}

	if (Settings->CurrencyItemClass.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySettings] CurrencyItemClass is not assigned."));
	}

	if (!Settings->CurrencyGameplayTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySettings] CurrencyGameplayTag is invalid."));
	}

	if (!Settings->AnyCategoryGameplayTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySettings] AnyCategoryGameplayTag is invalid."));
	}

	// Widgets

	if (!Settings->DragContainerWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySettings] DragContainerWidgetClass is not assigned."));
	}

	// Craft

	if (!Settings->Block_NoResources.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySettings] Block_NoResources is invalid."));
	}

	if (Settings->AvailableBlockReasons.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySettings] AvailableBlockReasons is empty."));
	}

	for (const FBlockReasonData& Reason : Settings->AvailableBlockReasons)
	{
		if (!Reason.Tag.IsValid())
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[InventorySettings] BlockReason has an invalid Tag."));
		}

		if (Reason.Message.IsEmpty())
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[InventorySettings] BlockReason '%s' has an empty Message."),
				Reason.Tag.IsValid() ? *Reason.Tag.ToString() : TEXT("<Invalid Tag>"));
		}
	}
}
