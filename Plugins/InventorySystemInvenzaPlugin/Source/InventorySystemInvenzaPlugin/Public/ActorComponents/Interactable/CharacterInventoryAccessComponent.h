//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "ActorComponents/Interactable/ContainerComponent.h"
#include "CharacterInventoryAccessComponent.generated.h"

/** Allows another character to access this owner's bag and equipment during an interaction. */
UCLASS(ClassGroup = (Inventory), Blueprintable, meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UCharacterInventoryAccessComponent : public UContainerComponent
{
	GENERATED_BODY()

public:
	UCharacterInventoryAccessComponent();

protected:
	// Unreal overrides
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Inventory")
	FText CharacterDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Inventory")
	FGameplayTag EquipmentInventoryTag;

	/** Creates a small demonstration bag and equipment grid when no startup inventories are configured. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Inventory")
	bool bCreateDefaultInventories = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Inventory",
			  meta = (EditCondition = "bCreateDefaultInventories"))
	TArray<FInitItemsEntry> BagStartingItems;

	// Functions
	UFUNCTION(BlueprintPure, Category = "Character Inventory")
	UInventoryBase* GetEquipmentInventory() const { return EquipmentInventory; }

	bool ContainsInventory(UInventoryBase* Inventory) const;

	bool TryBeginAccess(AActor* Interactor);

	void EndAccess(AActor* Interactor);

	bool IsAccessedBy(const AActor* Interactor) const { return CurrentAccessor == Interactor; }

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Transient, ReplicatedUsing = OnRep_MainLootInventory)
	TObjectPtr<UInventoryBase> EquipmentInventory;

	TWeakObjectPtr<AActor> CurrentAccessor;

	// Functions
	virtual void InitializeInteractionComponent() override;
};
