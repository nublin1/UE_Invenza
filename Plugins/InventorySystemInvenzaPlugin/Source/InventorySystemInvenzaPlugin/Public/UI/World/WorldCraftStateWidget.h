// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CraftSystem/CraftingStructs.h"
#include "Interface/World/WorldCraft_WidProvider.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Craft/Lists/QueueCraftListEntryWidget.h"
#include "WorldCraftStateWidget.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UWorldCraftStateWidget : public UInvenzaBaseWidget, public IWorldCraft_WidProvider
{
	GENERATED_BODY()
	
public:
	UWorldCraftStateWidget(){};
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UQueueCraftListEntryWidget> QueueCraftListEntryWidget;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	virtual UCraftingComponent* GetCraftComponentPtr() override { return CraftingComponentLink;}
	
	UFUNCTION()
	virtual void SetCraftComponentPtr(UCraftingComponent* NewCraftingComponent) override; 

protected:
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UCraftingComponent> CraftingComponentLink = nullptr;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION()
	void HandleCurrentCraftDataChanged(const FQueuedRecipe& Recipe);
	UFUNCTION()
	void RefreshFromRecipe(const FQueuedRecipe& Recipe);
};
