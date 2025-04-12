//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "UI/BaseUserWidget.h"
#include "UInventoryWidgetBase.generated.h"

#pragma region Delegates
class UItemCollection;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUpdateDelegate, const FItemMapping&, ItemSlots, UItemBase*,
                                             Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAddItemDelegate, FItemMapping, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRemoveItemDelegate, FItemMapping, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUseItemDelegate, USlotbasedInventorySlot*, ItemSlot, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWightUpdatedDelegate, float, InventoryTotalWeight, float, InventoryWeightCapacity);
#pragma endregion Delegates

/**
 * 
 */
UCLASS(Abstract)
class GRIDINVENTORYPLUGIN_API UUInventoryWidgetBase : public UBaseUserWidget
{
	GENERATED_BODY()
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	FOnItemUpdateDelegate OnItemUpdateDelegate;
	FOnAddItemDelegate OnAddItemDelegate;	
	FOnRemoveItemDelegate OnRemoveItemDelegate;
	FOnUseItemDelegate OnUseItemDelegate;
	FOnWightUpdatedDelegate OnWightUpdatedDelegate;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(Category="Inventory")
	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity) PURE_VIRTUAL(UUInventoryWidgetBase::HandleRemoveItem,);
	UFUNCTION(Category="Inventory")
	virtual void HandleRemoveItemFromContainer(UItemBase* Item) PURE_VIRTUAL(UUInventoryWidgetBase::HandleRemoveItemFromContainer,);
	UFUNCTION(BlueprintCallable, Category="Inventory")
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) PURE_VIRTUAL(UUInventoryWidgetBase::HandleAddItem, return FItemAddResult(););

	UFUNCTION(Category="Inventory")
	FORCEINLINE UItemCollection* GetItemCollection() {return ItemCollectionLink;}
	UFUNCTION(Category="Inventory")
	FORCEINLINE bool GetCanReferenceItems() const {return bCanReferenceItems;}
	UFUNCTION(Category="Inventory")
	FORCEINLINE bool GetIsUseReferences() const {return bUseReferences;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Data
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory")
	TObjectPtr<UItemCollection> ItemCollectionLink;

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(tooltip="If true this container will be used as reference."))
	bool bUseReferences = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(tooltip="Can the items be referenced from this container"))
	bool bCanReferenceItems = false;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	
};
