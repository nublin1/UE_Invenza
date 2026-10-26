//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "InventoryMannequin.generated.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class UInventoryCollectionComponent;
class UEquipmentComponent;
class UCharacterInventoryAccessComponent;

/** Stationary example of an NPC whose bag and equipment can be accessed. */
UCLASS(Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API AInventoryMannequin : public AActor
{
	GENERATED_BODY()

public:
	AInventoryMannequin();

protected:
	// Unreal overrides
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;

public:
	// Properties and variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mannequin")
	TObjectPtr<UCapsuleComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mannequin")
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryCollectionComponent> InventoryCollection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UEquipmentComponent> Equipment;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UCharacterInventoryAccessComponent> InventoryAccess;

protected:
	// Runtime
	// Functions
	void ApplyExampleDefaults();
};
