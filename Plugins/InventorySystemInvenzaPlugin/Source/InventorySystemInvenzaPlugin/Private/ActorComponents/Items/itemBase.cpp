//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Items/itemBase.h"

#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "Data/ItemData.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "Inventory"

UItemBase::UItemBase(): ItemRef(), Quantity(0)
{
}

bool UItemBase::bIsSameItems(UItemBase* FirstItem, UItemBase* SecondItem)
{
	if (!FirstItem || !SecondItem)
		return false;
	
	if (FirstItem->GetItemRef().ItemTextData.Name.EqualTo(SecondItem->GetItemRef().ItemTextData.Name))
		return true;

	return false;
}

void UItemBase::InitItem(const FName ID, FItemData Data, int32 InQuantity)
{
	this->ItemID = ID;
	this->SetItemRef(Data.ItemMetaData);
	if (InQuantity <= 0)
		InQuantity = 1;
	else if (InQuantity > ItemRef.ItemNumeraticData.MaxStackSize)
		InQuantity = ItemRef.ItemNumeraticData.MaxStackSize;
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
	UIInventoryManager* InventoryManager = Pawn->FindComponentByClass<UIInventoryManager>();

	if (!InventoryManager)
		return;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Pawn;
	SpawnParameters.bNoFail = true;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	const FVector SpawnLocation{Pawn->GetActorLocation() + (Pawn->GetActorForwardVector() * 50.0f)};
	const FTransform SpawnTransform(Pawn->GetActorRotation(), SpawnLocation);

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
