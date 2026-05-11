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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, struct FReplicationFlags* RepFlags) override;

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
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Equipment|Management")
	void Server_EquipItem(UItemBase* Item);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Equipment|Management")
	void Server_EquipItemToSlot(FGameplayTag SlotTag, UItemBase* Item);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Equipment|Management")
	void Server_UnequipItemFromSlot(FGameplayTag SlotTag);

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
	void ResourceAmountChanged(int32 AmountChanged, UItemBase* Item);

	FEquipmentSlotRuntime* FindSlot(FGameplayTag SlotTag);
};
