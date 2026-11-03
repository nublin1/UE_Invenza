//  Nublin Studio 2025 All Rights Reserved.

#include "Data/Items/itemBase.h"

#include "ActorComponents/Interactable/PickupComponent.h"
#include "Data/ItemData.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "GameplayTagContainer.h"
#include "Net/UnrealNetwork.h"
#include "Utility/InvenzayUtility.h"
#include "World/PortableInventoryState.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "Data/Inventory/InventoryBase.h"
#include "Engine/World.h"

UItemBase::UItemBase() : ItemRef(), Quantity(0)
{
}

void UItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItemBase, ItemID);
	DOREPLIFETIME(UItemBase, PersistentInstanceID);
	DOREPLIFETIME(UItemBase, ItemRow);
	DOREPLIFETIME(UItemBase, Quantity);
	DOREPLIFETIME(UItemBase, ReservedQuantity);
	DOREPLIFETIME(UItemBase, PortableContentsWeight);
	DOREPLIFETIME(UItemBase, bPortableHasContents);
}

void UItemBase::InitItem(const FName ID, FItemData Data, int32 InQuantity)
{
	if (!PersistentInstanceID.IsValid())
	{
		PersistentInstanceID = FGuid::NewGuid();
	}
	this->ItemID = ID;
	this->SetItemRef(Data.ItemMetaData);
	if (InQuantity <= 0)
	{
		InQuantity = 1;
	}
	else if (InQuantity > ItemRef.ItemNumeraticData.MaxStackSizeInCharacter)
	{
		InQuantity = ItemRef.ItemNumeraticData.MaxStackSizeInCharacter;
	}
	Quantity = InQuantity;
	if (IsPortableInventory())
	{
		Quantity = 1;
	}
}

bool UItemBase::CanPerformAction_Implementation(EObjectInteractionType Action,
												const UInvenzaInventorySettingsAsset* SettingsAsset)
{
	const UInvenzaInventorySettingsAsset* Settings = SettingsAsset;

	if (!Settings)
	{
		Settings = UInvenzayUtility::GetInvenzaGlobalSettings(GetWorld());
	}

	if (!Settings)
	{
		return false;
	}

	switch (Action)
	{
	case EObjectInteractionType::UseItem:
		return ItemRef.ItemCategory == Settings->ConsumableGameplayTag;

	case EObjectInteractionType::Equip:
		return true;

	case EObjectInteractionType::Drop:
		return ItemRef.bIsDroppable;

	case EObjectInteractionType::Destroy:
		return ItemRef.bIsDeletable && !HasPortableContents() &&
			(!GetPortableState() || !GetPortableState()->HasCraftObligations());

	case EObjectInteractionType::Split:
		return Execute_IsStackable(this);

	default:
		return false;
	}
}

void UItemBase::UseItem_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("UseItem was called!"));

	if (OnUseItemDelegate.IsBound())
	{
		OnUseItemDelegate.Broadcast(this);
	}
}

UObject* UItemBase::DuplicateItem_Implementation()
{
	if (HasPortableContents())
	{
		return nullptr;
	}
	UItemBase* NewItem = NewObject<UItemBase>(GetOuter(), GetClass());

	if (!NewItem)
	{
		return nullptr;
	}

	NewItem->PersistentInstanceID = FGuid::NewGuid();
	NewItem->ItemID = ItemID;
	NewItem->ItemRef = ItemRef;
	NewItem->Quantity = Quantity;
	NewItem->ItemRow = ItemRow;
	NewItem->RestoreSaveState(CaptureSaveState());

	return NewItem;
}

UWorld* UItemBase::GetWorld() const
{
	const AActor* Actor = GetTypedOuter<AActor>();
	return Actor ? Actor->GetWorld() : nullptr;
}

bool UItemBase::IsPortableInventory() const
{
	const auto Category = GetDefault<UInvenzaInventoryDeveloperSettings>()->PortableInventoryCategory;
	return Category.IsValid() && ItemRef.ItemCategory.MatchesTag(Category);
}

APortableInventoryState* UItemBase::GetPortableState() const
{
	return IsPortableInventory() ? APortableInventoryState::FindForItem(this) : nullptr;
}

APortableInventoryState* UItemBase::EnsurePortableState()
{
	if (auto* Existing = GetPortableState())
	{
		return Existing;
	}
	if (!IsPortableInventory() || !GetWorld() || GetWorld()->GetNetMode() == NM_Client)
	{
		return nullptr;
	}
	auto* State = GetWorld()->SpawnActor<APortableInventoryState>();
	if (State && !State->InitializeForItem(this))
	{
		State->Destroy();
		return nullptr;
	}
	return State;
}

bool UItemBase::HasPortableContents() const
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_Client)
	{
		return bPortableHasContents;
	}
	const auto* State = GetPortableState();
	return State && State->HasContents();
}

void UItemBase::RefreshPortableSummary()
{
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client || !IsPortableInventory())
	{
		return;
	}
	const auto* State = GetPortableState();
	const auto* Inventory = State ? State->GetInventory() : nullptr;
	PortableContentsWeight = Inventory ? Inventory->GetInventoryTotalWeight() : 0.f;
	bPortableHasContents = State && State->HasContents();
}

void UItemBase::OnRep_PortableSummary()
{
	OnItemDataReplicated.Broadcast(this);
}

bool UItemBase::CanStoreInInventory(const UInventoryBase* Inventory) const
{
	const auto* State = GetPortableState();
	return !IsPortableInventory() || (!HasPortableContents() &&
		(!State || (!State->HasCraftObligations() && State->GetInventory() != Inventory)));
}

float UItemBase::GetItemSingleWeight_Implementation()
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_Client)
	{
		return ItemRef.ItemNumeraticData.Weight + PortableContentsWeight;
	}
	const auto* State = GetPortableState();
	const auto* Inventory = State ? State->GetInventory() : nullptr;
	return ItemRef.ItemNumeraticData.Weight + (Inventory ? Inventory->GetInventoryTotalWeight() : 0.f);
}

float UItemBase::GetItemStackWeight_Implementation()
{
	return Quantity * GetItemSingleWeight_Implementation();
}

UItemBase* UItemBase::CreateIdentityTransferCopy(UObject* NewOuter)
{
	if (!IsValid(NewOuter))
	{
		return nullptr;
	}
	auto* Copy = DuplicateObject<UItemBase>(this, NewOuter);
	Copy->ReservedAmounts.Reset();
	Copy->ReservedQuantity = 0;
	Copy->OnUseItemDelegate.Clear();
	Copy->OnAmountChangedDelegate.Clear();
	Copy->OnItemDataReplicated.Clear();
	return Copy;
}

EItemOrientationType UItemBase::GetInitialItemOrientation_Implementation()
{
	if (ItemRef.ItemNumeraticData.InventoryVerticalSlots > ItemRef.ItemNumeraticData.InventoryHorizontalSlots)
	{
		return EItemOrientationType::Vertical;
	}
	return EItemOrientationType::Horizontal;
}

FIntPoint UItemBase::GetItemSize_Implementation(EItemOrientationType Orientation)
{
	const int32 H = ItemRef.ItemNumeraticData.InventoryHorizontalSlots;
	const int32 V = ItemRef.ItemNumeraticData.InventoryVerticalSlots;

	EItemOrientationType Initial = GetInitialItemOrientation_Implementation();

	if (GetInitialItemOrientation_Implementation() == EItemOrientationType::Horizontal)
	{
		if (Orientation == Initial)
		{
			return FIntPoint(H, V);
		}
		else
		{
			return FIntPoint(V, H);
		}
	}

	if (Orientation == Initial)
	{
		return FIntPoint(H, V);
	}

	return FIntPoint(V, H);
}

FString UItemBase::CategoryToString_Implementation()
{
	return ItemRef.ItemCategory.ToString();
}

FString UItemBase::CategoryToShortString_Implementation()
{
	return ItemRef.ItemCategory.GetTagName().GetPlainNameString();
}

int32 UItemBase::GetFreeAmount_Implementation() const
{
	return FMath::Max(0, Quantity - ReservedQuantity);
}

void UItemBase::UpdateReservedQuantity()
{
	int64 Sum = 0;
	for (const auto& Pair : ReservedAmounts)
	{
		Sum += FMath::Max(0, Pair.Value);
	}
	ReservedQuantity = static_cast<int32>(FMath::Min<int64>(Sum, MAX_int32));
	OnRep_AvailableAmount();
}

void UItemBase::OnRep_AvailableAmount()
{
	OnItemDataReplicated.Broadcast(this);
}

UItemBase* UItemBase::CreateTransferCopy(UObject* NewOuter, int32 Amount)
{
	if (!IsValid(NewOuter) || Amount <= 0 || Amount > Quantity)
	{
		return nullptr;
	}
	UItemBase* Copy = DuplicateObject<UItemBase>(this, NewOuter);
	if (Copy)
	{
		Copy->PersistentInstanceID = IsPortableInventory() && Amount == Quantity ? PersistentInstanceID : FGuid::NewGuid();
		Copy->Quantity = Amount;
		Copy->ReservedAmounts.Reset();
		Copy->ReservedQuantity = 0;
		Copy->OnUseItemDelegate.Clear();
		Copy->OnAmountChangedDelegate.Clear();
		Copy->OnItemDataReplicated.Clear();
	}
	return Copy;
}

bool UItemBase::ReserveAmount_Implementation(AActor* Requestor, int32 AmountToReserve)
{
	if (!Requestor || AmountToReserve <= 0)
	{
		return false;
	}

	int32 Free = IObjectDataProvider::Execute_GetFreeAmount(this);
	if (Free < AmountToReserve)
	{
		return false; // Not enough resources.
	}

	int32& CurrentReserved = ReservedAmounts.FindOrAdd(Requestor);
	CurrentReserved += AmountToReserve;
	UpdateReservedQuantity();
	return true;
}

void UItemBase::ReleaseReservation_Implementation(AActor* Requestor, int32 AmountToRelease)
{
	if (!Requestor || AmountToRelease <= 0)
	{
		return;
	}

	int32 Reserved = ReservedAmounts.FindRef(Requestor);
	if (Reserved <= 0)
	{
		return;
	}

	if (AmountToRelease >= Reserved)
	{
		ReservedAmounts.Remove(Requestor);
	}
	else
	{
		ReservedAmounts.FindChecked(Requestor) = Reserved - AmountToRelease;
	}
	UpdateReservedQuantity();
}

UObject* UItemBase::ConsumeReserved_Implementation(AActor* Requestor, int32 RequestedAmount)
{
	if (!Requestor || RequestedAmount <= 0)
	{
		return nullptr;
	}

	int32 Reserved = ReservedAmounts.FindRef(Requestor);
	if (Reserved <= 0)
	{
		return nullptr;
	}

	int32 ToConsume = FMath::Min(RequestedAmount, Reserved);
	ToConsume = FMath::Min(ToConsume, Quantity);
	if (ToConsume <= 0)
	{
		return nullptr;
	}

	auto NewRes = IObjectDataProvider::Execute_DuplicateItem(this);
	if (!NewRes)
	{
		return nullptr;
	}

	//NewRes->OnAmountChangedDelegate.Clear();

	IObjectDataProvider::Execute_SetQuantity(NewRes, ToConsume);

	auto OldAmount = Quantity;

	Quantity -= ToConsume;

	ReservedAmounts.Remove(Requestor);
	int32 RemainingReserve = Reserved - ToConsume;
	if (RemainingReserve > 0)
	{
		ReservedAmounts.Add(Requestor, RemainingReserve);
	}
	UpdateReservedQuantity();

	if (OnAmountChangedDelegate.IsBound())
	{
		OnAmountChangedDelegate.Broadcast(-ToConsume, this);
	}

	return NewRes;
}

void UItemBase::OnRep_ItemRow()
{
	if (!ItemRow.DataTable)
	{
		return;
	}

	if (FItemData* Row = ItemRow.GetRow<FItemData>(TEXT("OnRep_ItemRow")))
	{
		ItemRef = Row->ItemMetaData;
		OnItemDataReplicated.Broadcast(this);
	}
}

TMap<FName, FString> UItemBase::CaptureSaveState_Implementation() const
{
	return {};
}

void UItemBase::RestoreSaveState_Implementation(const TMap<FName, FString>& State)
{
}
