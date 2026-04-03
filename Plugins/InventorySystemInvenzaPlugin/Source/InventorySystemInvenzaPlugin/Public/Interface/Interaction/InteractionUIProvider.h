//  Nublin Studio 2026 All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "UI/Interaction/InteractionWidget.h"
#include "UObject/Interface.h"
#include "InteractionUIProvider.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInteractionUIProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYSYSTEMINVENZAPLUGIN_API IInteractionUIProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual UInteractionWidget* GetPawnInteractionWidget() const
	PURE_VIRTUAL(IInteractionUIProvider::GetPawnInteractionWidget, return nullptr; );
};
