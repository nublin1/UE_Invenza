//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractableComponent.h"
#include "Components/ActorComponent.h"
#include "Data/ItemDataStructures.h"
#include "Interface/Interaction/Pickupableass.h"
#include "PickupComponent.generated.h"


struct FInitItemsEntry;
class UItemBase;
class UBoxComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UPickupComponent : public UInteractableComponent, public IPickupableass
{
	GENERATED_BODY()

public:
	UPickupComponent();

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

public:	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void BeginFocus() override;
	virtual void EndFocus() override;
	virtual void Interact(UInteractionComponent* InteractionComponent) override;
	
	UFUNCTION(BlueprintCallable, Category = "Pickup | Initialization")
	virtual void InitializeDrop(UItemBase* ItemToDrop);

	//Getters
	UFUNCTION(BlueprintCallable, Category = "Pickup | Getters")
	FInitItemsEntry GetInitialItem() { return InitialItem; }
	UFUNCTION(BlueprintCallable, Category = "Pickup | Getters")
	virtual UItemBase* GetItemData() override;

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	virtual void OnPickedUp() override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CachedMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup | Item Initialization")
	FInitItemsEntry InitialItem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup | Item Reference")
	TObjectPtr<UItemBase> ItemBase;

	bool bIsPendingDestruction = false;

	//====================================================================
	// FUNCTIONS
	//====================================================================
		
	UFUNCTION()
	void InitializePickupComponent();

	virtual void UpdateInteractableData() override;
};
