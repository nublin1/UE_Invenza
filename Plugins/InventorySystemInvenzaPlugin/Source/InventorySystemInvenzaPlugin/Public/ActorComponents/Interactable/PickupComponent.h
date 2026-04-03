//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractableComponent.h"
#include "Components/ActorComponent.h"
#include "Data/ItemDataStructures.h"
#include "PickupComponent.generated.h"


struct FInitItemsEntry;
class UItemBase;
class UBoxComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UPickupComponent : public UInteractableComponent
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UPickupComponent();
	
	virtual void BeginFocus() override;
	virtual void EndFocus() override;
	virtual void Interact(UInteractionComponent* InteractionComponent) override;
	
	UFUNCTION(BlueprintCallable, Category = "Pickup | Initialization")
	virtual void InitializeDrop(UItemBase* ItemToDrop);

	//Getters
	UFUNCTION(BlueprintCallable, Category = "Pickup | Getters")
	FInitItemsEntry GetInitialItem() { return InitialItem; }

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CachedMesh;
	
	UPROPERTY(EditAnywhere, Category = "Pickup | Item Initialization")
	FInitItemsEntry InitialItem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup | Item Reference")
	TObjectPtr<UItemBase> ItemBase;
		
	UPROPERTY(EditAnywhere, Category = "Pickup | Debug")
	bool bIsDebug = false;

	bool bIsPendingDestruction = false;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
		
	UFUNCTION()
	void InitializePickupComponent();

	UFUNCTION()
	virtual void TakePickup(UInteractionComponent *Taker);

	virtual void UpdateInteractableData() override;
};
