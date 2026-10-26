//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/Interactable/PickupComponent.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#include "Data/Items/itemBase.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/ActorChannel.h"
#include "Factory/ItemFactory.h"
#include "Net/UnrealNetwork.h"
#include "Utility/InterfaceUtils.h"

UPickupComponent::UPickupComponent() : InitialItem()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void UPickupComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPickupComponent, ItemBase);
}

bool UPickupComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (ItemBase)
	{
		bWroteSomething |= Channel->ReplicateSubobject(ItemBase, *Bunch, *RepFlags);
	}

	return bWroteSomething;
}

void UPickupComponent::OnRegister()
{
	Super::OnRegister();
	UpdateInteractableData();
}

void UPickupComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>();

	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(
		this,
		[this]()
		{
			if (!GetWorld() || GetWorld()->bIsTearingDown)
			{
				return;
			}
			InitializePickupComponent();
			UpdateInteractableData();
			if (GetOwner()->HasAuthority() && GetWorld()->GetGameInstance())
			{
				GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->RegisterPickup(this);
			}
		}));
}

void UPickupComponent::BeginFocus()
{
	if (CachedMesh)
	{
		CachedMesh->SetRenderCustomDepth(true);
	}
}

void UPickupComponent::EndFocus()
{
	if (CachedMesh)
	{
		CachedMesh->SetRenderCustomDepth(false);
	}
}

void UPickupComponent::HandleInteract(UInteractionComponent* InteractionComponent)
{
	Super::HandleInteract(InteractionComponent);
}

void UPickupComponent::InitializeDrop(FInitItemsEntry ItemToDrop)
{
	InitialItem = ItemToDrop;
	ItemBase = nullptr;

	InitializePickupComponent();

	UpdateInteractableData();
}

UObject* UPickupComponent::GetItemData()
{
	if (bIsPendingDestruction)
	{
		return nullptr;
	}

	bIsPendingDestruction = true;

	return ItemBase;
}

void UPickupComponent::OnPickedUp()
{
	GetOwner()->Destroy();
}

void UPickupComponent::InitializePickupComponent()
{
	if (IsValid(ItemBase))
	{
		return;
	}
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (InitialItem.Item.RowName.IsNone())
	{
		return;
	}

	UObject* NewItem = UItemFactory::CreateItemByHandle(this, InitialItem.Item, InitialItem.Amount);
	if (!NewItem)
	{
		UE_LOG(LogTemp, Error, TEXT("Item creation failed for %s!"), *InitialItem.Item.RowName.ToString());
		return;
	}

	ItemBase = NewItem;

	const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(ItemBase);

	if (CachedMesh && Meta.ItemAssetData.Mesh)
	{
		CachedMesh->SetStaticMesh(Meta.ItemAssetData.Mesh);
	}
}

void UPickupComponent::OnRep_ItemBase()
{
	UpdateInteractableData();
}

void UPickupComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();

	FInteractableData PrimaryData;
	PrimaryData.DefaultInteractableType = EInteractableType::Pickup;
	PrimaryData.Action = GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("PickupComponent.Literal0"));
	PrimaryData.Quantity = -1;
	InteractableDataMap.Add(EInteractionType::Primary, PrimaryData);

	if (ItemBase &&
		UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemBase, TEXT("UpdateInteractableData")))
	{
		PrimaryData.Quantity = IObjectDataProvider::Execute_GetQuantity(ItemBase);

		if (PrimaryData.Name.IsEmpty())
		{
			const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(ItemBase);
			PrimaryData.Name = Meta.ItemTextData.DisplayName;
		}
	}
}

FString UPickupComponent::GetPersistentSaveKey() const
{
	if (!PersistentSaveKey.IsEmpty())
	{
		return PersistentSaveKey;
	}
	if (PersistentPickupID.IsValid())
	{
		return TEXT("Pickup:") + PersistentPickupID.ToString();
	}
	return TEXT("Pickup:") + UWorld::RemovePIEPrefix(GetOwner()->GetPathName());
}

void UPickupComponent::RestorePersistentItem(UObject* Item)
{
	ItemBase = Item;
	bIsPendingDestruction = false;
	if (Item && CachedMesh)
	{
		const auto Metadata = IObjectDataProvider::Execute_GetItemRef(Item);
		if (Metadata.ItemAssetData.Mesh)
		{
			CachedMesh->SetStaticMesh(Metadata.ItemAssetData.Mesh);
		}
	}
	OnRep_ItemBase();
}

void UPickupComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (GetOwner()->HasAuthority() && GetWorld() && GetWorld()->GetGameInstance())
	{
		GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->UnregisterPickup(this, Reason);
	}
	Super::EndPlay(Reason);
}
