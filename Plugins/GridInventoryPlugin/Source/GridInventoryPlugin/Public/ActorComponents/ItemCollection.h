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
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UItemCollection();

	void AddItem(UItemBase* NewItem, UBaseInventoryWidget* Container);
	void RemoveItem(UItemBase* Item, UBaseInventoryWidget* Container);
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
