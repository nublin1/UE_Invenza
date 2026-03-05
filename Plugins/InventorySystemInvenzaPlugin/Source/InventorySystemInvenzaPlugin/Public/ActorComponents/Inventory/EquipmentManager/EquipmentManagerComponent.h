// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "Data/EquipmentStructures.h"
#include "Data/Inventory/InventorySlotData.h"
#include "EquipmentManagerComponent.generated.h"

class UEquipmentSlotData;
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
	UEquipmentManagerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable, BlueprintCallable,Category = "Equipment|Events")
	FOnEquippedItem OnEquippedItem;
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable,Category = "Equipment|Events")
	FOnUnequippedItem OnUnequippedItem;

	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION(BlueprintCallable, Category = "Equipment|Initialization")
	virtual void Initialize();

	UFUNCTION(Category = "Equipment|Initialization")
	virtual void InitializeSlotsFromTable();
	
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	void HandleReplaceItem(TArray<UInventorySlotData*> OldItemSlots, TArray<UInventorySlotData*> NewItemSlots, UItemBase* Item);
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	void HandleItemEquippedFromMapping(FItemMapping ItemSlots, UItemBase* Item);
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	void EquipItemToSlot(FName SlotName, UItemBase* Item);
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	bool EquipItem(UItemBase* Item);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	void UnequipItemFromSlot(FName SlotName, UItemBase* Item);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	/** Initial items with their quantities */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Config")
	TArray<FInitItemsEntry> InitialItems;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Equipment|Config")
	TObjectPtr<UDataTable> SlotDefinitionTable;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Slots")
	TMap<FName, TObjectPtr<UEquipmentSlotData>> EquipmentSlots;

	//
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category="Equipment|Config")
	TObjectPtr<UInvBaseContainerWidget> CharacterEquipmentWidget = nullptr;

	// Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName InventoryContainerID = NAME_None; // Uniq ID
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION(Category = "Equipment|Initialization")
	virtual void BindWidgetsToSlots();
	
	UFUNCTION(Category = "Equipment|Management")
	void ResourceAmountChanged(int32 AmountChanged, UItemBase* Item);
	
};
