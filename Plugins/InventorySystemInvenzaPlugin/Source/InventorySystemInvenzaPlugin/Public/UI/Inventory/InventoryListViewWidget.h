// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Inventory/InventoryViewTypes.h"
#include "InventoryListViewWidget.generated.h"

class UEditableText;
class UInventoryBase;
class UInventoryCollectionComponent;
class UInventoryViewEntry;
class UListView;
class UUIButton;
struct FItemMapping;

/** Independent, read-only list projection of any existing Invenza container. */
UCLASS(Abstract, Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryListViewWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void BeginDestroy() override;
	virtual void NativeOnDragDetected(const FGeometry& Geometry, const FPointerEvent& Event,
		UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& Geometry, const FDragDropEvent& Event,
		UDragDropOperation* Operation) override;

public:
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Inventory View")
	void SetSourceInventory(UInventoryBase* Inventory);

	UFUNCTION(BlueprintPure, Category = "Inventory View")
	UInventoryBase* GetSourceInventory() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory View")
	void RefreshView();

	UFUNCTION(BlueprintCallable, Category = "Inventory View|Filters")
	void SetSearchText(const FText& Text);

	UFUNCTION(BlueprintPure, Category = "Inventory View|Filters")
	FText GetSearchText() const { return SearchQuery; }

	/** An empty filter shows all categories; tags include their child categories. */
	UFUNCTION(BlueprintCallable, Category = "Inventory View|Filters")
	void SetCategoryFilters(const FGameplayTagContainer& Categories);

	UFUNCTION(BlueprintCallable, Category = "Inventory View|Sorting")
	void SetSortMode(EInventoryViewSortMode Mode);

	UFUNCTION(BlueprintPure, Category = "Inventory View|Sorting")
	EInventoryViewSortMode GetSortMode() const { return SortMode; }

	UFUNCTION(BlueprintPure, Category = "Inventory View")
	TArray<UInventoryViewEntry*> GetViewEntries() const;

	UFUNCTION(BlueprintPure, Category = "Inventory View")
	TArray<UInventoryViewEntry*> GetVisibleEntries() const;

	UFUNCTION(BlueprintPure, Category = "Inventory View")
	UInventoryViewEntry* GetSelectedEntry() const;

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, Category = "Inventory View", meta = (BindWidget))
	TObjectPtr<UListView> ItemsList;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory View", meta = (BindWidgetOptional))
	TObjectPtr<UEditableText> SearchText;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory View", meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> SortByNameButton;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory View", meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> SortByQuantityButton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory View|Sorting")
	EInventoryViewSortMode SortMode = EInventoryViewSortMode::NameAscending;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory View|Text")
	FText SortByNameTooltip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory View|Text")
	FText NameAscendingActiveTooltip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory View|Text")
	FText NameDescendingActiveTooltip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory View|Text")
	FText SortByQuantityTooltip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory View|Text")
	FText QuantitySortActiveTooltip;

	UPROPERTY(Transient)
	TWeakObjectPtr<UInventoryBase> SourceInventory;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInventoryViewEntry>> Entries;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInventoryViewEntry>> VisibleEntries;

	UPROPERTY(Transient)
	FText SearchQuery;

	UPROPERTY(Transient)
	FGameplayTagContainer CategoryFilters;

	TWeakObjectPtr<UInventoryBase> BoundInventory;
	TWeakObjectPtr<UInventoryCollectionComponent> BoundCollection;
	TWeakObjectPtr<AActor> BoundOwner;
	TArray<TWeakObjectPtr<UObject>> BoundItems;
	FTimerHandle RefreshTimer;
	FDelegateHandle GarbageCollectedHandle;
	bool bViewActive = false;
	bool bRefreshing = false;

	// Functions
	void BindSource();
	void UnbindSource();
	void UnbindItem(UObject* Item);
	void CancelRefresh();
	void RebuildVisibleEntries();
	void UpdateSortTooltips();
	void ClearEntries();

	UFUNCTION()
	void RequestRefresh();

	UFUNCTION()
	void HandleCollectionChanged(const FString& InventoryID);

	UFUNCTION()
	void HandleItemAdded(FItemMapping& Mapping, UObject* Item);

	UFUNCTION()
	void HandleItemRemoved(FItemMapping Mapping, UObject* Item);

	UFUNCTION()
	void HandleItemChanged(UObject* Item);

	UFUNCTION()
	void HandleAmountChanged(int32 Amount, UObject* Item);

	UFUNCTION()
	void HandleSourceEndPlay(AActor* Actor, EEndPlayReason::Type Reason);

	UFUNCTION()
	void HandleSearchChanged(const FText& Text);

	UFUNCTION()
	void HandleSortByName(UUIButton* Button);

	UFUNCTION()
	void HandleSortByQuantity(UUIButton* Button);

	void HandleGarbageCollected();
};
