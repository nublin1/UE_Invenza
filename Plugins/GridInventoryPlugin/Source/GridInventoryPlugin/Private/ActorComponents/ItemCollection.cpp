// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/ItemCollection.h"

#include "ActorComponents/Items/itemBase.h"
#include "UI/Inventory/BaseInventoryWidget.h"


UItemCollection::UItemCollection()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UItemCollection::AddItem(UItemBase* NewItem,  UBaseInventoryWidget* Container)
{
	ItemContainers.FindOrAdd(NewItem).Add(Container);
	//UE_LOG(LogTemp, Warning, TEXT("Widget already exists for Item: %s"), *NewItem->GetName());
}

void UItemCollection::RemoveItem(UItemBase* Item, UBaseInventoryWidget* Container)
{
	if (auto* InventoryWidgets = ItemContainers.Find(Item))
	{
		if (InventoryWidgets->Remove(Container) > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully removed Widget for Item: %s"), *Item->GetName());
		}
	}
}

void UItemCollection::RemoveItemFromAllContainers(UItemBase* Item)
{
	if (auto* InventoryWidgets = ItemContainers.Find(Item))
	{
		for (auto* InventoryWidget : *InventoryWidgets)
		{
			if (UBaseInventoryWidget* CastedWidget = Cast<UBaseInventoryWidget>(InventoryWidget))
			{
				CastedWidget->HandleRemoveItem(Item);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to cast widget to UBaseInventoryWidget for Item: %s"), *Item->GetName());
			}
		}
		
		InventoryWidgets->Empty();
		ItemContainers.Remove(Item);
        
		//UE_LOG(LogTemp, Log, TEXT("Item %s removed from all containers"), *Item->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Item %s not found in ItemContainers"), *Item->GetName());
	}
}

