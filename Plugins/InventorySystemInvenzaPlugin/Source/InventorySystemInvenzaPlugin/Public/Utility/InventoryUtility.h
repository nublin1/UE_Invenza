//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryUtility.generated.h"

class IInventoryInteractionHandler;
class UInvenzaInventoryUISettingsAsset;
enum class EItemOrientationType : uint8;
class UInventoryBase;
class UItemBase;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "InventorySystemInvenza")
	static bool AddItemQuantity(UObject* Outer, UInventoryBase* TargetInventory, UItemBase* ItemSample, int32 TotalQuantity);

	UFUNCTION(BlueprintCallable, Category = "InventorySystemInvenza")
	static FVector2D CalculateItemVisualSize(UItemBase* Item, EItemOrientationType Orientation, FVector2D SlotSize, FMargin SlotSpacing, bool bIgnoreSize);

	UFUNCTION(BlueprintCallable, Category = "InventorySystemInvenza")
	static const UInvenzaInventoryUISettingsAsset* GetInvenzaGlobalSettings(const UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "InventorySystemInvenza")
	static TScriptInterface<IInventoryInteractionHandler> FindInventoryHandler(AActor* Actor);
};
