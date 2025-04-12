//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "InvBaseContainerWidget.generated.h"

class UUIManagerComponent;
class UInvWeightWidget;
class USlotbasedInventoryWidget;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UInvBaseContainerWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInvBaseContainerWidget();

	UFUNCTION(BlueprintCallable)
	virtual USlotbasedInventoryWidget* GetInventoryFromContainerSlot();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta=(BindWidgetOptional))
	TObjectPtr<UNamedSlot> HeaderSlot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta=(BindWidgetOptional))
	TObjectPtr<UNamedSlot> ContainerSlot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta=(BindWidgetOptional))
	TObjectPtr<UNamedSlot> OperationsSlot;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UInvWeightWidget> InvWeight;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	virtual void UpdateWeightInfo(float InventoryTotalWeight, float InventoryWeightCapacity);

	UFUNCTION()
	virtual void TakeAll();
	UFUNCTION()
	virtual void PlaceAll();
	UFUNCTION()
	void TransferAllItems(USlotbasedInventoryWidget* SourceInv, USlotbasedInventoryWidget* TargetInv);
	UFUNCTION()
	virtual void SortItems();
	
};
