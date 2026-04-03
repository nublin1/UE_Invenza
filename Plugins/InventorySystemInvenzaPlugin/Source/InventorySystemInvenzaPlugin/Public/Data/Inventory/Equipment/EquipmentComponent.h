// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/Equipment/EquipmentSlotDefinition.h"
#include "EquipmentComponent.generated.h"

class UInvBaseContainerWidget;
struct FInventorySlotData;
struct FItemMapping;
class UItemBase;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INVENTORYSYSTEMINVENZAPLUGIN_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()
	
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItem, FGameplayTag, SlotTag, UItemBase*, Item);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnequippedItem, FGameplayTag, SlotTag, UItemBase*, Item);
#pragma endregion

public:
	UEquipmentComponent();

protected:
	virtual void BeginPlay() override;
	
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
	UFUNCTION(Category = "Equipment|Initialization")
	virtual void InitializeSlotsFromTable();
	
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	bool EquipItem(UItemBase* Item);
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	bool EquipItemToSlot(FGameplayTag SlotTag, UItemBase* Item);	
	UFUNCTION(BlueprintCallable, Category = "Equipment|Management")
	void UnequipItemFromSlot(FGameplayTag SlotTag);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Equipment|Config")
	TArray<FDataTableRowHandle> SlotRows;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Slots")
	TMap<FGameplayTag, FEquipmentSlotRuntime> EquipmentSlots;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(Category = "Equipment|Management")
	void ResourceAmountChanged(int32 AmountChanged, UItemBase* Item);
	
};
