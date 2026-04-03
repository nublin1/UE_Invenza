//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Inventory/UInventoryWidgetBase.h"
#include "UObject/Interface.h"
#include "InvUIProvider.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInvUIProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYSYSTEMINVENZAPLUGIN_API IInvUIProvider
{
	GENERATED_BODY()

public:
	virtual TArray<UUInventoryWidgetBase*> GetAllPawnInventories() const
	PURE_VIRTUAL(IInvUIProvider::GetAllPawnInventories, return TArray<UUInventoryWidgetBase*>(););

	virtual TArray<UInvBaseContainerWidget*> GetAllPawnInvContainers() const
	PURE_VIRTUAL(IInvUIProvider::GetAllPawnInvContainers, return TArray<UInvBaseContainerWidget*>(););

	virtual UPanelSlot* AddPawnInvContainers(UInvBaseContainerWidget* InvContainerToAdd) const
	PURE_VIRTUAL(IInvUIProvider::AddPawnInvContainers, return nullptr;);
};
