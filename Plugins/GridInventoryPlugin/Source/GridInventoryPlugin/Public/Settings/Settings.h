//  Nublin Studio 2025 All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Settings.generated.h"

class APickup;
class UInventoryItemWidget;

USTRUCT(Blueprintable)
struct FUISettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UInventoryItemWidget> InventoryItemVisualClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UInventoryItemWidget> DraggedWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ContainerWidgetName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D SlotSize = FVector2D(0.f);
	
};

USTRUCT(Blueprintable)
struct FRegularSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> PickupClass;
};