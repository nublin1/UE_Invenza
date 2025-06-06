// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
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
	UPROPERTY(BlueprintAssignable, BlueprintCallable,Category = "Equipment|Events")
	FOnEquippedItem OnEquippedItem;
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable,Category = "Equipment|Events")
	FOnUnequippedItem OnUnequippedItem;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UEquipmentManagerComponent();

	UFUNCTION(BlueprintCallable, Category = "Equipment|Initialization")
	virtual void Initialize();

	UFUNCTION(BlueprintCallable, Category = "Equipment|Validation")
	void ValidateEquippedItems();
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	void HandleReplaceItem(TArray<FInventorySlotData> OldItemSlots, TArray<FInventorySlotData> NewItemSlots, UItemBase* Item);
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	void HandleItemEquippedFromMapping(FItemMapping ItemSlots, UItemBase* Item);
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	void EquipItemToSlot(TArray<FInventorySlotData>& ItemSlotsData, UItemBase* Item);
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	void EquipItem(UItemBase* Item);

	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	void HandleItemUnequippedFromMapping(FItemMapping ItemSlots, UItemBase* Item, int32 RemoveQuantity);
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	void UnequipItemFromSlot(TArray<FInventorySlotData>& ItemSlotsData, UItemBase* Item, int32 RemoveQuantity);

	UFUNCTION(BlueprintCallable, Category = "Equipment|Data")
	TMap<UItemBase*, FEquipmentSlot> GetEquippedItemsData();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Slots")
	TMap<FName, FEquipmentSlot> EquipmentSlots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Equipment|Config")
	TObjectPtr<UDataTable> SlotDefinitionTable;

	//
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category="Equipment|Config")
	TObjectPtr<UInvBaseContainerWidget> CharacterEquipmentWidget = nullptr;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void BeginPlay() override;

	UFUNCTION(Category = "Equipment|Initialization")
	virtual void InitializeSlotsFromTable();
	UFUNCTION(Category = "Equipment|Initialization")
	virtual void BindWidgetsToSlots();

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
