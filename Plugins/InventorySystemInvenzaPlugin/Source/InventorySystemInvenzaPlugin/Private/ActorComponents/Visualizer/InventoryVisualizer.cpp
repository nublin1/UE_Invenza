//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/Visualizer/InventoryVisualizer.h"

#include "ActorComponents/ItemCollection.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Items/ItemBase.h"
#include "Net/UnrealNetwork.h"
#include "Utility/InterfaceUtils.h"


UInventoryVisualizer::UInventoryVisualizer()
{
	SetIsReplicatedByDefault(true);
}

void UInventoryVisualizer::BeginPlay()
{
	Super::BeginPlay();

	FindParentMesh();
	InitializeCachedSlots();

	if (GetNetMode() != NM_DedicatedServer)
	{
		ObservedCollection = GetOwner()->FindComponentByClass<UItemCollection>();
		if (ObservedCollection.IsValid())
		{
			ObservedCollection->OnInventoryItemsChanged.AddUniqueDynamic(
				this, &UInventoryVisualizer::HandleInventoryItemsChanged);
		}
	}

	if (GetOwner()->HasAuthority())
	{
		InitializeInventoriesByTag(DefaultSearchTag, bTrackAllInvs);
	}
	else
	{
		OnRep_TargetInventories();
	}
}

void UInventoryVisualizer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindInventoryEvents();
	if (ObservedCollection.IsValid())
	{
		ObservedCollection->OnInventoryItemsChanged.RemoveDynamic(
			this, &UInventoryVisualizer::HandleInventoryItemsChanged);
	}
	ObservedCollection.Reset();
	Super::EndPlay(EndPlayReason);
}

void UInventoryVisualizer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryVisualizer, TargetInventories);
	DOREPLIFETIME(UInventoryVisualizer, DisplayMode);
}

void UInventoryVisualizer::UnbindInventoryEvents()
{
	for (const auto& Inventory : BoundInventories)
	{
		if (Inventory.IsValid())
		{
			Inventory->OnAddItemDelegate.RemoveDynamic(this, &UInventoryVisualizer::AddItemVisual);
			Inventory->OnItemRemovedDelegate.RemoveDynamic(this, &UInventoryVisualizer::RemoveItemVisual);
		}
	}
	BoundInventories.Empty();
}

void UInventoryVisualizer::OnRep_TargetInventories()
{
	UnbindInventoryEvents();
	if (GetNetMode() == NM_DedicatedServer) return;

	for (UInventoryBase* Inventory : TargetInventories)
	{
		if (!IsValid(Inventory)) continue;
		Inventory->OnAddItemDelegate.AddUniqueDynamic(this, &UInventoryVisualizer::AddItemVisual);
		Inventory->OnItemRemovedDelegate.AddUniqueDynamic(this, &UInventoryVisualizer::RemoveItemVisual);
		BoundInventories.Add(Inventory);
	}
	RefreshVisuals();
}

void UInventoryVisualizer::HandleInventoryItemsChanged(const FString& InventoryID)
{
	for (UInventoryBase* Inventory : TargetInventories)
	{
		if (IsValid(Inventory) && Inventory->GetInventoryContainerID() == InventoryID)
		{
			RefreshVisuals();
			return;
		}
	}
}

void UInventoryVisualizer::InitializeInventoriesByTag(FGameplayTag ContainerTag, bool bTrackAll)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;
	
	UItemCollection* ItemCollection = Owner->FindComponentByClass<UItemCollection>();
	if (!ItemCollection)
	{
		UE_LOG(LogTemp, Warning, TEXT("Visualizer: Collection not found on %s"), *Owner->GetName());
		return;
	}

	TargetInventories.Empty();

	TArray<UInventoryBase*> FoundInventories = ItemCollection->GetActorInventories();

	for (UInventoryBase* Inv : FoundInventories)
	{
		if (!Inv) continue;
		if (bTrackAll || Inv->GetInventorySettings().InventoryTag.MatchesTag(ContainerTag))
		{
			TargetInventories.Add(Inv);
		}
	}

	OnRep_TargetInventories();
}

void UInventoryVisualizer::InitializeCachedSlots()
{
	CachedSlots.Empty();
	if (!ParentMeshPtr) return;

	TArray<FName> AllSockets = ParentMeshPtr->GetAllSocketNames();
	for (FName SocketName : AllSockets)
	{
		FString NameStr = SocketName.ToString();
		bool bMatches = false;

		for (const FString& Keyword : SocketKeywords)
		{
			if (NameStr.Contains(Keyword))
			{
				bMatches = true;
				break;
			}
		}

		if (bMatches)
		{
			FCachedSocketData Data;
			Data.SocketName = SocketName;
			Data.RelativeTransform = ParentMeshPtr->GetSocketTransform(SocketName, RTS_Component);
			CachedSlots.Add(Data);
		}
	}
}

void UInventoryVisualizer::RefreshVisuals()
{
	if (GetNetMode() == NM_DedicatedServer) return;
	
	if (DisplayMode == EVisualizerMode::OccupancyMeshSwap)
	{
		UpdateOccupancyMesh();
	}
	else
	{
		UpdateIndividualItems();
	}
}

void UInventoryVisualizer::AddItemVisual(FItemMapping& ItemSlots, UObject* Item)
{
	if (GetNetMode() == NM_DedicatedServer || !Item)
		return;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("AddItemVisual")))
		return;

	if (TrackedVisuals.Contains(Item))
		return;

	if (DisplayMode == EVisualizerMode::OccupancyMeshSwap)
	{
		UpdateOccupancyMesh();
		return;
	}

	if (!IsValid(ParentMeshPtr))
		return;

	int32 FreeIndex = GetFirstFreeSocketIndex();
	if (FreeIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("Visualizer: No free sockets for item %s"), *Item->GetName());
		return;
	}
	
	UStaticMesh* SM_Item = IObjectDataProvider::Execute_GetItemRef(Item).ItemAssetData.Mesh;
	if (!SM_Item)
		return;

	UStaticMeshComponent* NewItemMesh = NewObject<UStaticMeshComponent>(GetOwner());
	NewItemMesh->SetStaticMesh(SM_Item);

	NewItemMesh->SetupAttachment(ParentMeshPtr, CachedSlots[FreeIndex].SocketName);
	NewItemMesh->SetRelativeTransform(FTransform::Identity);
	NewItemMesh->RegisterComponent();

	TrackedVisuals.Add(Item, NewItemMesh);
	OccupiedSocketIndices.Add(FreeIndex, Item);
}

void UInventoryVisualizer::RemoveItemVisual(FItemMapping ItemSlots, UObject* Item)
{
	if (!Item) return;

	if (DisplayMode == EVisualizerMode::OccupancyMeshSwap)
	{
		UpdateOccupancyMesh();
		return;
	}
	
	if (TObjectPtr<UStaticMeshComponent>* FoundMesh = TrackedVisuals.Find(Item))
	{
		if (*FoundMesh)
		{
			(*FoundMesh)->DestroyComponent();
		}
		TrackedVisuals.Remove(Item);
	}

	if (const int32* FoundIndex = OccupiedSocketIndices.FindKey(Item))
	{
		OccupiedSocketIndices.Remove(*FoundIndex);
	}
}

float UInventoryVisualizer::GetTotalOccupancy() const
{
	if (TargetInventories.Num() == 0) return 0.0f;

	float CombinedOccupancy = 0.0f;
	int32 ReadyInventoryCount = 0;
	for (auto Inv : TargetInventories)
	{
		if (IsValid(Inv) && IsValid(Inv->GetItemCollectionLinked()))
		{
			CombinedOccupancy += Inv->GetInventoryOccupancyPercent();
			++ReadyInventoryCount;
		}
	}
	return ReadyInventoryCount > 0 ? CombinedOccupancy / ReadyInventoryCount : 0.0f;
}

int32 UInventoryVisualizer::GetFirstFreeSocketIndex() const
{
	for (int32 i = 0; i < CachedSlots.Num(); ++i)
	{
		if (!OccupiedSocketIndices.Contains(i))
		{
			return i;
		}
	}
	return INDEX_NONE;
}

void UInventoryVisualizer::UpdateOccupancyMesh()
{
	UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(ParentMeshPtr);
	if (!SMC) return;

	// Inventory APIs return 0..100; mesh thresholds use 0..1.
	const float AvgPerc = FMath::Clamp(GetTotalOccupancy() / 100.0f, 0.0f, 1.0f);

	UStaticMesh* BestMesh = nullptr;
	float BestThreshold = -1.0f;

	for (auto& Elem : OccupancyMeshes)
	{
		if (AvgPerc >= Elem.Key && Elem.Key > BestThreshold)
		{
			BestThreshold = Elem.Key;
			BestMesh = Elem.Value;
		}
	}

	if (BestMesh && SMC->GetStaticMesh() != BestMesh)
	{
		SMC->SetStaticMesh(BestMesh);
		InitializeCachedSlots(); 
	}
}

void UInventoryVisualizer::UpdateIndividualItems()
{
	if (CachedSlots.Num() == 0 || !ParentMeshPtr) return;
	
	TArray<UObject*> CurrentItems;
	for (auto Inv : TargetInventories)
	{
		if (Inv && Inv->GetItemCollectionLinked())
		{
			CurrentItems.Append(Inv->GetItemCollectionLinked()->GetAllItemsByContainer(Inv->GetInventoryContainerID()));
		}
	}
	
	TArray<UObject*> VisualsToRemove;
	for (auto& Pair : TrackedVisuals)
	{
		if (!CurrentItems.Contains(Pair.Key))
		{
			VisualsToRemove.Add(Pair.Key);
		}
	}

	for (UObject* ItemToRemove : VisualsToRemove)
	{
		FItemMapping ItemSlots;
		RemoveItemVisual(ItemSlots, ItemToRemove);
	}
	
	for (UObject* Item : CurrentItems)
	{
		FItemMapping ItemSlots;
		AddItemVisual(ItemSlots, Item);
	}
}

void UInventoryVisualizer::FindParentMesh()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;
	
	ParentMeshPtr = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!ParentMeshPtr)
	{
		ParentMeshPtr = Owner->FindComponentByClass<UStaticMeshComponent>();
	}
}

