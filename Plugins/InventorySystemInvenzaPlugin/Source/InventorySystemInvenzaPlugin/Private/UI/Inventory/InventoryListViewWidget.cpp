// Nublin Studio 2026 All Rights Reserved.

#include "UI/Inventory/InventoryListViewWidget.h"

#include "ActorComponents/InventoryCollectionComponent.h"
#include "Algo/StableSort.h"
#include "Components/EditableText.h"
#include "Components/ListView.h"
#include "Data/Inventory/InventoryBase.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Interface/Interaction/ObjectDataProvider.h"
#include "TimerManager.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Inventory/InventoryViewEntry.h"
#include "UObject/UObjectGlobals.h"

void UInventoryListViewWidget::NativeConstruct()
{
	Super::NativeConstruct();
	bViewActive = true;
	if (SearchText)
	{
		SearchText->SetText(SearchQuery);
		SearchText->OnTextChanged.AddUniqueDynamic(this, &UInventoryListViewWidget::HandleSearchChanged);
	}
	if (SortByNameButton)
	{
		SortByNameButton->OnButtonClicked.AddUniqueDynamic(this, &UInventoryListViewWidget::HandleSortByName);
	}
	if (SortByQuantityButton)
	{
		SortByQuantityButton->OnButtonClicked.AddUniqueDynamic(this, &UInventoryListViewWidget::HandleSortByQuantity);
	}
	if (!GarbageCollectedHandle.IsValid())
	{
		GarbageCollectedHandle = FCoreUObjectDelegates::GetPostGarbageCollect().AddUObject(
			this, &UInventoryListViewWidget::HandleGarbageCollected);
	}
	BindSource();
	RefreshView();
}

void UInventoryListViewWidget::NativeDestruct()
{
	bViewActive = false;
	CancelRefresh();
	UnbindSource();
	ClearEntries();
	if (SearchText)
	{
		SearchText->OnTextChanged.RemoveAll(this);
	}
	if (SortByNameButton)
	{
		SortByNameButton->OnButtonClicked.RemoveAll(this);
	}
	if (SortByQuantityButton)
	{
		SortByQuantityButton->OnButtonClicked.RemoveAll(this);
	}
	FCoreUObjectDelegates::GetPostGarbageCollect().Remove(GarbageCollectedHandle);
	GarbageCollectedHandle.Reset();
	Super::NativeDestruct();
}

void UInventoryListViewWidget::BeginDestroy()
{
	bViewActive = false;
	CancelRefresh();
	UnbindSource();
	FCoreUObjectDelegates::GetPostGarbageCollect().Remove(GarbageCollectedHandle);
	Super::BeginDestroy();
}

UInventoryBase* UInventoryListViewWidget::GetSourceInventory() const
{
	return SourceInventory.Get();
}

void UInventoryListViewWidget::SetSourceInventory(UInventoryBase* Inventory)
{
	if (SourceInventory.Get() != Inventory || !IsValid(Inventory))
	{
		CancelRefresh();
		UnbindSource();
		ClearEntries();
		SourceInventory = IsValid(Inventory) ? Inventory : nullptr;
		if (bViewActive)
		{
			BindSource();
		}
	}
	RefreshView();
}

void UInventoryListViewWidget::BindSource()
{
	UnbindSource();
	UInventoryBase* Inventory = SourceInventory.Get();
	if (!bViewActive || !IsValid(Inventory))
	{
		return;
	}
	BoundInventory = Inventory;
	Inventory->OnAddItemDelegate.AddUniqueDynamic(this, &UInventoryListViewWidget::HandleItemAdded);
	Inventory->OnItemRemovedDelegate.AddUniqueDynamic(this, &UInventoryListViewWidget::HandleItemRemoved);
	Inventory->OnStackedItemDelegate.AddUniqueDynamic(this, &UInventoryListViewWidget::HandleItemChanged);
	Inventory->OnUnstackedItemDelegate.AddUniqueDynamic(this, &UInventoryListViewWidget::HandleItemChanged);
	Inventory->OnInventoryRedrawRequested.AddUniqueDynamic(this, &UInventoryListViewWidget::RequestRefresh);
	Inventory->OnConfigurationChanged.AddUniqueDynamic(this, &UInventoryListViewWidget::RequestRefresh);

	UInventoryCollectionComponent* Collection = Inventory->GetItemCollectionLinked();
	if (IsValid(Collection))
	{
		BoundCollection = Collection;
		Collection->OnInventoryItemsChanged.AddUniqueDynamic(this, &UInventoryListViewWidget::HandleCollectionChanged);
		Collection->OnActorInventoriesChanged.AddUniqueDynamic(this, &UInventoryListViewWidget::RequestRefresh);
	}
	AActor* Owner = IsValid(Collection) ? Collection->GetOwner() : Inventory->GetInventoryOwnerActor();
	if (IsValid(Owner))
	{
		BoundOwner = Owner;
		Owner->OnEndPlay.AddUniqueDynamic(this, &UInventoryListViewWidget::HandleSourceEndPlay);
	}
}

void UInventoryListViewWidget::UnbindSource()
{
	if (UInventoryBase* Inventory = BoundInventory.Get())
	{
		Inventory->OnAddItemDelegate.RemoveAll(this);
		Inventory->OnItemRemovedDelegate.RemoveAll(this);
		Inventory->OnStackedItemDelegate.RemoveAll(this);
		Inventory->OnUnstackedItemDelegate.RemoveAll(this);
		Inventory->OnInventoryRedrawRequested.RemoveAll(this);
		Inventory->OnConfigurationChanged.RemoveAll(this);
	}
	if (UInventoryCollectionComponent* Collection = BoundCollection.Get())
	{
		Collection->OnInventoryItemsChanged.RemoveAll(this);
		Collection->OnActorInventoriesChanged.RemoveAll(this);
	}
	if (AActor* Owner = BoundOwner.Get())
	{
		Owner->OnEndPlay.RemoveAll(this);
	}
	for (const TWeakObjectPtr<UObject>& Item : BoundItems)
	{
		UnbindItem(Item.Get());
	}
	BoundItems.Reset();
	BoundInventory.Reset();
	BoundCollection.Reset();
	BoundOwner.Reset();
}

void UInventoryListViewWidget::UnbindItem(UObject* Item)
{
	if (IObjectDataProvider* Provider = Cast<IObjectDataProvider>(Item))
	{
		Provider->GetOnAmountChangedDelegate().RemoveAll(this);
		Provider->GetOnItemDataReplicatedDelegate().RemoveAll(this);
	}
}

void UInventoryListViewWidget::RefreshView()
{
	if (bRefreshing)
	{
		RequestRefresh();
		return;
	}
	TGuardValue<bool> RefreshGuard(bRefreshing, true);
	CancelRefresh();
	UInventoryBase* Inventory = SourceInventory.Get();
	UInventoryCollectionComponent* Collection = IsValid(Inventory) ? Inventory->GetItemCollectionLinked() : nullptr;
	if (bViewActive && (BoundInventory.Get() != Inventory || BoundCollection.Get() != Collection))
	{
		BindSource();
	}
	if (!IsValid(Inventory) || !IsValid(Collection))
	{
		ClearEntries();
		return;
	}

	TMap<UObject*, UInventoryViewEntry*> ExistingEntries;
	for (UInventoryViewEntry* Entry : Entries)
	{
		if (Entry && IsValid(Entry->GetItem()))
		{
			ExistingEntries.Add(Entry->GetItem(), Entry);
		}
	}
	TArray<TObjectPtr<UInventoryViewEntry>> UpdatedEntries;
	TSet<UObject*> CurrentItems;
	for (UObject* Item : Collection->GetAllItemsByContainer(Inventory->GetInventoryContainerID()))
	{
		if (!IsValid(Item) || !Item->Implements<UObjectDataProvider>() || CurrentItems.Contains(Item))
		{
			continue;
		}
		CurrentItems.Add(Item);
		UInventoryViewEntry* Entry = ExistingEntries.FindRef(Item);
		if (!Entry)
		{
			Entry = NewObject<UInventoryViewEntry>(this, NAME_None, RF_Transient);
			Entry->Initialize(Inventory, Item);
		}
		else
		{
			Entry->RefreshDisplayData();
		}
		UpdatedEntries.Add(Entry);
		if (bViewActive)
		{
			if (IObjectDataProvider* Provider = Cast<IObjectDataProvider>(Item))
			{
				Provider->GetOnAmountChangedDelegate().AddUniqueDynamic(this, &UInventoryListViewWidget::HandleAmountChanged);
				Provider->GetOnItemDataReplicatedDelegate().AddUniqueDynamic(this, &UInventoryListViewWidget::HandleItemChanged);
				BoundItems.AddUnique(Item);
			}
		}
	}
	for (int32 Index = BoundItems.Num() - 1; Index >= 0; --Index)
	{
		if (!CurrentItems.Contains(BoundItems[Index].Get()))
		{
			UnbindItem(BoundItems[Index].Get());
			BoundItems.RemoveAtSwap(Index);
		}
	}
	Entries = MoveTemp(UpdatedEntries);
	RebuildVisibleEntries();
}

void UInventoryListViewWidget::RebuildVisibleEntries()
{
	UpdateSortTooltips();
	VisibleEntries.Reset();
	const FString Query = SearchQuery.ToString().TrimStartAndEnd();
	for (UInventoryViewEntry* Entry : Entries)
	{
		if (!Entry || (!CategoryFilters.IsEmpty() && !Entry->Category.MatchesAny(CategoryFilters)))
		{
			continue;
		}
		if (!Query.IsEmpty() && !Entry->DisplayName.ToString().Contains(Query, ESearchCase::IgnoreCase))
		{
			continue;
		}
		VisibleEntries.Add(Entry);
	}
	if (SortMode != EInventoryViewSortMode::SourceOrder)
	{
		const EInventoryViewSortMode Mode = SortMode;
		Algo::StableSort(VisibleEntries, [Mode](const TObjectPtr<UInventoryViewEntry>& Left,
			const TObjectPtr<UInventoryViewEntry>& Right)
		{
			if (Mode == EInventoryViewSortMode::QuantityDescending && Left->Quantity != Right->Quantity)
			{
				return Left->Quantity > Right->Quantity;
			}
			const int32 Comparison = Left->DisplayName.CompareTo(Right->DisplayName);
			return Mode == EInventoryViewSortMode::NameDescending ? Comparison > 0 : Comparison < 0;
		});
	}
	if (!ItemsList)
	{
		return;
	}

	// Quantity changes already reached live rows through OnEntryUpdated. Only reset the
	// ListView's items if membership or ordering changed, preserving its scroll/selection.
	const TArray<UObject*>& PreviousItems = ItemsList->GetListItems();
	bool bListChanged = PreviousItems.Num() != VisibleEntries.Num();
	for (int32 Index = 0; !bListChanged && Index < VisibleEntries.Num(); ++Index)
	{
		bListChanged = PreviousItems[Index] != VisibleEntries[Index];
	}
	if (bListChanged)
	{
		UInventoryViewEntry* Selection = GetSelectedEntry();
		const float ScrollOffset = ItemsList->GetScrollOffset();
		ItemsList->SetListItems(VisibleEntries);
		if (Selection && VisibleEntries.Contains(Selection))
		{
			ItemsList->SetSelectedItem(Selection);
		}
		ItemsList->SetScrollOffset(ScrollOffset);
	}
}

void UInventoryListViewWidget::UpdateSortTooltips()
{
	if (SortByNameButton)
	{
		const FText& Tooltip = SortMode == EInventoryViewSortMode::NameAscending ? NameAscendingActiveTooltip :
			(SortMode == EInventoryViewSortMode::NameDescending ? NameDescendingActiveTooltip : SortByNameTooltip);
		SortByNameButton->SetToolTipText(Tooltip);
	}
	if (SortByQuantityButton)
	{
		SortByQuantityButton->SetToolTipText(SortMode == EInventoryViewSortMode::QuantityDescending ?
			QuantitySortActiveTooltip : SortByQuantityTooltip);
	}
}

void UInventoryListViewWidget::ClearEntries()
{
	if (ItemsList)
	{
		ItemsList->ClearListItems();
	}
	for (const TWeakObjectPtr<UObject>& Item : BoundItems)
	{
		UnbindItem(Item.Get());
	}
	BoundItems.Reset();
	VisibleEntries.Reset();
	Entries.Reset();
}

void UInventoryListViewWidget::SetSearchText(const FText& Text)
{
	SearchQuery = Text;
	if (SearchText && !SearchText->GetText().EqualTo(Text))
	{
		SearchText->SetText(Text);
	}
	RebuildVisibleEntries();
}

void UInventoryListViewWidget::SetCategoryFilters(const FGameplayTagContainer& Categories)
{
	CategoryFilters = Categories;
	RebuildVisibleEntries();
}

void UInventoryListViewWidget::SetSortMode(EInventoryViewSortMode Mode)
{
	SortMode = Mode;
	RebuildVisibleEntries();
}

TArray<UInventoryViewEntry*> UInventoryListViewWidget::GetViewEntries() const
{
	TArray<UInventoryViewEntry*> Result;
	for (UInventoryViewEntry* Entry : Entries)
	{
		Result.Add(Entry);
	}
	return Result;
}

TArray<UInventoryViewEntry*> UInventoryListViewWidget::GetVisibleEntries() const
{
	TArray<UInventoryViewEntry*> Result;
	for (UInventoryViewEntry* Entry : VisibleEntries)
	{
		Result.Add(Entry);
	}
	return Result;
}

UInventoryViewEntry* UInventoryListViewWidget::GetSelectedEntry() const
{
	return ItemsList ? ItemsList->GetSelectedItem<UInventoryViewEntry>() : nullptr;
}

void UInventoryListViewWidget::RequestRefresh()
{
	UWorld* World = GetWorld();
	if (bViewActive && World && !World->bIsTearingDown && !RefreshTimer.IsValid())
	{
		RefreshTimer = World->GetTimerManager().SetTimerForNextTick(this, &UInventoryListViewWidget::RefreshView);
	}
}

void UInventoryListViewWidget::CancelRefresh()
{
	if (RefreshTimer.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(RefreshTimer);
		}
		RefreshTimer.Invalidate();
	}
}

void UInventoryListViewWidget::HandleCollectionChanged(const FString& InventoryID)
{
	if (UInventoryBase* Inventory = SourceInventory.Get())
	{
		if (Inventory->GetInventoryContainerID() == InventoryID)
		{
			RequestRefresh();
		}
	}
}

void UInventoryListViewWidget::HandleItemAdded(FItemMapping& Mapping, UObject* Item)
{
	RequestRefresh();
}

void UInventoryListViewWidget::HandleItemRemoved(FItemMapping Mapping, UObject* Item)
{
	RequestRefresh();
}

void UInventoryListViewWidget::HandleItemChanged(UObject* Item)
{
	RequestRefresh();
}

void UInventoryListViewWidget::HandleAmountChanged(int32 Amount, UObject* Item)
{
	RequestRefresh();
}

void UInventoryListViewWidget::HandleSourceEndPlay(AActor* Actor, EEndPlayReason::Type Reason)
{
	SetSourceInventory(nullptr);
}

void UInventoryListViewWidget::HandleGarbageCollected()
{
	if (!SourceInventory.IsValid() || BoundCollection.IsStale())
	{
		SetSourceInventory(nullptr);
	}
}

void UInventoryListViewWidget::HandleSearchChanged(const FText& Text)
{
	SetSearchText(Text);
}

void UInventoryListViewWidget::HandleSortByName(UUIButton* Button)
{
	SetSortMode(SortMode == EInventoryViewSortMode::NameAscending ?
		EInventoryViewSortMode::NameDescending : EInventoryViewSortMode::NameAscending);
}

void UInventoryListViewWidget::HandleSortByQuantity(UUIButton* Button)
{
	SetSortMode(EInventoryViewSortMode::QuantityDescending);
}

void UInventoryListViewWidget::NativeOnDragDetected(const FGeometry& Geometry, const FPointerEvent& Event,
	UDragDropOperation*& OutOperation)
{
	OutOperation = nullptr;
}

bool UInventoryListViewWidget::NativeOnDrop(const FGeometry& Geometry, const FDragDropEvent& Event,
	UDragDropOperation* Operation)
{
	return IsValid(Cast<UItemDragDropOperation>(Operation));
}
