// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStructures.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractionInterface.h"
#include "Pickup.generated.h"

class UBoxComponent;

UCLASS()
class GRIDINVENTORYPLUGIN_API APickup : public AActor,  public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	APickup();

	virtual void BeginFocus() override;
	virtual void EndFocus() override;

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
	FItemMetaData ItemRef;
	UPROPERTY(EditInstanceOnly, Category = "Pickup | Item Initialization")
	int32 ItemQuantity;


	UPROPERTY()
	bool bIsDebug = false;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void PostInitializeComponents() override;

	UFUNCTION()
	void InitializePickup();

};
