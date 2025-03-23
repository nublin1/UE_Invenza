// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemCollection.generated.h"


class UBaseInventoryWidget;
class UItemBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRIDINVENTORYPLUGIN_API UItemCollection : public UActorComponent
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	/** Data table containing item information */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Collection|Config")
	TObjectPtr<UDataTable> ItemDataTable;

	/** Map of initial items with their quantities */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Collection|Config")
	TMap<FName, int32> InitItems;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UItemCollection();
	
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void AddItem(UItemBase* NewItem, UBaseInventoryWidget* Container);
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void RemoveItem(UItemBase* Item, UBaseInventoryWidget* Container);
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void RemoveItemFromAllContainers(UItemBase* Item);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	TMap<UItemBase*, TArray<UBaseInventoryWidget*>> ItemContainers;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================


};
