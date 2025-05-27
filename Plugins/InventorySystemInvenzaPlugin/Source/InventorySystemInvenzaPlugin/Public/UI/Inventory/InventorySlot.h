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

	bool operator==(const UInventorySlot& other) const
	{
		return SlotData.SlotPosition == other.GetSlotPosition();
	}
	
	UFUNCTION(BlueprintCallable)
	virtual void UpdateVisualWithItemInfo(UItemBase* Item);
	UFUNCTION(BlueprintCallable)
	virtual void UpdateVisualWithTexture(UTexture2D* NewTexture);
	UFUNCTION(BlueprintCallable)
	virtual void ResetVisual();
	UFUNCTION(BlueprintCallable)
	virtual void ClearVisual();
	
	//Getters
	FInventorySlotData GetSlotData() const {return SlotData;}
	FIntVector2 GetSlotPosition() const { return SlotData.SlotPosition; }
	UInputAction* GetUseAction() const { return SlotData.UseAction; }	
	
	//Setters
	void SetSlotData(const FInventorySlotData NewSlotData) { this->SlotData = NewSlotData; }
	void SetSlotPosition(const FIntVector2 InSlotPosition) { this->SlotData.SlotPosition = InSlotPosition; }
	virtual void SetSlotNameText(FString InUseKeyText);
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FInventorySlotData SlotData;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
