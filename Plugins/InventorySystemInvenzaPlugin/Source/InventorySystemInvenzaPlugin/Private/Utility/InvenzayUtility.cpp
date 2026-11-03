//  Nublin Studio 2026 All Rights Reserved.

#include "Utility/InvenzayUtility.h"
#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"

#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"
#include "Layout/Margin.h"

#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/InventoryTypes.h"
#include "Data/Items/itemBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Factory/ItemFactory.h"
#include "Interface/Inventory/InventoryInteractionHandler.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "Utility/InterfaceUtils.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetTree.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "UI/Inventory/SlotbasedInventorySlot.h"
#include "UI/Inventory/UInventoryBaseWidget.h"

UInventoryBase*
UInvenzayUtility::CreateStartupInventory(UObject* WorldContextObject, UInventoryCollectionComponent* ItemCollection,
										 const FInventoryStartupData& StartupData,
										 TMap<TObjectPtr<UInventoryBase>, FInitItemsList>& StartingItems)
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

	UInventoryBase* Inventory = UInventoryBase::CreateInventoryAdvanced(Owner, StartupData, Owner, ItemCollection);
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

bool UInvenzayUtility::CollectAuthoredInventoryLayout(const FInventorySettings& Settings,
	FSlotBasedInventoryWidgetInitData& Layout)
{
	const auto* WidgetClass = Cast<UWidgetBlueprintGeneratedClass>(Settings.InventoryWidgetClass.Get());
	const auto* Tree = WidgetClass ? WidgetClass->GetWidgetTreeArchetype() : nullptr;
	auto* Panel = Tree ? Cast<UUniformGridPanel>(Tree->FindWidget(TEXT("SlotsGridPanel"))) : nullptr;
	if (!Panel)
	{
		return false;
	}
	Layout = FSlotBasedInventoryWidgetInitData();
	Layout.InventorySize = FIntPoint::ZeroValue;
	Layout.InvCellSize = Settings.InventorySlotBasedSettings.InvCellSize;
	Layout.SlotSpacing = Panel->GetSlotPadding();
	for (auto* Child : Panel->GetAllChildren())
	{
		const auto* Cell = Cast<USlotbasedInventorySlot>(Child);
		const auto* Slot = Cell ? Cast<UUniformGridSlot>(Cell->Slot) : nullptr;
		if (!Slot)
		{
			continue;
		}
		FInventorySlotInfo Info;
		Info.CellPosition = FIntPoint(Slot->GetRow(), Slot->GetColumn());
		Info.AllowedCategory = Cell->AllowedSlotCategory;
		Info.LinkedEquipmentSlot = Cell->LinkedEquipmentSlotTag;
		Layout.SlotLayout.Add(Info);
		Layout.InventorySize.X = FMath::Max(Layout.InventorySize.X, Info.CellPosition.X + 1);
		Layout.InventorySize.Y = FMath::Max(Layout.InventorySize.Y, Info.CellPosition.Y + 1);
	}
	return !Layout.SlotLayout.IsEmpty();
}

int32 UInvenzayUtility::GrantStartingStack(UInventoryBase* Inventory, UObject* Item)
{
	if (!Inventory || !Item || Inventory->GetInventorySettings().bIsReferenceContainer)
	{
		return 0;
	}
	const int32 Requested = IObjectDataProvider::Execute_GetQuantity(Item);
	const float Capacity = Inventory->GetInventorySettings().InventoryMaxWeightCapacity;
	const float SingleWeight = IObjectDataProvider::Execute_GetItemSingleWeight(Item);
	int32 Allowed = Requested;
	if (Capacity >= 0.f && SingleWeight > 0.f)
	{
		Inventory->UpdateWeightInfo();
		Allowed = FMath::Min(Requested, FMath::Max(0,
			FMath::FloorToInt((Capacity - Inventory->GetInventoryTotalWeight() + KINDA_SMALL_NUMBER) / SingleWeight)));
	}
	if (Allowed <= 0)
	{
		return 0;
	}
	IObjectDataProvider::Execute_SetQuantity(Item, Allowed);
	FItemMoveData Move;
	Move.SourceItem = Item;
	Move.TargetInventory = Inventory;
	const int32 Added = FMath::Clamp(Inventory->HandleAddItem(Move).ActualAmountAdded, 0, Allowed);
	if (Added == 0)
	{
		IObjectDataProvider::Execute_SetQuantity(Item, Requested);
	}
	return Added;
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
		if (!Inventory || Inventory->GetInventoryContainerID().IsEmpty() || InitItems.Items.IsEmpty() ||
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

bool UInvenzayUtility::AddItemQuantity(UObject* Outer, UInventoryBase* TargetInventory,
									   const FInitItemsEntry InitItemsEntry)
{
	if (!TargetInventory || InitItemsEntry.Item.IsNull() || InitItemsEntry.Amount <= 0)
	{
		return false;
	}

	UObject* ItemForDuplicate = UItemFactory::CreateItemByHandle(Outer, InitItemsEntry.Item, InitItemsEntry.Amount);
	return AddItemQuantityInternal(TargetInventory, ItemForDuplicate, InitItemsEntry.Amount);
}

bool UInvenzayUtility::AddItemQuantityBySample(UObject* Outer, UInventoryBase* TargetInventory, UObject* ItemSample,
											   int32 TotalQuantity)
{
	if (!TargetInventory || !ItemSample || TotalQuantity <= 0)
	{
		return false;
	}

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemSample, TEXT("AddItemQuantityBySample")))
	{
		return false;
	}

	UObject* ItemForDuplicate =
		UItemFactory::CreateItemByHandle(Outer, IObjectDataProvider::Execute_GetItemRow(ItemSample), TotalQuantity);

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

	const FName FirstItemID = IObjectDataProvider::Execute_GetItemID(FirstItem);

	const FName SecondItemID = IObjectDataProvider::Execute_GetItemID(SecondItem);

	return FirstItemID == SecondItemID;
}

bool UInvenzayUtility::DoItemsHaveSameFootprint(UObject* FirstItem, UObject* SecondItem,
												EItemOrientationType OrientationFirstItem,
												EItemOrientationType OrientationSecondItem, bool bIgnoreSize)
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

	const FIntPoint FirstSize = IObjectDataProvider::Execute_GetItemSize(FirstItem, OrientationFirstItem);

	const FIntPoint SecondSize = IObjectDataProvider::Execute_GetItemSize(SecondItem, OrientationSecondItem);

	return FirstSize == SecondSize;
}

AActor* UInvenzayUtility::DropItem(UWorld* World, AActor* OwnerActor, const FDataTableRowHandle& ItemRow,
								int32 AmountToDrop, const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if (!IsValid(World) || World->GetNetMode() == NM_Client || !IsValid(OwnerActor) ||
		!OwnerActor->HasAuthority() || OwnerActor->GetWorld() != World || AmountToDrop <= 0 ||
		!ItemRow.DataTable || ItemRow.RowName.IsNone())
	{
		return nullptr;
	}

	auto Settings = GetInvenzaGlobalSettings(World);
	if (!Settings || !Settings->PickupClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PickupClass is not set."));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* Pickup = World->SpawnActor<AActor>(Settings->PickupClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (!IsValid(Pickup))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn Pickup."));
		return nullptr;
	}

	UPickupComponent* PickupComponent = Pickup->FindComponentByClass<UPickupComponent>();
	if (!IsValid(PickupComponent))
	{
		Pickup->Destroy();
		return nullptr;
	}

	FInitItemsEntry ItemDrop;
	ItemDrop.Item = ItemRow;
	ItemDrop.Amount = AmountToDrop;
	PickupComponent->InitializeDrop(ItemDrop);

	UObject* DroppedItem = PickupComponent->PeekItemData();
	if (!IsValid(Pickup) || !IsValid(DroppedItem) || !DroppedItem->Implements<UObjectDataProvider>() ||
		IObjectDataProvider::Execute_GetQuantity(DroppedItem) != AmountToDrop)
	{
		Pickup->Destroy();
		return nullptr;
	}

	if (auto* Mesh = Pickup->FindComponentByClass<UStaticMeshComponent>(); Mesh && Mesh->GetStaticMesh())
	{
		FHitResult Ground;
		FCollisionQueryParams Query(SCENE_QUERY_STAT(InvenzaDropGround), false, OwnerActor);
		Query.AddIgnoredActor(Pickup);
		const FVector Up(0.f, 0.f, Settings->DropGroundTraceHeight);
		const FVector Down(0.f, 0.f, Settings->DropGroundTraceDepth);
		if (!World->LineTraceSingleByChannel(Ground, SpawnLocation + Up, SpawnLocation - Down,
			ECC_WorldStatic, Query))
		{
			Pickup->Destroy();
			return nullptr;
		}
		// Initialization replaces the mesh and scale. Place its actual lower bound above the surface.
		Mesh->UpdateBounds();
		const float BottomOffset = Mesh->Bounds.Origin.Z - Mesh->Bounds.BoxExtent.Z - Pickup->GetActorLocation().Z;
		FVector SafeLocation = Ground.ImpactPoint;
		SafeLocation.Z += Settings->DropGroundClearance - BottomOffset;
		Pickup->SetActorLocation(SafeLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}
	return Pickup;
}

FVector2D UInvenzayUtility::CalculateItemVisualSize(UObject* Item, EItemOrientationType Orientation, FVector2D SlotSize,
													FMargin SlotSpacing, bool bIgnoreSize)
{
	if (!Item ||
		!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("CalculateItemVisualSize")))
	{
		return FVector2D::ZeroVector;
	}

	FIntPoint ItemSize = bIgnoreSize ? FIntPoint(1, 1) : IObjectDataProvider::Execute_GetItemSize(Item, Orientation);

	return FVector2D(SlotSize.X * ItemSize.X + SlotSpacing.Left * (ItemSize.X - 1),
					 SlotSize.Y * ItemSize.Y + SlotSpacing.Top * (ItemSize.Y - 1));
}

TMap<EObjectInteractionType, FModalActionConfig> UInvenzayUtility::CollectAccessibleObjectActions(UWorld* World,
																								  UObject* InItem)
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
	if (!WorldContext)
	{
		return nullptr;
	}

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
	{
		return Handler;
	}

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
	if (!ItemForDuplicate || !UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(
								 ItemForDuplicate, TEXT("AddItemQuantityInternal")))
	{
		return false;
	}

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
		{
			return false;
		}

		Remaining -= AddAmount;
	}

	return true;
}

void UInvenzayUtility::ApplyMaterialOverrides(UMeshComponent* Component,
	const TArray<TObjectPtr<UMaterialInterface>>& Materials)
{
	if (!Component)
	{
		return;
	}
	Component->EmptyOverrideMaterials();
	for (int32 Index = 0; Index < Materials.Num(); ++Index)
	{
		if (Materials[Index])
		{
			Component->SetMaterial(Index, Materials[Index]);
		}
	}
}

FInventoryPanelCanvasPlacement UInvenzayUtility::CaptureInventoryPanel(UInventoryContainerWidget* Widget)
{
	FInventoryPanelCanvasPlacement Placement;
	Placement.Widget = Widget;
	if (IsValid(Widget))
	{
		Placement.Visibility = Widget->GetVisibility();
		if (auto* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
		{
			Placement.Parent = Cast<UCanvasPanel>(Widget->GetParent());
			Placement.Layout = CanvasSlot->GetLayout();
			Placement.bAutoSize = CanvasSlot->GetAutoSize();
			Placement.ZOrder = CanvasSlot->GetZOrder();
		}
	}
	return Placement;
}

bool UInvenzayUtility::RestoreInventoryPanel(const FInventoryPanelCanvasPlacement& Placement)
{
	if (!IsValid(Placement.Widget) || !IsValid(Placement.Parent))
	{
		return false;
	}
	Placement.Widget->RemoveFromParent();
	auto* Restored = Placement.Parent->AddChildToCanvas(Placement.Widget);
	Restored->SetLayout(Placement.Layout);
	Restored->SetAutoSize(Placement.bAutoSize);
	Restored->SetZOrder(Placement.ZOrder);
	Placement.Widget->SetVisibility(Placement.Visibility);
	Placement.Widget->ReDrawRequest();
	return true;
}
