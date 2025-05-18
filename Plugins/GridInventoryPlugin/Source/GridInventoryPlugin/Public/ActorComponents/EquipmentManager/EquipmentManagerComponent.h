// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/EquipmentStructures.h"
#include "EquipmentManagerComponent.generated.h"

struct FItemMapping;
class UItemBase;

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItem, FName, SlotName, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnequippedItem, FName, SlotName, UItemBase*, Item);
#pragma endregion

UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
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

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void Initialize();

	UFUNCTION(BlueprintCallable)
	void EquipItemToSlot(FItemMapping ItemSlots, UItemBase* Item);
	UFUNCTION(BlueprintCallable)
	void EquipItem(UItemBase* Item);

	UFUNCTION(BlueprintCallable)
	void UnequipItemFromSlot(FItemMapping ItemSlots, UItemBase* Item, int32 RemoveQuantity);

	UFUNCTION(BlueprintCallable)
	TMap<UItemBase*, FEquipmentSlot> GetEquippedItemsData();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FName, FEquipmentSlot> EquipmentSlots;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category="Config")
	TObjectPtr<UDataTable> SlotDefinitionTable;
	
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
