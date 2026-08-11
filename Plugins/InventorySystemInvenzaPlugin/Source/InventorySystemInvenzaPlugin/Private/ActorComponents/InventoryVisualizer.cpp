//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/InventoryVisualizer.h"

#include "ActorComponents/ItemCollection.h"
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

	InitializeInventoriesByTag(DefaultSearchTag, bTrackAllInvs);
}

void UInventoryVisualizer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryVisualizer, TargetInventories);
	DOREPLIFETIME(UInventoryVisualizer, DisplayMode);
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
	if (FoundInventories.IsEmpty())
		return;

	for (UInventoryBase* Inv : FoundInventories)
	{
		if (!Inv) continue;
		if (bTrackAll || Inv->GetInventorySettings().InventoryTag.MatchesTag(ContainerTag))
		{
			TargetInventories.Add(Inv);
			if (GetNetMode() != NM_DedicatedServer)
			{
				Inv->OnAddItemDelegate.AddUniqueDynamic(this, &UInventoryVisualizer::AddItemVisual);
				Inv->OnItemRemovedDelegate.AddUniqueDynamic(this, &UInventoryVisualizer::RemoveItemVisual);
			}
		}
	}

	// 3. После того как нашли всё — обновляем картинку
	RefreshVisuals();
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
	
	UStaticMesh* SM_Item =
		IObjectDataProvider::Execute_GetItemRef(Item).ItemAssetData.AlternativeMesh;

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
	for (auto Inv : TargetInventories)
	{
		if (Inv) CombinedOccupancy += Inv->GetInventoryOccupancyPercent();
	}
	return CombinedOccupancy / TargetInventories.Num();
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

	float TotalPerc = 0.0f;
	for (auto Inv : TargetInventories) { if (Inv) TotalPerc += Inv->GetInventoryOccupancyPercent(); }
	float AvgPerc = TotalPerc / FMath::Max(1, TargetInventories.Num());

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

