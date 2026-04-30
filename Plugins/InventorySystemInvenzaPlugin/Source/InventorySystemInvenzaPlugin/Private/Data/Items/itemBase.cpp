//  Nublin Studio 2025 All Rights Reserved.

#include "Data/Items/itemBase.h"

#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "Data/ItemData.h"
#include "Data/Settings/InvenzaInventoryUISettingsAsset.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Utility/InventoryUtility.h"

UItemBase::UItemBase(): ItemRef(), Quantity(0)
{
	
}

void UItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItemBase, ItemID);
	DOREPLIFETIME(UItemBase, ItemRow);
	DOREPLIFETIME(UItemBase, Quantity);
}

bool UItemBase::bIsSameItems(UItemBase* FirstItem, UItemBase* SecondItem)
{
	if (!FirstItem || !SecondItem)
		return false;
	
	if (FirstItem->GetItemID() == SecondItem->GetItemID())
		return true;

	return false;
}

bool UItemBase::DoItemsHaveSameFootprint(UItemBase* FirstItem, UItemBase* SecondItem,
	EItemOrientationType OrientationFirstItem, EItemOrientationType OrientationSecondItem, bool bIgnoreSize)
{
	if (!FirstItem || !SecondItem)
		return false;

	if (bIgnoreSize)
		return true;

	auto FirstSize = FirstItem->GetItemSize(OrientationFirstItem);
	auto SecondSize = SecondItem->GetItemSize(OrientationSecondItem);
	
	if (FirstSize != SecondSize)
		return false;

	return true;
}

void UItemBase::InitItem(const FName ID, FItemData Data, int32 InQuantity)
{
	this->ItemID = ID;
	this->SetItemRef(Data.ItemMetaData);
	if (InQuantity <= 0)
		InQuantity = 1;
	else if (InQuantity > ItemRef.ItemNumeraticData.MaxStackSizeInCharacter)
		InQuantity = ItemRef.ItemNumeraticData.MaxStackSizeInCharacter;
	Quantity = InQuantity;
}

void UItemBase::UseItem()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("UseItem was called!"));
	
	if (OnUseItemDelegate.IsBound())
		OnUseItemDelegate.Broadcast(this);
}

UItemBase* UItemBase::DuplicateItem()
{
	UItemBase* NewItem = NewObject<UItemBase>();
	if (NewItem && this)
	{
		NewItem->ItemID = this->ItemID;
		NewItem->ItemRef = this->ItemRef;
		NewItem->Quantity = this->Quantity;
		NewItem->ItemRow = this->ItemRow;
	}
	return NewItem;
}

void UItemBase::DropItem(UWorld* World)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if (!PlayerController || !this) return;
	auto Pawn = PlayerController->GetPawn();
	APawn* PawnRaw = Pawn.Get();
	UIInventoryManager* InventoryManager = Pawn->FindComponentByClass<UIInventoryManager>();

	if (!InventoryManager)
		return;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Pawn;
	SpawnParameters.bNoFail = true;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	const FVector SpawnLocation{PawnRaw->GetActorLocation() + (PawnRaw->GetActorForwardVector() * 50.0f)};
	const FTransform SpawnTransform(PawnRaw->GetActorRotation(), SpawnLocation);

	
	auto Pickup = World->SpawnActor<AActor>(UInventoryUtility::GetInvenzaGlobalSettings(GetWorld())->PickupClass, SpawnTransform, SpawnParameters);
	if (!Pickup)
		return;
	
	if (auto PickupComponent = Pickup->FindComponentByClass<UPickupComponent>())
	{
		PickupComponent->InitializeDrop(this);
	}
}

EItemOrientationType UItemBase::GetInitialItemOrientation()
{
	if (ItemRef.ItemNumeraticData.InventoryVerticalSlots > ItemRef.ItemNumeraticData.InventoryHorizontalSlots)
		return EItemOrientationType::Vertical;
	return EItemOrientationType::Horizontal;
}

FIntPoint UItemBase::GetItemSize(EItemOrientationType Orientation) 
{
	const int32 H = ItemRef.ItemNumeraticData.InventoryHorizontalSlots;
	const int32 V = ItemRef.ItemNumeraticData.InventoryVerticalSlots;

	EItemOrientationType Initial = GetInitialItemOrientation();

	if (GetInitialItemOrientation() == EItemOrientationType::Horizontal)
	{
		if (Orientation == Initial)
			return FIntPoint(H, V);
		else
			return FIntPoint(V, H);
	}

	if (Orientation == Initial)
	{
		return FIntPoint(H, V);
	}

	return FIntPoint(V, H);
}

FString UItemBase::CategoryToString()
{
	return StaticEnum<EItemCategory>()->GetNameStringByValue(static_cast<int32>(ItemRef.ItemCategory));
}

int32 UItemBase::GetFreeAmount() const
{
	int32 ReservedSum = 0;
	for (const auto& Pair : ReservedAmounts)
	{
		//ReservedSum += Pair.Value;
	}
	return FMath::Max(0, Quantity - ReservedSum);
}

bool UItemBase::ReserveAmount(AActor* Requestor, int32 AmountToReserve)
{
	if (!Requestor || AmountToReserve <= 0) return false;
    
	int32 Free = GetFreeAmount();
	if (Free < AmountToReserve)
	{
		return false; // Недостаточно ресурсов
	}
    
	int32& CurrentReserved = ReservedAmounts.FindOrAdd(Requestor);
	CurrentReserved += AmountToReserve;
	return true;
}

void UItemBase::ReleaseReservation(AActor* Requestor, int32 AmountToRelease)
{
	if (!Requestor || AmountToRelease <= 0) return;

	int32 Reserved = ReservedAmounts.FindRef(Requestor);
	if (Reserved <= 0)
		return;

	auto v = ReservedAmounts.Find(Requestor);
	v -= AmountToRelease;

	if (Reserved>=AmountToRelease)
		ReservedAmounts.Remove(Requestor);
}

UItemBase* UItemBase::ConsumeReserved(AActor* Requestor, int32 RequestedAmount)
{
	if (!Requestor || RequestedAmount <= 0) return nullptr;
    
	int32 Reserved = ReservedAmounts.FindRef(Requestor);
	if (Reserved<=0) return nullptr;
    
	int32 ToConsume = FMath::Min(RequestedAmount, Reserved);
	ToConsume = FMath::Min(ToConsume, Quantity);
	if (ToConsume <= 0) return nullptr;

	auto NewRes = DuplicateItem();
	if (!NewRes) return nullptr;

	NewRes->OnAmountChangedDelegate.Clear();
	
	NewRes->SetQuantity(ToConsume);

	auto OldAmount = Quantity;    

	Quantity -= ToConsume;
	    
	ReservedAmounts.Remove(Requestor);
	int32 RemainingReserve = Reserved - ToConsume;

	if (OnAmountChangedDelegate.IsBound())
		OnAmountChangedDelegate.Broadcast(-ToConsume , this);
    
	return NewRes;
}

void UItemBase::OnRep_ItemRow()
{
	if (!ItemRow.DataTable)
		return;
	
	if (FItemData* Row = ItemRow.GetRow<FItemData>(TEXT("OnRep_ItemRow")))
	{
		ItemRef = Row->ItemMetaData;
		OnItemDataReplicated.Broadcast(this);
	}
}
