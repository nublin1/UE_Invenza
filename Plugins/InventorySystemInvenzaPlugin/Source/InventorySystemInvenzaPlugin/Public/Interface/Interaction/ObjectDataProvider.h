// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ObjectDataProvider.generated.h"

enum class EObjectInteractionType : uint8;
class UInvenzaInventorySettingsAsset;
// This class does not need to be modified.
UINTERFACE()
class UObjectDataProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYSYSTEMINVENZAPLUGIN_API IObjectDataProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	
public:	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData")
	bool CanPerformAction(EObjectInteractionType Action, const UInvenzaInventorySettingsAsset* SettingsAsset = nullptr);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData")
	bool IsStackable();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData")
	FVector2D GetMinMaxSplit();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData")
	bool IsFullItemStack();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData")
	float GetItemStackWeight();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData")
	float GetItemSingleWeight();
};
