//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InventoryTypes.h"
#include "UI/BaseUserWidget.h"
#include "InventorySlot.generated.h"

class UCoreCellWidget;
class UInputAction;
class UItemBase;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventorySlot : public UBaseUserWidget
{
	GENERATED_BODY()
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInventorySlot();

	bool operator==(const UInventorySlot& Other) const
	{
		return SlotData == Other.SlotData;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Visual")
	virtual void UpdateVisualWithItemInfo(UItemBase* Item);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Visual")
	virtual void UpdateVisualWithTexture(UTexture2D* NewTexture);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Visual")
	virtual void ResetVisual();
	UFUNCTION(BlueprintCallable, Category = "Inventory|Visual")
	virtual void ClearVisual();
	
	//Getters
	UInventorySlotData* GetSlotData() {return SlotData;}
	FIntPoint GetSlotPosition() const { return SlotData->CellPosition; }
	UInputAction* GetUseAction() const { return SlotData->UseAction; }	
	
	//Setters
	void SetSlotData(UInventorySlotData* NewSlotData) { this->SlotData = NewSlotData; }
	void SetSlotPosition(const FIntPoint InSlotPosition) const { this->SlotData->CellPosition = InSlotPosition; }
	virtual void SetSlotNameText(FString InUseKeyText);
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Data")
	TObjectPtr<UInventorySlotData> SlotData;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
