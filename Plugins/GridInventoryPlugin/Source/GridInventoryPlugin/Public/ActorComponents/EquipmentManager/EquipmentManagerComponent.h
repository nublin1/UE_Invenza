// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/EquipmentStructures.h"
#include "EquipmentManagerComponent.generated.h"

class UItemBase;


#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItem, FName, SlotName, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnequippedItem, FName, SlotName, UItemBase*, Item);
#pragma endregion

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GRIDINVENTORYPLUGIN_API UEquipmentManagerComponent : public UActorComponent
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

	UFUNCTION(BlueprintCallable)
	void InitializeSlotsFromTable();

	UFUNCTION(BlueprintCallable)
	bool EquipItemToSlot(FName SlotName, UItemBase* Item);
	UFUNCTION(BlueprintCallable)
	bool EquipItem(UItemBase* Item);

	UFUNCTION(BlueprintCallable)
	bool UnequipItem(FName SlotName);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere)
	TMap<FName, FEquipmentSlot> EquipmentSlots;

	UPROPERTY(EditDefaultsOnly, Category="Config")
	UDataTable* SlotDefinitionTable;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
