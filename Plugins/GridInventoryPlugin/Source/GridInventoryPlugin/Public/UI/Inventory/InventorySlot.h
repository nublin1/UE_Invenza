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
class GRIDINVENTORYPLUGIN_API UInventorySlot : public UBaseUserWidget
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
	virtual void UpdateVisual(UItemBase* Item);
	virtual void UpdateVisual(UTexture2D* NewTexture);
	UFUNCTION(BlueprintCallable)
	virtual void ResetVisual();
	
	//Getters
	FORCEINLINE FInventorySlotData GetSlotData() const {return SlotData;}
	FORCEINLINE FIntVector2 GetSlotPosition() const { return SlotData.SlotPosition; }
	FORCEINLINE UInputAction* GetUseAction() const { return SlotData.UseAction; }	
	
	//Setters
	FORCEINLINE void SetSlotData(const FInventorySlotData NewSlotData) { this->SlotData = NewSlotData; }
	FORCEINLINE void SetSlotPosition(const FIntVector2 InSlotPosition) { this->SlotData.SlotPosition = InSlotPosition; }
	FORCEINLINE virtual void SetItemUseKeyText(FString InUseKeyText);
	
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
