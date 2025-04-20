//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Items/itemBase.h"

#include "ActorComponents/Interactable/PickupComponent.h"
#include "Data/ItemData.h"
#include "Kismet/GameplayStatics.h"
#include "World/AUIManagerActor.h"

#define LOCTEXT_NAMESPACE "Inventory"

UItemBase::UItemBase(): ItemRef(), Quantity(0)
{
}

UItemBase* UItemBase::CreateFromDataTable(UDataTable* ItemDataTable, const FName& DesiredItemID, int32 InitQuantity)
{
	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemDataTable is null!"));
		return nullptr;
	}

	FItemData* ItemData = ItemDataTable->FindRow<FItemData>(DesiredItemID, TEXT("CreateFromDataTable"));
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item data not found for ID: %s"), *DesiredItemID.ToString());
		return nullptr;
	}
	
	UItemBase* NewItem = NewObject<UItemBase>();
	if (!NewItem)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create UItemBase object!"));
		return nullptr;
	}
	
	NewItem->SetItemRef(ItemData->ItemMetaData);
	int32 ClampedQuantity = FMath::Clamp(InitQuantity, 1, ItemData->ItemMetaData.ItemNumeraticData.MaxStackSize);
	NewItem->SetQuantity(ClampedQuantity);

	return NewItem;
}

bool UItemBase::bIsSameItems(UItemBase* FirstItem, UItemBase* SecondItem)
{
	if (!FirstItem || !SecondItem)
		return false;
	
	if (FirstItem->GetItemRef().ItemTextData.Name.EqualTo(SecondItem->GetItemRef().ItemTextData.Name))
		return true;

	return false;
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
	auto ManagerActor = Cast<AUIManagerActor>(UGameplayStatics::GetActorOfClass(World, AUIManagerActor::StaticClass()));

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Pawn;
	SpawnParameters.bNoFail = true;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	const FVector SpawnLocation{Pawn->GetActorLocation() + (Pawn->GetActorForwardVector() * 50.0f)};
	const FTransform SpawnTransform(Pawn->GetActorRotation(), SpawnLocation);

	auto Pickup = World->SpawnActor<AActor>(ManagerActor->GetUISettings().PickupClass, SpawnTransform, SpawnParameters);
	if (auto PickupComponent = Pickup->FindComponentByClass<UPickupComponent>())
	{
		PickupComponent->InitializeDrop(this);
	}
}

FText UItemBase::GetItemCategoryString()
{
	FText Result = FText::GetEmpty();

	if (ItemRef.ItemCategory2 & (1 <<static_cast<uint8>(EItemCategory::Consumable)))
	{
		Result = FText::Join(FText::FromString(" "), Result, LOCTEXT("Category_Consumable", "Consumable"));
	}
	if (ItemRef.ItemCategory2 & (1 <<static_cast<uint8>(EItemCategory::Money)))
	{
		Result = FText::Join(FText::FromString(" "), Result, LOCTEXT("Category_Money", "Money"));
	}

	if (ItemRef.ItemCategory2 & (1 <<static_cast<uint8>(EItemCategory::None)) || Result.IsEmpty())
	{
		Result = FText::Join(FText::FromString(" "), Result, LOCTEXT("Category_None", "None"));
	}

	return Result;
}
