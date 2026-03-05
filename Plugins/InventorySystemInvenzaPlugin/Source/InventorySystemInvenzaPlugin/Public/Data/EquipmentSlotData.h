// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemDataStructures.h"
#include "EquipmentSlotData.generated.h"

UCLASS(BlueprintType)
class INVENTORYSYSTEMINVENZAPLUGIN_API UEquipmentSlotData : public UObject
{
	GENERATED_BODY()

public:
	UEquipmentSlotData();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory | Slot")
	FName SlotName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	EItemCategory AllowedCategory = EItemCategory::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory | Slot")
	TObjectPtr<UItemBase> ItemEquipped = nullptr;
};
