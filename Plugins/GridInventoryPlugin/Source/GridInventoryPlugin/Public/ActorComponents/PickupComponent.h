// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableComponent.h"
#include "Components/ActorComponent.h"
#include "Data/ItemDataStructures.h"
#include "PickupComponent.generated.h"


class UItemBase;
class UBoxComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRIDINVENTORYPLUGIN_API UPickupComponent : public UInteractableComponent
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

	UFUNCTION()
	virtual void Interact(UInteractionComponent* InteractionComponent) override;

	//Getters
	UItemBase* GetItemBase() {return ItemBase;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, Category = "Pickup | Components")
	TObjectPtr<UStaticMeshComponent> PickupMesh;
	UPROPERTY(VisibleAnywhere, Category = "Pickup | Components")
	TObjectPtr<UBoxComponent> BoxCollider;
	
	UPROPERTY(EditAnywhere, Category = "Pickup | Item Initialization")
	TObjectPtr<UDataTable>ItemDataTable;
	UPROPERTY(EditAnywhere, Category = "Pickup | Item Initialization")
	FName DesiredItemID;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup | Item Reference")
	TObjectPtr<UItemBase> ItemBase;


	UPROPERTY()
	bool bIsDebug = false;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void InitializePickup();

	UFUNCTION()
	virtual void TakePickup(const UInteractionComponent *Taker);

	virtual void UpdateInteractableData() override;
};
