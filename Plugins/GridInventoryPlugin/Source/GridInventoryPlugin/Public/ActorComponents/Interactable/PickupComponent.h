// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableComponent.h"
#include "Components/ActorComponent.h"
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
	virtual void Interact(UInteractionComponent* InteractionComponent) override;
	
	UFUNCTION(BlueprintCallable, Category = "Pickup | Initialization")
	virtual void InitializeDrop(UItemBase* ItemToDrop);

	//Getters
	UFUNCTION(BlueprintCallable, Category = "Pickup | Getters")
	UItemBase* GetItemBase() { return ItemBase; }

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================	
	UPROPERTY(EditAnywhere, Category = "Pickup | Item Initialization")
	FName DesiredItemID;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup | Item Reference")
	TObjectPtr<UItemBase> ItemBase;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup | Item Reference")
	int InitQuantity = 1;
	
	UPROPERTY(EditAnywhere, Category = "Pickup | Debug")
	bool bIsDebug = false;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
		
	UFUNCTION()
	void InitializePickupComponent();

	UFUNCTION()
	virtual void TakePickup(const UInteractionComponent *Taker);

	virtual void UpdateInteractableData() override;
};
