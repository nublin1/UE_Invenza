// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/Equipment/EquipmentSlotDefinition.h"
#include "EquipmentComponent.generated.h"

class UInventoryContainerWidget;
struct FInventorySlotData;
struct FItemMapping;
class UItemBase;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItem, FGameplayTag, SlotTag, UObject*, Item);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnequippedItem, FGameplayTag, SlotTag, UObject*, Item);
#pragma endregion

public:
	UEquipmentComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
	                                 struct FReplicationFlags* RepFlags) override;

protected:
	virtual void BeginPlay() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Equipment|Events")
	FOnEquippedItem OnEquippedItem;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Equipment|Events")
	FOnUnequippedItem OnUnequippedItem;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(Category = "Equipment|Initialization")
	virtual void InitializeSlotsFromTable();

	/**
	 * Checks whether the item category is compatible with the specified slot.
	 * This only validates category/slot compatibility and does not check whether the slot is occupied.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
	bool IsCategoryCompatibleWithSlot(FGameplayTag SlotTag, FGameplayTag ItemCategory) const;

	/**
	 * Checks whether the specified item can be equipped to the slot.
	 * Validates all equip conditions, including category compatibility and whether the slot is available.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
	bool CanEquipItemToSlot(const UObject* Item, FGameplayTag SlotTag) const;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Equipment|Management")
	void Server_EquipItem(UObject* Item);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Equipment|Management")
	void Server_EquipItemToSlot(FGameplayTag SlotTag, UObject* Item);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Equipment|Management")
	void Server_UnequipItemFromSlot(FGameplayTag SlotTag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
	bool IsItemEquipped(const UObject* Item) const;
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool FindSlotByItem(const UObject* Item, FGameplayTag& OutSlotTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
	bool DoesSlotExist(FGameplayTag SlotTag) const;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Equipment|Config")
	TArray<FDataTableRowHandle> SlotRows;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_EquipmentSlots, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Slots")
	TArray<FEquipmentSlotRuntime> EquipmentSlotsArray;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void OnRep_EquipmentSlots();

	UFUNCTION(Category = "Equipment|Management")
	void ResourceAmountChanged(int32 AmountChanged, UObject* Item);

	FEquipmentSlotRuntime* FindSlot(FGameplayTag SlotTag);
	const FEquipmentSlotRuntime* FindSlot(FGameplayTag SlotTag) const;
};
