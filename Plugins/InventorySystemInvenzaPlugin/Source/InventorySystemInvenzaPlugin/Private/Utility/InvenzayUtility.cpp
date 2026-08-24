//  Nublin Studio 2026 All Rights Reserved.

#include "Utility/InvenzayUtility.h"

#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/InventoryTypes.h"
#include "Data/Items/itemBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Factory/ItemFactory.h"
#include "Interface/Inventory/InventoryInteractionHandler.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "Utility/InterfaceUtils.h"


UInventoryBase* UInvenzayUtility::CreateStartupInventory(UObject* WorldContextObject, UItemCollection* ItemCollection,
	const FInventoryStartupData& StartupData, TMap<TObjectPtr<UInventoryBase>, FInitItemsList>& StartingItems)
{
	if (!WorldContextObject || !ItemCollection)
	{
		return nullptr;
	}

	AActor* Owner = WorldContextObject->GetTypedOuter<AActor>();
	if (!Owner || !Owner->HasAuthority())
	{
		return nullptr;
	}

	UInventoryBase* Inventory = UInventoryBase::CreateInventoryAdvanced(Owner,StartupData, Owner, ItemCollection);
	if (!Inventory)
	{
		return nullptr;
	}

	FInitItemsList InitItemsList;
	InitItemsList.Items = StartupData.StartItems;

	StartingItems.Add(Inventory, InitItemsList);

	ItemCollection->AddPawnInventory_Internal(Inventory);

	return Inventory;
}

void UInvenzayUtility::SetupStartingResources(UObject* WorldContextObject,
                                              TMap<TObjectPtr<UInventoryBase>, FInitItemsList>& StartingItems)
{
	if (StartingItems.IsEmpty())
	{
		return;
	}

	for (auto& [Inventory, InitItems] : StartingItems)
	{
		if (!Inventory ||
			Inventory->GetInventoryContainerID().IsEmpty() ||
			InitItems.Items.IsEmpty() ||
			!Inventory->GetItemCollectionLinked())
		{
			continue;
		}

		for (const FInitItemsEntry& InitItemEntry : InitItems.Items)
		{
			if (InitItemEntry.Item.RowName.IsNone())
			{
				continue;
			}

			UObject* ItemSample = UItemFactory::CreateItemByHandle(WorldContextObject, InitItemEntry.Item, 1);
			if (!ItemSample)
			{
				continue;
			}

			AddItemQuantityBySample(WorldContextObject, Inventory, ItemSample, InitItemEntry.Amount);
		}
	}

	StartingItems.Empty();
}

bool UInvenzayUtility::AddItemQuantity(UObject* Outer, UInventoryBase* TargetInventory, const FInitItemsEntry InitItemsEntry)
{
	if (!TargetInventory || InitItemsEntry.Item.IsNull() || InitItemsEntry.Amount <= 0)
		return false;

	UObject* ItemForDuplicate = UItemFactory::CreateItemByHandle(Outer, InitItemsEntry.Item, InitItemsEntry.Amount);
	return AddItemQuantityInternal(TargetInventory, ItemForDuplicate, InitItemsEntry.Amount);
}

bool UInvenzayUtility::AddItemQuantityBySample(UObject* Outer, UInventoryBase* TargetInventory, UObject* ItemSample,
                                                int32 TotalQuantity)
{
	if (!TargetInventory || !ItemSample || TotalQuantity <= 0)
		return false;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemSample, TEXT("AddItemQuantityBySample")))
		return false;

	UObject* ItemForDuplicate =
		UItemFactory::CreateItemByHandle(Outer,
			IObjectDataProvider::Execute_GetItemRow(ItemSample),
			TotalQuantity);

	return AddItemQuantityInternal(TargetInventory, ItemForDuplicate, TotalQuantity);
}

bool UInvenzayUtility::bIsSameItems(UObject* FirstItem, UObject* SecondItem)
{
	if (!FirstItem || !SecondItem)
	{
		return false;
	}

	if (!FirstItem->Implements<UObjectDataProvider>())
	{
		return false;
	}

	if (!SecondItem->Implements<UObjectDataProvider>())
	{
		return false;
	}

	const FName FirstItemID =
		IObjectDataProvider::Execute_GetItemID(FirstItem);

	const FName SecondItemID =
		IObjectDataProvider::Execute_GetItemID(SecondItem);

	return FirstItemID == SecondItemID;
}

bool UInvenzayUtility::DoItemsHaveSameFootprint(UObject* FirstItem, UObject* SecondItem,
	EItemOrientationType OrientationFirstItem, EItemOrientationType OrientationSecondItem, bool bIgnoreSize)
{
	if (!FirstItem || !SecondItem)
	{
		return false;
	}

	if (!FirstItem->Implements<UObjectDataProvider>())
	{
		return false;
	}

	if (!SecondItem->Implements<UObjectDataProvider>())
	{
		return false;
	}

	if (bIgnoreSize)
	{
		return true;
	}

	const FIntPoint FirstSize =
		IObjectDataProvider::Execute_GetItemSize(
			FirstItem,
			OrientationFirstItem
		);

	const FIntPoint SecondSize =
		IObjectDataProvider::Execute_GetItemSize(
			SecondItem,
			OrientationSecondItem
		);

	return FirstSize == SecondSize;
}

void UInvenzayUtility::DropItem(UWorld* World, AActor* OwnerActor, const FDataTableRowHandle& ItemRow, int32 AmountToDrop,
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

FVector2D UInvenzayUtility::CalculateItemVisualSize(UObject* Item, EItemOrientationType Orientation,
	FVector2D SlotSize, FMargin SlotSpacing, bool bIgnoreSize)
{
	if (!Item ||
		!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("CalculateItemVisualSize")))
		return FVector2D::ZeroVector;

	FIntPoint ItemSize = bIgnoreSize
		? FIntPoint(1, 1)
		: IObjectDataProvider::Execute_GetItemSize(Item, Orientation);

	return FVector2D(
		SlotSize.X * ItemSize.X + SlotSpacing.Left * (ItemSize.X - 1),
		SlotSize.Y * ItemSize.Y + SlotSpacing.Top  * (ItemSize.Y - 1));
}

TMap<EObjectInteractionType, FModalActionConfig> UInvenzayUtility::CollectAccessibleObjectActions(UWorld* World, UObject* InItem)
{
	TMap<EObjectInteractionType, FModalActionConfig> AllowedActions;

	if (!InItem)
	{
		return AllowedActions;
	}

	const auto* Settings = GetInvenzaGlobalSettings(World);

	if (!InItem->Implements<UObjectDataProvider>())
	{
		return AllowedActions;
	}

	for (const auto& Pair : Settings->InvItemsModalActions)
	{
		if (IObjectDataProvider::Execute_CanPerformAction(InItem, Pair.Key, Settings))
		{
			AllowedActions.Add(Pair);
		}
	}

	return AllowedActions;
}

UInvenzaInventorySettingsAsset* UInvenzayUtility::GetInvenzaGlobalSettings(const UObject* WorldContext)
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

TScriptInterface<IInventoryInteractionHandler> UInvenzayUtility::FindInventoryHandler(AActor* Actor)
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

bool UInvenzayUtility::AddItemQuantityInternal(UInventoryBase* TargetInventory, UObject* ItemForDuplicate,
	int32 Remaining)
{
	if (!ItemForDuplicate ||
		!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemForDuplicate, TEXT("AddItemQuantityInternal")))
		return false;

	while (Remaining > 0)
	{
		UObject* Item = IObjectDataProvider::Execute_DuplicateItem(ItemForDuplicate);

		const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(Item);
		const int32 MaxStack = Meta.ItemNumeraticData.MaxStackSizeInCharacter;

		int32 AddAmount = FMath::Min(Remaining, MaxStack);
		IObjectDataProvider::Execute_SetQuantity(Item, AddAmount);

		FItemMoveData MoveData;
		MoveData.SourceItem = Item;
		MoveData.TargetInventory = TargetInventory;

		if (TargetInventory->HandleAddItem(MoveData).OperationResult == EItemAddResult::IAR_NoItemAdded)
			return false;

		Remaining -= AddAmount;
	}

	return true;
}
