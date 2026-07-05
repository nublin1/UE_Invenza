// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ModalButtonsPanelInterface.generated.h"

class UUIButton;
// This class does not need to be modified.
UINTERFACE()
class UModalButtonsPanelInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYSYSTEMINVENZAPLUGIN_API IModalButtonsPanelInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Modal")
	TArray<UUIButton*> GetButtons();
	
	UFUNCTION(BlueprintNativeEvent, Category = "Modal")
	bool IsInteractionEnabled() const;
};
