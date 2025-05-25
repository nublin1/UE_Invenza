// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/EquipmentStructures.h"
#include "EquipmentManagerComponent.generated.h"

class UInvBaseContainerWidget;
struct FInventorySlotData;
struct FItemMapping;
class UItemBase;

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItem, FName, SlotName, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnequippedItem, FName, SlotName, UItemBase*, Item);
#pragma endregion

UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class INVENTORYSYSTEMINVENZAPLUGIN_API UEquipmentManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable, Category = "Equipment")
	FOnEquippedItem OnEquippedItem;
	
	UPROPERTY(BlueprintAssignable, Category = "Equipment")
	FOnUnequippedItem OnUnequippedItem;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UEquipmentManagerComponent();

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void Initialize();

	UFUNCTION()
	void ValidateEquippedItems();
	UFUNCTION(BlueprintCallable)
	void HandleReplaceItem(TArray<FInventorySlotData> OldItemSlots, TArray<FInventorySlotData> NewItemSlots, UItemBase* Item);
	UFUNCTION(BlueprintCallable)
	void HandleItemEquippedFromMapping(FItemMapping ItemSlots, UItemBase* Item);
	UFUNCTION(BlueprintCallable)
	void EquipItemToSlot(TArray<FInventorySlotData>& ItemSlotsData, UItemBase* Item);
	UFUNCTION(BlueprintCallable)
	void EquipItem(UItemBase* Item);

	UFUNCTION(BlueprintCallable)
	void HandleItemUnequippedFromMapping(FItemMapping ItemSlots, UItemBase* Item, int32 RemoveQuantity);
	UFUNCTION(BlueprintCallable)
	void UnequipItemFromSlot(TArray<FInventorySlotData>& ItemSlotsData, UItemBase* Item, int32 RemoveQuantity);

	UFUNCTION(BlueprintCallable)
	TMap<UItemBase*, FEquipmentSlot> GetEquippedItemsData();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FName, FEquipmentSlot> EquipmentSlots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config")
	TObjectPtr<UDataTable> SlotDefinitionTable;

	//
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category="Config")
	TObjectPtr<UInvBaseContainerWidget> CharacterEquipmentWidget = nullptr;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void InitializeSlotsFromTable();
	UFUNCTION()
	virtual void BindWidgetsToSlots();

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
