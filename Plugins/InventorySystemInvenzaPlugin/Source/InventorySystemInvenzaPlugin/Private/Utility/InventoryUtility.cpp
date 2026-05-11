//  Nublin Studio 2026 All Rights Reserved.

#include "Utility/InventoryUtility.h"

#include "ActorComponents/Interactable/PickupComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/InventoryTypes.h"
#include "Data/Items/itemBase.h"
#include "Data/Settings/InvenzaInventoryUISettingsAsset.h"
#include "Factory/ItemFactory.h"
#include "Interface/Inventory/InventoryInteractionHandler.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"

void UInventoryUtility::DropItem(UWorld* World, AActor* OwnerActor, const FDataTableRowHandle& ItemRow, int32 AmountToDrop,
	const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if (!World) return;

	auto Settings = GetInvenzaGlobalSettings(World);
	if (!Settings || !Settings->PickupClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PickupClass is not set."));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerActor;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* Pickup = World->SpawnActor<AActor>(
		Settings->PickupClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams);

	if (!Pickup)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn Pickup."));
		return;
	}

	if (UPickupComponent* PickupComponent = Pickup->FindComponentByClass<UPickupComponent>())
	{
		FInitItemsEntry ItemDrop;
		ItemDrop.Item = ItemRow;
		ItemDrop.Amount = AmountToDrop;	
		
		PickupComponent->InitializeDrop(ItemDrop);
	}
}

bool UInventoryUtility::AddItemQuantity(UObject* Outer, UInventoryBase* TargetInventory, UItemBase* ItemSample,
                                        int32 TotalQuantity)
{
	if (!TargetInventory || !ItemSample ||TotalQuantity <= 0)
		return false;

	int32 Remaining = TotalQuantity;

	auto ItemClass = ItemSample->GetItemRow();
	UItemBase* ItemForDuplicate = UItemFactory::CreateItemByHandle(Outer, ItemClass, Remaining);
	if (!ItemForDuplicate)
		return false;
	
	while (Remaining > 0)
	{
		auto Item = ItemForDuplicate->DuplicateItem();

		int32 MaxStack = Item->GetItemRef().ItemNumeraticData.MaxStackSizeInCharacter;
		int32 AddAmount = FMath::Min(Remaining, MaxStack);

		Item->SetQuantity(AddAmount);

		FItemMoveData MoveData;
		MoveData.SourceItem = Item;
		MoveData.TargetInventory = TargetInventory;

		FItemAddResult Result = TargetInventory->HandleAddItem(MoveData);

		if (Result.OperationResult == EItemAddResult::IAR_NoItemAdded)
			return false;

		Remaining -= AddAmount;
	}

	return true;
}

FVector2D UInventoryUtility::CalculateItemVisualSize(UItemBase* Item, EItemOrientationType Orientation,
	FVector2D SlotSize, FMargin SlotSpacing, bool bIgnoreSize)
{
	if (!Item)
		return FVector2D::ZeroVector;

	FIntPoint ItemSize = bIgnoreSize
		? FIntPoint(1, 1)
		: Item->GetItemSize(Orientation);

	return FVector2D(
		SlotSize.X * ItemSize.X + SlotSpacing.Left * (ItemSize.X - 1),
		SlotSize.Y * ItemSize.Y + SlotSpacing.Top  * (ItemSize.Y - 1));
}

const UInvenzaInventoryUISettingsAsset* UInventoryUtility::GetInvenzaGlobalSettings(const UObject* WorldContext)
{
	if (!WorldContext) return nullptr;

	if (auto GI = WorldContext->GetWorld()->GetGameInstance())
	{
		if (auto Subsystem = GI->GetSubsystem<UInvenzaInventorySettingsSubsystem>())
		{
			return Subsystem->GetSettings();
		}
	}

	return nullptr;
}

TScriptInterface<IInventoryInteractionHandler> UInventoryUtility::FindInventoryHandler(AActor* Actor)
{
	TScriptInterface<IInventoryInteractionHandler> Handler;

	if (!Actor)
		return Handler;

	for (UActorComponent* Comp : Actor->GetComponents())
	{
		if (Comp && Comp->GetClass()->ImplementsInterface(UInventoryInteractionHandler::StaticClass()))
		{
			Handler.SetObject(Comp);
			Handler.SetInterface(Cast<IInventoryInteractionHandler>(Comp));
			return Handler;
		}
	}

	return Handler;
}