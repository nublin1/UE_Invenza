// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UIInterface.generated.h"

class UUIButton;
// This class does not need to be modified.
UINTERFACE()
class UUIInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYSYSTEMINVENZAPLUGIN_API IUIInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Craft Toolbar")
	TArray<UUIButton*> GetButtons() const;
};
