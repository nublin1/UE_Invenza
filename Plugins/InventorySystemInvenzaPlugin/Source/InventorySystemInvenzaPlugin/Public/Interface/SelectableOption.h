// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "UObject/Interface.h"
#include "SelectableOption.generated.h"

// This class does not need to be modified.
UINTERFACE()
class USelectableOption : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYSYSTEMINVENZAPLUGIN_API ISelectableOption
{
	GENERATED_BODY()

public:
	DECLARE_EVENT_OneParam(ISelectableOption, FOnSelectionRequested, UWidget* /* Sender */);
	virtual FOnSelectionRequested& OnSelectionRequested() = 0;

	virtual void SetSelected(bool bIsSelected) = 0;
	virtual bool IsSelected() const = 0;
};
