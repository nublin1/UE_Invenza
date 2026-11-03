//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/Interactable/PickupComponent.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Utility/InventoryGrantService.h"
#include "ActorComponents/InteractionComponent.h"
#include "UObject/StrongObjectPtr.h"

#include "Data/Items/itemBase.h"
#include "Utility/InvenzayUtility.h"
#include "World/PortableInventoryState.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
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
	DOREPLIFETIME(UPickupComponent, PortableContainer);
	DOREPLIFETIME(UPickupComponent, PickupItems);
	DOREPLIFETIME(UPickupComponent, bBundlePickup);
}

bool UPickupComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (ItemBase)
	{
		if (auto* Item = Cast<UItemBase>(ItemBase))
		{
			Item->RefreshPortableSummary();
		}
		bWroteSomething |= Channel->ReplicateSubobject(ItemBase, *Bunch, *RepFlags);
	}

	for (UObject* Item : PickupItems)
	{
		if (IsValid(Item) && Item != ItemBase)
		{
			if (auto* PortableItem = Cast<UItemBase>(Item))
			{
				PortableItem->RefreshPortableSummary();
			}
			bWroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
		}
	}
	return bWroteSomething;
}

void UPickupComponent::OnRegister()
{
	Super::OnRegister();
	RefreshAppearance();
	UpdateInteractableData();
}

void UPickupComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheVisualMesh();

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


void UPickupComponent::InitializeItems(const TArray<FInitItemsEntry>& Items)
{
	if (!GetOwner()->HasAuthority() || bCollecting || GetOwner()->IsActorBeingDestroyed())
	{
		return;
	}
	PortableContainer = nullptr;
	InitialItems = Items;
	InitialItem = FInitItemsEntry();
	PickupItems.Reset();
	ItemBase = nullptr;
	bContentsInitialized = false;
	bIsPendingDestruction = false;
	InitializePickupComponent();
}

void UPickupComponent::InitializeDrop(FInitItemsEntry ItemToDrop)
{
	PortableContainer = nullptr;
	InitialItem = ItemToDrop;
	InitialItems.Reset();
	PickupItems.Reset();
	bBundlePickup = false;
	bContentsInitialized = false;
	bIsPendingDestruction = false;
	ItemBase = nullptr;

	InitializePickupComponent();

	UpdateInteractableData();
}

UObject* UPickupComponent::GetItemData()
{
	if (bIsPendingDestruction || bBundlePickup)
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
	if (bContentsInitialized || IsValid(ItemBase) || !GetOwner()->HasAuthority())
	{
		return;
	}
	bBundlePickup = !InitialItems.IsEmpty();
	if (!bBundlePickup)
	{
		if (InitialItem.Item.IsNull() || InitialItem.Amount <= 0)
		{
			return;
		}
		ItemBase = UItemFactory::CreateItemByHandle(this, InitialItem.Item, InitialItem.Amount);
		bContentsInitialized = IsValid(ItemBase);
	}
	else
	{
		TArray<TObjectPtr<UObject>> Created;
		for (const auto& Entry : InitialItems)
		{
			if (Entry.Item.IsNull() || Entry.Amount <= 0)
			{
				UE_LOG(LogTemp, Error, TEXT("Bundle pickup %s contains an invalid item entry; entry discarded."), *GetNameSafe(GetOwner()));
				continue;
			}
			int32 Remaining = Entry.Amount;
			while (Remaining > 0)
			{
				auto* Item = UItemFactory::CreateItemByHandle(this, Entry.Item, 1);
				if (!Item)
				{
					UE_LOG(LogTemp, Error, TEXT("Bundle pickup %s: item %s is unavailable; %d units discarded."),
						*GetNameSafe(GetOwner()), *Entry.Item.RowName.ToString(), Remaining);
					break;
				}
				const auto Meta = IObjectDataProvider::Execute_GetItemRef(Item);
				const int32 Limit = IObjectDataProvider::Execute_IsStackable(Item)
					? FMath::Max(1, Meta.ItemNumeraticData.MaxStackSizeInCharacter) : 1;
				const int32 Amount = FMath::Min(Remaining, Limit);
				Item->SetQuantityForTransfer(Amount);
				Created.Add(Item);
				Remaining -= Amount;
			}
		}
		PickupItems = MoveTemp(Created);
		bContentsInitialized = true;
		ItemBase = PickupItems.IsEmpty() ? nullptr : PickupItems[0];
	}
	ContentsChanged();
}

TArray<UObject*> UPickupComponent::GetPickupItems() const
{
	TArray<UObject*> Result;
	if (bBundlePickup)
	{
		for (UObject* Item : PickupItems)
		{
			if (IsValid(Item) && IObjectDataProvider::Execute_GetQuantity(Item) > 0)
			{
				Result.Add(Item);
			}
		}
	}
	else if (IsValid(ItemBase) && IObjectDataProvider::Execute_GetQuantity(ItemBase) > 0)
	{
		Result.Add(ItemBase);
	}
	return Result;
}

void UPickupComponent::OnRep_ItemBase()
{
	for (auto* Object : GetPickupItems())
	{
		if (auto* Item = Cast<UItemBase>(Object))
		{
			Item->OnItemDataReplicated.AddUniqueDynamic(this, &ThisClass::ItemVisualDataChanged);
		}
	}
	RefreshAppearance();
	UpdateInteractableData();
}

void UPickupComponent::CacheVisualMesh()
{
	if (CachedMesh)
	{
		return;
	}
	CachedMesh = Cast<UStaticMeshComponent>(VisualMeshComponent.GetComponent(GetOwner()));
	if (!CachedMesh || CachedMesh->GetOwner() != GetOwner())
	{
		CachedMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
	}
	if (CachedMesh)
	{
		AuthoredMeshScale = CachedMesh->GetRelativeScale3D();
		for (int32 Index = 0; Index < CachedMesh->GetNumMaterials(); ++Index)
		{
			AuthoredMaterials.Add(CachedMesh->GetMaterial(Index));
		}
	}
}

void UPickupComponent::RefreshAppearance()
{
	CacheVisualMesh();
	if (!CachedMesh)
	{
		return;
	}
	if (MeshSource == EPickupMeshSource::CustomMesh)
	{
		if (CustomMesh)
		{
			CachedMesh->SetStaticMesh(CustomMesh);
		}
		CachedMesh->SetRelativeScale3D(AuthoredMeshScale * CustomMeshScale);
		for (int32 Index = 0; Index < CachedMesh->GetNumMaterials(); ++Index)
		{
			CachedMesh->SetMaterial(Index, AuthoredMaterials.IsValidIndex(Index)
				? AuthoredMaterials[Index].Get() : CachedMesh->GetStaticMesh()->GetMaterial(Index));
		}
		if (!CustomMaterials.IsEmpty())
		{
			UInvenzayUtility::ApplyMaterialOverrides(CachedMesh, CustomMaterials);
		}
	}
	else if (IsValid(ItemBase) && ItemBase->Implements<UObjectDataProvider>())
	{
		const auto Assets = IObjectDataProvider::Execute_GetItemRef(ItemBase).ItemAssetData;
		if (Assets.Mesh)
		{
			CachedMesh->SetStaticMesh(Assets.Mesh);
			CachedMesh->SetRelativeScale3D(AuthoredMeshScale * Assets.MeshScale);
			UInvenzayUtility::ApplyMaterialOverrides(CachedMesh, Assets.Materials);
		}
	}
}

void UPickupComponent::ItemVisualDataChanged(UObject* Item)
{
	if (Item == ItemBase)
	{
		OnRep_ItemBase();
	}
}

void UPickupComponent::UpdateInteractableData()
{
	if (!bAuthoredInteractionCached)
	{
		if (const auto* Data = InteractableDataMap.Find(EInteractionType::Primary))
		{
			AuthoredPrimaryData = *Data;
		}
		bAuthoredInteractionCached = true;
	}
	FInteractableData PrimaryData = AuthoredPrimaryData;
	PrimaryData.DefaultInteractableType = EInteractableType::Pickup;
	if (PrimaryData.Action.IsEmpty())
	{
		PrimaryData.Action = GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("PickupComponent.Literal0"));
	}
	PrimaryData.Quantity = -1;
	if (ItemBase && ItemBase->Implements<UObjectDataProvider>())
	{
		if (!bBundlePickup)
		{
			PrimaryData.Quantity = IObjectDataProvider::Execute_GetQuantity(ItemBase);
		}
		if (PrimaryData.Name.IsEmpty())
		{
			PrimaryData.Name = IObjectDataProvider::Execute_GetItemDisplayText(ItemBase);
		}
	}
	if (auto* Portable = Cast<UItemBase>(ItemBase); !bBundlePickup && Portable && Portable->IsPortableInventory())
	{
		PrimaryData.DefaultInteractableType = EInteractableType::Container;
		PrimaryData.Action = GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("ContainerComponent.Literal0"));
		FInteractableData Equip = PrimaryData;
		Equip.Action = IObjectDataProvider::Execute_GetItemRef(Portable).ItemInventoryData.EquipAction;
		if (Equip.Action.IsEmpty())
		{
			if (const auto* Settings = UInvenzayUtility::GetInvenzaGlobalSettings(this))
			{
				if (const auto* Action = Settings->ModalActions.Find(EObjectInteractionType::Equip))
				{
					Equip.Action = Action->DisplayText;
				}
			}
		}
		InteractableDataMap.Add(EInteractionType::Secondary, Equip);
	}
	else
	{
		InteractableDataMap.Remove(EInteractionType::Secondary);
	}
	InteractableDataMap.Add(EInteractionType::Primary, PrimaryData);
}

void UPickupComponent::ContentsChanged()
{
	if (bBundlePickup)
	{
		PickupItems.RemoveAll([](const auto& Object)
		{
			return !IsValid(Object) || IObjectDataProvider::Execute_GetQuantity(Object) <= 0;
		});
		ItemBase = PickupItems.IsEmpty() ? nullptr : PickupItems[0];
	}
	OnRep_ItemBase();
	if (GetOwner()->HasAuthority())
	{
		GetOwner()->ForceNetUpdate();
		if (GetWorld() && GetWorld()->GetGameInstance())
		{
			auto* Save = GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>();
			Save->RegisterPickup(this);
			Save->MarkSessionChanged();
		}
	}
}

bool UPickupComponent::CollectItems(UIInventoryManager* Manager)
{
	if (!Manager || !GetOwner()->HasAuthority() || GetOwner()->IsActorBeingDestroyed() || bCollecting ||
		!IsInteractionOwnedBy(Manager->GetOwner()) ||
		!UInteractionComponent::IsWithinInteractionDistance(Manager->GetOwner(), this))
	{
		return false;
	}
	InitializePickupComponent();
	const auto Items = GetPickupItems();
	if (Items.IsEmpty())
	{
		if (bBundlePickup && bContentsInitialized)
		{
			OnPickedUp();
			return true;
		}
		return false;
	}
	TGuardValue<bool> Guard(bCollecting, true);
	const TArray<UInventoryBase*> Targets = bBundlePickup ? Manager->GetReceivingInventories()
		: TArray<UInventoryBase*>{Manager->GetPawnMainInventory()};
	int32 DropIndex = 0;
	for (auto* Object : Items)
	{
		auto* Item = Cast<UItemBase>(Object);
		if (!Item)
		{
			continue;
		}
		FInventoryGrantPolicy Policy;
		Policy.bLogOverflowFailure = false;
		Policy.bAllowPartial = bBundlePickup;
		Policy.Overflow = bBundlePickup ? EInventoryGrantOverflow::DropAndDiscardOnFailure
			: EInventoryGrantOverflow::Retain;
		Policy.DropTransform = FTransform(GetOwner()->GetActorRotation(), GetOwner()->GetActorLocation() +
			GetOwner()->GetActorRotation().RotateVector(OverflowDropOffset) * static_cast<float>(++DropIndex));
		const auto Result = FInventoryGrantService::Grant(Item, Targets, Manager->GetOwner(), Policy);
		if (Result.Discarded > 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Bundle pickup %s failed to drop %d x %s (%s); remainder discarded."),
				*GetNameSafe(GetOwner()), Result.Discarded, *IObjectDataProvider::Execute_GetItemID(Item).ToString(),
				*Item->GetPersistentInstanceID().ToString());
		}
	}
	ContentsChanged();
	if (GetPickupItems().IsEmpty())
	{
		OnPickedUp();
		return true;
	}
	return false;
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
	RestorePersistentItems(Item ? TArray<UObject*>{Item} : TArray<UObject*>(), false);
}

void UPickupComponent::RestorePersistentItems(const TArray<UObject*>& Items, bool bBundle)
{
	bBundlePickup = bBundle;
	bContentsInitialized = true;
	bIsPendingDestruction = false;
	PickupItems.Reset();
	for (auto* Item : Items)
	{
		PickupItems.Add(Item);
	}
	ItemBase = Items.IsEmpty() ? nullptr : Items[0];
	PortableContainer = nullptr;
	RefreshPortableContainer();
	ContentsChanged();
}

void UPickupComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (GetOwner()->HasAuthority() && GetWorld() && GetWorld()->GetGameInstance())
	{
		GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->UnregisterPickup(this, Reason);
	}
	Super::EndPlay(Reason);
}


void UPickupComponent::RefreshPortableContainer()
{
	if (auto* Item = Cast<UItemBase>(ItemBase); !bBundlePickup && Item && Item->IsPortableInventory())
	{
		auto* State = GetOwner()->HasAuthority() ? Item->EnsurePortableState() : Item->GetPortableState();
		if (State && GetOwner()->HasAuthority())
		{
			State->SetCarrier(GetOwner());
			Item->RefreshPortableSummary();
		}
		PortableContainer = State ? State->GetInventory() : nullptr;
	}
	UpdateInteractableData();
}
