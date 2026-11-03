// Nublin Studio 2026 All Rights Reserved.

#include "World/PortableInventoryState.h"

#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/Crafting/CraftingComponent.h"
#include "Components/SceneComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Utility/InvenzayUtility.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "Data/Items/itemBase.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Subsystems/PortableInventorySubsystem.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "Net/UnrealNetwork.h"

APortableInventoryState::APortableInventoryState()
{
	bReplicates = true;
	bAlwaysRelevant = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	Collection = CreateDefaultSubobject<UInventoryCollectionComponent>(TEXT("PortableCollection"));
}

bool APortableInventoryState::InitializeForItem(UItemBase* Item)
{
	if (!HasAuthority() || !Item || !Item->IsPortableInventory())
	{
		return false;
	}
	SetCarrier(Item->GetTypedOuter<AActor>());
	Collection->SetRuntimeSaveIdentity(TEXT("Portable:") + Item->GetPersistentInstanceID().ToString());
	if (GetInventory())
	{
		return true;
	}
	const auto Metadata = IObjectDataProvider::Execute_GetItemRef(Item);
	const auto& Data = Metadata.ItemInventoryData;
	if (Data.Rows < 1 || Data.Columns < 1 || Data.Rows > 256 || Data.Columns > 256)
	{
		return false;
	}
	FInventoryStartupData Startup;
	Startup.Settings = GetDefault<UInvenzaInventoryDeveloperSettings>()->CharacterBagTemplate.ResolveSettings();
	Startup.Settings.InventorySlotBasedSettings.InitNumberRows = Data.Rows;
	Startup.Settings.InventorySlotBasedSettings.InitNumColumns = Data.Columns;
	Startup.Settings.InventoryMaxWeightCapacity = Data.MaxWeight;
	Startup.Settings.ContainerTitle = Metadata.ItemTextData.DisplayName;
	if (const auto* Settings = UInvenzayUtility::GetInvenzaGlobalSettings(this))
	{
		Startup.Settings.InventoryTag = Settings->MainInvTag;
	}
	Startup.Settings.InventoryType = EInventoryType::MainInventory;
	Startup.Settings.CustomInventoryID = TEXT("Contents");
	Startup.Settings.bCollectInvDataFromWidget = false;
	Startup.Settings.bIsReferenceContainer = false;
	auto* Inventory = UInventoryBase::CreateInventoryAdvanced(this, Startup, this, Collection);
	if (!Inventory)
	{
		return false;
	}
	Inventory->SetInventoryContainerID(Collection->GetPersistentOwnerKey() + TEXT(":Contents"));
	Collection->AddPawnInventory_Internal(Inventory);
	return true;
}

UInventoryBase* APortableInventoryState::GetInventory() const
{
	const auto Inventories = Collection->GetActorInventories();
	return Inventories.IsEmpty() ? nullptr : Inventories[0];
}

bool APortableInventoryState::HasContents() const
{
	return !Collection->GetItemLocations().Items.IsEmpty() || !Collection->GetRecoveryItems().IsEmpty();
}

bool APortableInventoryState::HasCraftObligations() const
{
	const auto* Registry = GetWorld()->GetSubsystem<UPortableInventorySubsystem>();
	return Registry && Registry->HasCraftObligations(GetInventory());
}

APortableInventoryState* APortableInventoryState::FindForItem(const UItemBase* Item)
{
	const auto* World = Item ? Item->GetWorld() : nullptr;
	const auto* Registry = World ? World->GetSubsystem<UPortableInventorySubsystem>() : nullptr;
	return Registry ? Registry->FindState(Item->GetPersistentInstanceID()) : nullptr;
}

bool APortableInventoryState::SetItemIdentity(const FGuid& ID)
{
	auto* Registry = GetWorld()->GetSubsystem<UPortableInventorySubsystem>();
	if (!Registry || !Registry->RegisterState(ID, this))
	{
		return false;
	}
	if (ItemIdentity != ID)
	{
		Registry->UnregisterState(ItemIdentity, this);
		ItemIdentity = ID;
	}
	return true;
}

void APortableInventoryState::OnRep_ItemIdentity()
{
	SetItemIdentity(ItemIdentity);
}

void APortableInventoryState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APortableInventoryState, ItemIdentity);
}

void APortableInventoryState::SetCarrier(AActor* Actor)
{
	if (HasAuthority() && IsValid(Actor) && Actor != this && Actor->GetWorld() == GetWorld())
	{
		Carrier = Actor;
		SetOwner(Actor);
		FlushNetDormancy();
		ForceNetUpdate();
	}
}

bool APortableInventoryState::CanReplicateTo(const AActor* RealViewer) const
{
	const auto* Controller = Cast<APlayerController>(RealViewer);
	const APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	if (!IsValid(Pawn))
	{
		return false;
	}
	if (Pawn == Carrier.Get())
	{
		return true;
	}
	const auto* Manager = Pawn->FindComponentByClass<UIInventoryManager>();
	return Manager && GetInventory() && Manager->IsInventoryAccessible(GetInventory());
}

bool APortableInventoryState::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget,
	const FVector& SrcLocation) const
{
	return CanReplicateTo(RealViewer);
}

bool APortableInventoryState::DestroyPermanently()
{
	if (!HasAuthority() || HasContents() || HasCraftObligations())
	{
		return false;
	}
	auto* Instance = GetGameInstance();
	auto* Registry = GetWorld()->GetSubsystem<UPortableInventorySubsystem>();
	if (!Destroy())
	{
		return false;
	}
	if (Registry)
	{
		Registry->UnregisterState(ItemIdentity, this);
	}
	if (Instance)
	{
		Instance->GetSubsystem<UInvenzaSaveSubsystem>()->ForgetPortableOwner(Collection);
	}
	return true;
}

void APortableInventoryState::EndPlay(const EEndPlayReason::Type Reason)
{
	if (auto* Registry = GetWorld()->GetSubsystem<UPortableInventorySubsystem>())
	{
		Registry->UnregisterState(ItemIdentity, this);
	}
	Super::EndPlay(Reason);
}
