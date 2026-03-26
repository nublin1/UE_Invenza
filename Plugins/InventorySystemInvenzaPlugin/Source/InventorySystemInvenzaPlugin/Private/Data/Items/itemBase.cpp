//  Nublin Studio 2025 All Rights Reserved.

#include "Data/Items/itemBase.h"

#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "Data/ItemData.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UItemBase::UItemBase(): ItemRef(), Quantity(0)
{
}

bool UItemBase::bIsSameItems(UItemBase* FirstItem, UItemBase* SecondItem)
{
	if (!FirstItem || !SecondItem)
		return false;
	
	if (FirstItem->GetItemRef().ItemTextData.NameID.EqualTo(SecondItem->GetItemRef().ItemTextData.NameID))
		return true;

	return false;
}

bool UItemBase::DoItemsHaveSameFootprint(UItemBase* FirstItem, UItemBase* SecondItem)
{
	if (!FirstItem || !SecondItem)
		return false;
	
	if (FirstItem->GetItemRef().ItemNumeraticData.InventoryHorizontalSlots != SecondItem->GetItemRef().ItemNumeraticData.InventoryHorizontalSlots)
		return false;
	if (FirstItem->GetItemRef().ItemNumeraticData.InventoryVerticalSlots != SecondItem->GetItemRef().ItemNumeraticData.InventoryVerticalSlots)
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

	auto Pickup = World->SpawnActor<AActor>(InventoryManager->GetUISettings().PickupClass, SpawnTransform, SpawnParameters);
	if (!Pickup)
		return;
	
	if (auto PickupComponent = Pickup->FindComponentByClass<UPickupComponent>())
	{
		PickupComponent->InitializeDrop(this);
	}
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
	// Списываем
	Quantity -= ToConsume;
	    
	ReservedAmounts.Remove(Requestor);
	int32 RemainingReserve = Reserved - ToConsume;

	if (OnAmountChangedDelegate.IsBound())
		OnAmountChangedDelegate.Broadcast(-ToConsume , this);
    
	return NewRes;
}