// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interface/Inventory/InvUIProvider.h"
#include "UI/Layout/UILayer.h"
#include "GameMenuLayerInv.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UGameMenuLayerInv : public UUILayer, public IInvUIProvider
{
	GENERATED_BODY()

public:
	UGameMenuLayerInv();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UPanelWidget> PawnInventories;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual TArray<UUInventoryBaseWidget*> GetAllPawnInventories() const override;

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual TArray<UInventoryContainerWidget*> GetAllPawnInvContainers() const override;

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual UPanelSlot* AddPawnInvContainerWidget(UInventoryContainerWidget* InvContainerWidgetToAdd) const override;
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual void RemovePawnInvContainer(UInventoryContainerWidget* InvContainerToRemove) const override;

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual void ToggleInventoryLayout() override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory UI")
	TMap<EInventoryType, FVector2D> InventoryDefaultPositions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory UI")
	bool bInventoryOpen = false;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
