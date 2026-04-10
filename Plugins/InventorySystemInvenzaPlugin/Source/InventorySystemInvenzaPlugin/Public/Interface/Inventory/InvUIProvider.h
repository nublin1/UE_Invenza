//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Inventory/UInventoryBaseWidget.h"
#include "UObject/Interface.h"
#include "InvUIProvider.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
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
	virtual TArray<UUInventoryBaseWidget*> GetAllPawnInventories() const
	PURE_VIRTUAL(IInvUIProvider::GetAllPawnInventories, return TArray<UUInventoryBaseWidget*>(););

	virtual TArray<UInventoryContainerWidget*> GetAllPawnInvContainers() const
	PURE_VIRTUAL(IInvUIProvider::GetAllPawnInvContainers, return TArray<UInventoryContainerWidget*>(););

	virtual UPanelSlot* AddPawnInvContainerWidget(UInventoryContainerWidget* InvContainerWidgetToAdd) const
	PURE_VIRTUAL(IInvUIProvider::AddPawnInvContainers, return nullptr;);

	virtual void RemovePawnInvContainer(UInventoryContainerWidget* InvContainerToRemove) const
	PURE_VIRTUAL(IInvUIProvider::RemovePawnInvContainer,;);
	
	virtual void ToggleInventoryLayout(){};
};
